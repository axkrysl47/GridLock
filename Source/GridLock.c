/*==============================================================================
File name   : GridLock.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-08-24
Version     : 0.1
Description : Window organizing utility via Windows hotkeys.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <windows.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <dwmapi.h>
#include <commctrl.h>
#include <shlwapi.h>
#include <strsafe.h>

#include "GridLock.h"

#include "Message.h"
#include "Config.h"
#include "Box.h"
#include "Action.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#ifndef VERSION_STRING
#define VERSION_STRING "Unreleased"
#endif

#define GRIDLOCK_CLASS_NAME "GridLockClass"

//------------------------------------------------------------------------------
#define USE_DYNAMIC_MEMORY_ALLOCATION

#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
#define alignup(value, alignment) \
    (((value) + ((alignment) - 1)) & ~((alignment) - 1))
#endif

//------------------------------------------------------------------------------
// Common Control API //
typedef HRESULT (WINAPI *p_DllGetVersion_t)(DLLVERSIONINFO *);
typedef BOOL(WINAPI* p_SetProcessDpiAwarenessContext_t)(HANDLE);

//------------------------------------------------------------------------------
// Unique identifiers for our system tray icon and messages
typedef enum
{
    WMID_NOTIFICATION_AREA_ICON = (WM_APP + 1),
    WMID_TOAST_NOTIFICATION     = (WM_APP + 2),

    WMID_GRIDLOCK_DEFAULT_CONFIGURATION = 1000,
    WMID_GRIDLOCK_DOCUMENTATION         = 1001,

    WMID_GRIDLOCK_ICO         = 1002,
    WMID_GRIDLOCK_ERROR_ICO   = 1003,
    WMID_GRIDLOCK_WARNING_ICO = 1004,
    WMID_GRIDLOCK_INFO_ICO    = 1005,

    WMID_NOTIFICATION_AREA_CONTEXT_ABOUT_GRIDLOCK     = 1006,
    WMID_NOTIFICATION_AREA_CONTEXT_RESTART            = 1007,
    WMID_NOTIFICATION_AREA_CONTEXT_OPEN_DOCUMENTATION = 1008,
    WMID_NOTIFICATION_AREA_CONTEXT_EDIT_CONFIGURATION = 1009,
    WMID_NOTIFICATION_AREA_CONTEXT_RESET_TO_DEFAULTS  = 1010,
    WMID_NOTIFICATION_AREA_CONTEXT_QUIT               = 1011,

    WMID_HOTKEY_DEFINITION_START = 2000,
}
WindowMessageId_t;

//------------------------------------------------------------------------------
typedef enum
{
    TOAST_STATUS_SUCCESS  = 0,
    TOAST_STATUS_FAILED   = 1,
    TOAST_ALREADY_RUNNING = 2,
}
ToastNotificationType_t;


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProcedure(HWND h_window, UINT message, WPARAM wParam,
    LPARAM lParam
);

//------------------------------------------------------------------------------
static void IfLaunchedFromConsoleAttach(void);
static status_t VerifyCommonControlsVersion(uint32_t major, uint32_t minor);

//------------------------------------------------------------------------------
static inline status_t WriteDefaultConfig(FILE* file);
static inline bool ResetToDefaults(bool confirm, bool notify);

//------------------------------------------------------------------------------
static inline status_t InitHotkeys(bool createConfigIfMissing);
static inline void DeinitHotkeys(void);

//------------------------------------------------------------------------------
static inline status_t OpenDocumentation(const char* gridLockFilepath);
static inline status_t EditConfiguration(const char* configurationFilename);


/* Private Member Variables ==================================================*/
//------------------------------------------------------------------------------
static const char* GridLockFilepath;
static const char* ConfigurationFilename;

//------------------------------------------------------------------------------
static HWND h_Window;
static HANDLE h_Mutex;
static HMENU h_PopupMenu;
static NOTIFYICONDATA NotifyIconData; // System Tray //

//------------------------------------------------------------------------------
static uint16_t HotkeysLength;
static uint16_t HotkeySequencesLength;

#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
static void* p_Arena;
static Hotkey_t* Hotkeys;
static HotkeyAction_t* HotkeySequences;
#else
static Hotkey_t Hotkeys[64];
#define HOTKEY_DEFINITIONS_LENGTH \
    (sizeof(Hotkeys) / sizeof(Hotkey_t))

static HotkeyAction_t HotkeySequences[64];
#define HOTKEY_SEQUENCES_LENGTH \
    (sizeof(HotkeySequences) / sizeof(HotkeyAction_t))
#endif


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static void IfLaunchedFromConsoleAttach(void)
{
    BOOL attached = AttachConsole((DWORD)(-1));
    if (false == attached)
    {
        return;
    }

    SacAbortIfNull(freopen("CONOUT$", "w", stdout));
    SacAbortIfNull(freopen("CONOUT$", "w", stderr));
    SacAbortIfNull(freopen("CONIN$", "r", stdin));

    printf("\n");
}

//------------------------------------------------------------------------------
static status_t VerifyCommonControlsVersion(uint32_t major, uint32_t minor)
{
    HMODULE hMod = GetModuleHandleA("comctl32.dll");
    SacReturnIfNull(hMod,
        Message_InitPrintErrorExit(
            "Failed to get Windows Common Controls Library",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    p_DllGetVersion_t p_DllGetVersion =
        (p_DllGetVersion_t)(void*)GetProcAddress(hMod, "DllGetVersion");
    SacReturnIfNull(p_DllGetVersion,
        Message_InitPrintErrorExit(
            "Could not request Windows Common Controls Library version",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    DLLVERSIONINFO dllVersionInfo = {0};
    dllVersionInfo.cbSize = sizeof(dllVersionInfo);

    HRESULT h_result = p_DllGetVersion(&dllVersionInfo);
    SacReturnIf(FAILED(h_result),
        Message_InitPrintErrorExit(
            "Failed request for Windows Common Controls Library version",
            HRESULT_CODE(h_result),
            STATUS_FAILURE
        )
    );

    printd("ComCtl32 Version: %lu.%lu\n",
        dllVersionInfo.dwMajorVersion, dllVersionInfo.dwMinorVersion
    );

    SacReturnIf((dllVersionInfo.dwMajorVersion < major), STATUS_NOTSUPPORTED);
    SacReturnIf(
        ((dllVersionInfo.dwMajorVersion == major)
        && (dllVersionInfo.dwMinorVersion < minor)),
        STATUS_NOTSUPPORTED
    );
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static inline status_t WriteDefaultConfig(FILE* file)
{
    assert(file);

    HRSRC h_resource =
        FindResource(
            NULL,
            MAKEINTRESOURCE(WMID_GRIDLOCK_DEFAULT_CONFIGURATION),
            "YAMLDATA"
        );

    HGLOBAL h_global = LoadResource(NULL, h_resource);
    SacPrintReturnIfNot(h_global, STATUS_FAILURE, "Failed to load resource\n");

    DWORD size = SizeofResource(NULL, h_resource);
    SacPrintReturnIf((0 == size), STATUS_FAILURE, "Size of resource is 0\n");

    char* p_data = (char*)LockResource(h_global);
    SacPrintReturnIfNull(p_data, STATUS_FAILURE, "Data pointer is null\n");

    fwrite(p_data, 1, size, file);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static inline bool ResetToDefaults(bool confirm, bool notify)
{
    // Create backup filename //
    char backupFilename[MAX_PATH];
    Utility_StringCopy(
        backupFilename, ConfigurationFilename, sizeof(backupFilename)
    );

    // strrchr = string reverse character search //
    const char* dot       = strrchr(ConfigurationFilename, '.');
    const char* slash     = strrchr(ConfigurationFilename, '/');
    const char* backslash = strrchr(ConfigurationFilename, '\\');
    const char* lastSlash =
        (NULL == slash) ? backslash
        : (NULL == backslash) ? slash
        : (slash < backslash) ? backslash : slash;
    if ((NULL != dot) && (NULL != lastSlash) && (dot < lastSlash))
    {
        dot = NULL; // Could not find a valid dot.
    }

    if (NULL == dot)
    {
        // No extension found, just append it to the end
        snprintf(
            backupFilename, sizeof(backupFilename),
            "%s.saved.yaml",
            ConfigurationFilename
        );
    }
    else
    {
        size_t baseLen = dot - ConfigurationFilename;
        snprintf(
            backupFilename, sizeof(backupFilename),
            "%.*s.saved.%s",
            (int)baseLen, ConfigurationFilename, dot + 1
        );
    }

    // Confirm with user before continuing. //
    if (confirm)
    {
        confirm = Message_ConfirmResetToDefaults(
            ConfigurationFilename, backupFilename
        );
        if (false == confirm)
        {
            return false;
        }
    }

    remove(backupFilename);
    int renamed = rename(ConfigurationFilename, backupFilename);
    if (0 != renamed)
    {
        // Open configuration file should fail, otherwise backup failed. //
        FILE* file = fopen(ConfigurationFilename, "r");
        if (NULL != file)
        {
            fclose(file);
            Message_WarningBackupFailed(ConfigurationFilename);
            printd("Failed to back up existing configuration file");
            return false;
        }
    }

    FILE* file = fopen(ConfigurationFilename, "wb");
    if (NULL == file)
    {
        Message_InitConfigPrintErrorExit(
            ConfigurationFilename,
            "Failed to open configuration file",
            GetLastError()
        );
        printd("Failed to open configuration file\n");
        return false;
    }

    const status_t written = WriteDefaultConfig(file);
    if (STATUS_SUCCESS != written)
    {
        Message_InitConfigPrintErrorExit(
            ConfigurationFilename,
            "Failed to initialize default configuration file",
            GetLastError()
        );
        printd("Failed to initialize default configuration file\n");
        fclose(file);
        return false;
    }

    fclose(file);

    if (notify)
    {
        notify = Message_NotifyResetToDefaults();
        if (false == notify)
        {
            return false;
        }
    }

    DeinitHotkeys();
    const bool createConfigIfMissing = false;
    const bool initialized = InitHotkeys(createConfigIfMissing);
    if (STATUS_SUCCESS != initialized)
    {
        if (NULL != h_Window && IsWindow(h_Window))
        {
            PostQuitMessage(initialized);
        }
        return false;
    }

    return true;
}


//------------------------------------------------------------------------------
static inline status_t InitHotkeys(bool createConfigIfMissing)
{
    FILE* file = fopen(ConfigurationFilename, "r");
    if (NULL == file)
    {
        if (false == createConfigIfMissing)
        {
            // We were passed a filename which we cannot open, exit //
            return Message_InitConfigPrintErrorExit(
                ConfigurationFilename,
                "Failed to open configuration file",
                GetLastError()
            );
        }

        SacReturnIfNot(
            Message_Welcome(ConfigurationFilename), STATUS_ACCESSDENIED
        );

        file = fopen(ConfigurationFilename, "wb");
        SacReturnIfNull(file,
            Message_InitConfigPrintErrorExit(
                ConfigurationFilename,
                "Failed to open configuration file to write",
                GetLastError()
            )
        );

        const status_t status = WriteDefaultConfig(file);
        if (STATUS_SUCCESS != status)
        {
            const DWORD error = GetLastError();
            Message_InitConfigPrintErrorExit(
                ConfigurationFilename,
                "Failed to initialize default configuration file",
                error
            );
            fclose(file);
            return error;
        }

        fclose(file);

        file = fopen(ConfigurationFilename, "r");
    }
    SacReturnIfNull(file,
        Message_InitConfigPrintErrorExit(
            ConfigurationFilename,
            "Failed to open file",
            GetLastError()
        )
    );

    ConfigInitialPass_t configInitialPass =
    {
        .version.major    = 0,
        .version.minor    = 0,
        .version.revision = 0,
        .minimum.width = 100,
        .minimum.height = 100,
        .borderVisibility = BORDER_INVISIBLE,
        .n_hotkeys = 0,
        .n_sequences = 0,
    };
    status_t status = Config_InitialPass(file, &configInitialPass);
    if (STATUS_SUCCESS != status)
    {
        fclose(file);
        return STATUS_INVALIDFORMAT;
    }

    HotkeysLength = configInitialPass.n_hotkeys;
    HotkeySequencesLength = configInitialPass.n_sequences;

    SacReturnIf((HotkeysLength <= 0),
        Message_InitConfigPrintErrorExit(
            ConfigurationFilename,
            "Configuration contains no hotkeys",
            STATUS_INVALIDPARAM
        )
    );

#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
    const size_t arenaSize =
        alignup((HotkeysLength * sizeof(Hotkey_t)), sizeof(HotkeyAction_t))
        + (HotkeySequencesLength * sizeof(HotkeyAction_t));
    p_Arena = malloc(arenaSize);
    if (NULL == p_Arena)
    {
        fclose(file);
        return Message_InitConfigPrintErrorExit(
            ConfigurationFilename,
            "Failed to allocate memory for hotkeys",
            GetLastError()
        );
    }
    memset(p_Arena, 0u, arenaSize);

    Hotkeys = (Hotkey_t*)p_Arena;
    HotkeySequences = (HotkeyAction_t*)&(Hotkeys[HotkeysLength]);
#else
    if ((HOTKEY_DEFINITIONS_LENGTH < HotkeysLength)
        || (HOTKEY_SEQUENCES_LENGTH < HotkeySequencesLength))
    {
        fclose(file);
        return STATUS_INVALIDFORMAT;
    }

    memset(Hotkeys, 0, sizeof(Hotkeys));
    memset(HotkeySequences, 0, sizeof(HotkeySequences));
#endif

    rewind(file);

    status = Config_ParseHotkeys(
        file, HotkeysLength, Hotkeys, HotkeySequencesLength, HotkeySequences
    );
    fclose(file);
    SacReturnIfErr(status,
        Message_InitConfigExit(
            ConfigurationFilename,
            "Failed configure hotkeys",
            STATUS_INVALIDADDR
        )
    );

    HMODULE h_user32Module = GetModuleHandleA("user32.dll");
    SacReturnIfNull(h_user32Module,
        Message_InitPrintErrorExit(
            "Failed to get Windows user API client",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    p_IsWindowArranged_t p_IsWindowArranged =
        (p_IsWindowArranged_t)(void*)
        GetProcAddress(h_user32Module, "IsWindowArranged");
    if (NULL == p_IsWindowArranged)
    {
        printd("Could not dynamically load \"IsWindowArranged\"\n");
    }

    p_SetProcessDpiAwarenessContext_t p_SetProcessDpiAwarenessContext =
        (p_SetProcessDpiAwarenessContext_t)(void*)
        GetProcAddress(h_user32Module, "SetProcessDpiAwarenessContext");

    if (NULL == p_SetProcessDpiAwarenessContext)
    {
        printd(
            "Could not dynamically load \"SetProcessDpiAwarenessContext\"\n"
        );
    }
    else
    {
        BOOL aware =
            p_SetProcessDpiAwarenessContext(
                DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2
            );
        if (false == aware)
        {
            printd("Failed to set process DPI awareness context\n");
        }
    }

    WindowConfig_t windowConfig =
    {
        .p_IsWindowArranged = p_IsWindowArranged,
        .minimum =
        {
            .width  = configInitialPass.minimum.width,
            .height = configInitialPass.minimum.height
        },
        .borderVisibility = configInitialPass.borderVisibility
    };
    status = Action_Init(&windowConfig);
    SacReturnIfErr(status,
        Message_InitConfigExit(
            ConfigurationFilename,
            "Failed configure hotkey actions",
            STATUS_FAILURE
        )
    );

    for (uint_fast16_t i = 0; i < HotkeysLength; i++)
    {
        UnregisterHotKey(h_Window, WMID_HOTKEY_DEFINITION_START + i);

        printf("Registering:\n%s\n", Config_StringPrintHotkey(&(Hotkeys[i])));

        BOOL registered =
            RegisterHotKey(
                h_Window,
                WMID_HOTKEY_DEFINITION_START + i,
                (UINT)Hotkeys[i].keybind.modifiers,
                (UINT)Hotkeys[i].keybind.key
            );
        if (false == registered)
        {
            DWORD error = GetLastError();
            if (ERROR_HOTKEY_ALREADY_REGISTERED == error)
            {
                Message_WarningHotkeyAlreadyRegistered(
                    Config_StringPrintHotkey(&(Hotkeys[i]))
                );
                printd("Failed to register hotkey, another app is using it\n");
            }
            else
            {
                Message_ErrorHotkeyRegistration(
                    Config_StringPrintHotkey(&(Hotkeys[i])),
                    error
                );
                printd("Failed to register hotkey");
            }
        }
    }

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static inline void DeinitHotkeys(void)
{
    const uint16_t length = HotkeysLength;
    HotkeysLength = 0;
    for (uint_fast16_t i = 0; i < length; i++)
    {
        UnregisterHotKey(h_Window, WMID_HOTKEY_DEFINITION_START + i);
    }
#ifdef USE_DYNAMIC_MEMORY_ALLOCATION
    free(p_Arena);
    p_Arena = NULL;
#endif
}

//------------------------------------------------------------------------------
static inline status_t OpenDocumentation(const char* gridLockFilepath)
{
    char path[MAX_PATH];

    const char* lastBackslash = strrchr(gridLockFilepath, '\\');
    if (NULL == lastBackslash)
    {
        strcpy(path, "GridLockDocumentation.html");
    }
    else
    {
        size_t dirLength = (lastBackslash - gridLockFilepath) + 1;

        SacPrintReturnIf(((MAX_PATH - 30) <= dirLength), STATUS_FAILURE,
            "Directory length too long to open documentation\n"
        );

        strncpy(path, gridLockFilepath, dirLength);
        path[dirLength] = '\0';

        strcat(path, "GridLockDocumentation.html");
    }

    FILE* file = fopen(path, "rb");
    if (NULL == file)
    {
        HRSRC h_resource =
            FindResource(
                NULL,
                MAKEINTRESOURCE(WMID_GRIDLOCK_DOCUMENTATION),
                "HTMLDATA"
            );
        SacPrintReturnIfNot(
            h_resource, STATUS_FAILURE, "Failed to find resource\n"
        );

        HGLOBAL h_global = LoadResource(NULL, h_resource);
        SacPrintReturnIfNot(h_global, STATUS_FAILURE,
            "Failed to load resource\n"
        );

        DWORD size = SizeofResource(NULL, h_resource);
        SacPrintReturnIf((0 == size), STATUS_FAILURE,
            "Size of resource is 0\n"
        );

        char* p_data = (char*)LockResource(h_global);
        SacPrintReturnIfNull(p_data, STATUS_FAILURE, "Data pointer is null\n");

        file = fopen(path, "wb");
        SacPrintReturnIfNull(file, STATUS_FAILURE,
            "Could not open documentation\n"
        );

        fwrite(p_data, 1, size, file);
    }

    fclose(file);

    ShellExecuteA(NULL, "open", path, NULL, NULL, SW_SHOWNORMAL);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t EditConfiguration(const char* configurationFilename)
{
    INT_PTR result =
        (INT_PTR)ShellExecuteA(
            NULL, "open", configurationFilename, NULL, NULL, SW_SHOWNORMAL
        );
    if (SHELL_EXECUTE_MAX_ERROR < result)
    {
        return STATUS_SUCCESS;
    }

    if (SE_ERR_NOASSOC == result)
    {
        result =
        (INT_PTR)ShellExecuteA(
            NULL,
            "open", "notepad.exe", configurationFilename, NULL, SW_SHOWNORMAL
        );
        if (SHELL_EXECUTE_MAX_ERROR < result)
        {
            return STATUS_SUCCESS;
        }

        Message_NotepadFailed(configurationFilename);
        return STATUS_FAILURE;
    }

    Message_OpenFailed(configurationFilename);
    return STATUS_FAILURE;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Main Process
// int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//     LPSTR lpCmdLine, int nCmdShow)
//------------------------------------------------------------------------------
int main(int argc, char *argv[])
{
    IfLaunchedFromConsoleAttach();

    const MessageIconIds_t messageIconIds =
    {
        .icon = WMID_GRIDLOCK_ICO,
        .error = WMID_GRIDLOCK_ERROR_ICO,
        .warning = WMID_GRIDLOCK_WARNING_ICO,
        .info = WMID_GRIDLOCK_INFO_ICO
    };
    Message_Init(&messageIconIds, SOUND_UNLATCHED);

    if (3 <= argc)
    {
        printf("Error - GridLock\nUsage: %s <filename or filepath>\n", argv[0]);
        return STATUS_INVALIDPARAM;
    }
    if (2 == argc)
    {
        printd("GridLock was passed configuration file \"%s\"\n", argv[1]);
    }

    if (1 <= argc)
    {
        GridLockFilepath = argv[0];
    }
    ConfigurationFilename = (2 == argc) ? argv[1] : "GridLockConfig.yaml";

    printd("Starting GridLock\n");

    // There is no ANSI ASCII version of this function //
    HRESULT h_result = SetCurrentProcessExplicitAppUserModelID(L"GridLock");
    SacReturnIf(FAILED(h_result),
        Message_InitPrintErrorExit(
            "Failed to set app user mode ID",
            HRESULT_CODE(h_result),
            STATUS_INVALIDHANDLE
        )
    );

    INITCOMMONCONTROLSEX initCommonControlEx =
    {
        .dwSize = sizeof(INITCOMMONCONTROLSEX),
        .dwICC = ICC_STANDARD_CLASSES,
    };
    SacReturnIfNot(
        InitCommonControlsEx(&initCommonControlEx),
        Message_InitPrintErrorExit(
            "Windows common controls version failed to initialize",
            ERROR_BAD_ENVIRONMENT,
            STATUS_INVALIDHANDLE
        )
    );
    SacReturnIfErr(
        VerifyCommonControlsVersion(6u, 0u),
        Message_InitPrintErrorExit(
            "Windows common controls version unsupported",
            ERROR_BAD_ENVIRONMENT,
            STATUS_INVALIDHANDLE
        )
    );

    h_Mutex = CreateMutexA(NULL, TRUE, "Local\\GridLockRunningMutex");
    SacReturnIfNull(h_Mutex,
        Message_InitPrintErrorExit(
            "Failed to create mutex",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    if (ERROR_ALREADY_EXISTS == GetLastError())
    {
        printd("GridLock mutex already exists, yielding...\n");

        HWND h_window = FindWindowA(GRIDLOCK_CLASS_NAME, NULL);
        if (NULL != h_window)
        {
            PostMessageA(h_window,
                WMID_TOAST_NOTIFICATION, TOAST_ALREADY_RUNNING, 0
            );
        }

        CloseHandle(h_Mutex);

        return STATUS_SUCCESS;
    }

    HINSTANCE h_instance = GetModuleHandleA(NULL);
    SacReturnIfNull(h_instance,
        Message_InitPrintErrorExit(
            "Failed to get module instance handle",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    WNDCLASSEX windowClass =
    {
        .cbSize = sizeof(WNDCLASSEX),
        .lpfnWndProc = WindowProcedure,
        .hInstance = h_instance,
        .lpszClassName = GRIDLOCK_CLASS_NAME,
        .hIcon = LoadIconA(h_instance, MAKEINTRESOURCEA(WMID_GRIDLOCK_ICO)),
        .hIconSm = LoadIconA(h_instance, MAKEINTRESOURCEA(WMID_GRIDLOCK_ICO)),
    };
    SacReturnIfNot(RegisterClassEx(&windowClass),
        Message_InitPrintErrorExit(
            "Failed to register class",
            GetLastError(),
            STATUS_ENTRYEXISTS
        )
    );

    // Create empty window, GridLock only runs in the background //
    h_Window =
        CreateWindowExA(
            0, GRIDLOCK_CLASS_NAME, "GridLock",
            WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, NULL, NULL, h_instance, NULL
        );
    SacReturnIfNull(h_Window,
        Message_InitPrintErrorExit(
            "Failed to create window",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    //--------------------------------------------------------------------------
    // Initialize the GridLock Hotkeys
    //--------------------------------------------------------------------------
    const bool createConfigIfMissing = (2 != argc);
    status_t status = InitHotkeys(createConfigIfMissing);
    if (STATUS_ACCESSDENIED == status)
    {
        CloseHandle(h_Mutex);
        DestroyWindow(h_Window);
        UnregisterClassA(GRIDLOCK_CLASS_NAME, h_instance);
        return ERROR_SUCCESS;
    }
    if (STATUS_SUCCESS != status)
    {
        if (false == Message_RequestResetToDefaults())
        {
            CloseHandle(h_Mutex);
            DestroyWindow(h_Window);
            UnregisterClassA(GRIDLOCK_CLASS_NAME, h_instance);
            return status;
        }

        const bool confirm = true;
        const bool notify = false;
        bool reset = ResetToDefaults(confirm, notify);
        if (false == reset)
        {
            CloseHandle(h_Mutex);
            DestroyWindow(h_Window);
            UnregisterClassA(GRIDLOCK_CLASS_NAME, h_instance);
            return status;
        }
    }

    h_PopupMenu = CreatePopupMenu();
    SacReturnIfNull(h_PopupMenu,
        Message_InitPrintErrorExit(
            "Failed to create popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    SacReturnIfNot(
        AppendMenuA(h_PopupMenu,
            MF_STRING,
            WMID_NOTIFICATION_AREA_CONTEXT_ABOUT_GRIDLOCK,
            "About GridLock"
        ),
        Message_InitPrintErrorExit(
            "Failed to append \"About GridLock\" to popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    SacReturnIfNot(
        AppendMenuA(h_PopupMenu,
            MF_STRING,
            WMID_NOTIFICATION_AREA_CONTEXT_RESTART,
            "Restart"
        ),
        Message_InitPrintErrorExit(
            "Failed to append \"Restart\" to popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    SacReturnIfNot(
        AppendMenuA(h_PopupMenu,
            MF_STRING,
            WMID_NOTIFICATION_AREA_CONTEXT_OPEN_DOCUMENTATION,
            "Open Documentation"
        ),
        Message_InitPrintErrorExit(
            "Failed to append \"Open Documentation\" to popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    SacReturnIfNot(
        AppendMenuA(h_PopupMenu,
            MF_STRING,
            WMID_NOTIFICATION_AREA_CONTEXT_EDIT_CONFIGURATION,
            "Edit Configuration"
        ),
        Message_InitPrintErrorExit(
            "Failed to append \"Edit Configuration\" to popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    SacReturnIfNot(
        AppendMenuA(h_PopupMenu,
            MF_STRING,
            WMID_NOTIFICATION_AREA_CONTEXT_RESET_TO_DEFAULTS,
            "Reset to Defaults"
        ),
        Message_InitPrintErrorExit(
            "Failed to append \"Reset to Defaults\" to popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    SacReturnIfNot(
        AppendMenuA(h_PopupMenu,
            MF_STRING,
            WMID_NOTIFICATION_AREA_CONTEXT_QUIT,
            "Quit"
        ),
        Message_InitPrintErrorExit(
            "Failed to append \"Quit\" to popup menu",
            GetLastError(),
            STATUS_INVALIDHANDLE
        )
    );

    NotifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    NotifyIconData.hWnd = h_Window;
    NotifyIconData.uID = WMID_GRIDLOCK_ICO;
    NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    NotifyIconData.uCallbackMessage = WMID_NOTIFICATION_AREA_ICON;
    NotifyIconData.hIcon =
        (HICON)LoadImageA(
            h_instance,
            MAKEINTRESOURCEA(WMID_GRIDLOCK_ICO),
            IMAGE_ICON,
            0, 0,
            LR_DEFAULTSIZE
        );
    NotifyIconData.dwInfoFlags = NIIF_USER | NIIF_LARGE_ICON;
    h_result =
        StringCchCopyA(NotifyIconData.szTip, sizeof(NotifyIconData.szTip),
            "GridLock"
        );
    SacReturnIf(FAILED(h_result),
        Message_InitPrintErrorExit(
            "Failed to title notification area icon",
            HRESULT_CODE(h_result),
            STATUS_TIMEDOUT
        )
    );
    SacReturnIfNot(
        Shell_NotifyIconA(NIM_ADD, &NotifyIconData),
        Message_InitPrintErrorExit(
            "Failed to create notification area icon",
            GetLastError(),
            STATUS_TIMEDOUT
        )
    );

    Message_SetSoundLatch(SOUND_FREERUNNING);
    PostMessageA(h_Window, WMID_TOAST_NOTIFICATION, TOAST_STATUS_SUCCESS, 0);

    // Standard Win32 Message Loop //
    MSG message;
    while (0 < GetMessageA(&message, NULL, 0, 0))
    {
        TranslateMessage(&message);
        DispatchMessageA(&message);
    }

    DeinitHotkeys();
    NOTIFYICONDATA deleteNotifyIconData = {0};
    deleteNotifyIconData.cbSize = sizeof(deleteNotifyIconData);
    deleteNotifyIconData.hWnd = h_Window;
    deleteNotifyIconData.uID = WMID_GRIDLOCK_ICO;
    Shell_NotifyIconA(NIM_DELETE, &deleteNotifyIconData);
    DestroyMenu(h_PopupMenu);
    CloseHandle(h_Mutex);
    DestroyWindow(h_Window);
    UnregisterClassA(GRIDLOCK_CLASS_NAME, h_instance);
    return ERROR_SUCCESS;
}

//------------------------------------------------------------------------------
// GridLock_Help
//------------------------------------------------------------------------------
status_t GridLock_Help(void)
{
    assert(NULL != GridLockFilepath);
    assert(NULL != ConfigurationFilename);

    return Message_Help(
        VERSION_STRING, GridLockFilepath, ConfigurationFilename
    );
}


/* Global Callback Function Implementations ==================================*/
//------------------------------------------------------------------------------
// The Window Procedure processes messages sent to our invisible window
//------------------------------------------------------------------------------
LRESULT CALLBACK WindowProcedure(HWND h_window, UINT message, WPARAM wParam,
    LPARAM lParam)
{
    //--------------------------------------------------------------------------
    switch (message) {

    case (WM_HOTKEY):
    {
        // Return ERROR_SUCCESS as we do not want to close the application. //
        SacPrintReturnIf((wParam < WMID_HOTKEY_DEFINITION_START), ERROR_SUCCESS,
            "Hotkey index out of bounds error\n"
        );

        uint16_t i_hotkey = wParam - WMID_HOTKEY_DEFINITION_START;
        SacPrintReturnIf((HotkeysLength <= i_hotkey), ERROR_SUCCESS,
            "Hotkey index out of bounds error\n"
        );

        HWND h_foregroundWindow = GetForegroundWindow();
        SacPrintReturnIfNull(h_foregroundWindow,
            ERROR_SUCCESS,
            "No active window detected"
            " (desktop or taskbar might be in focus).\n"
        );

        char windowText[256];
        int charsCopied =
            GetWindowTextA(h_foregroundWindow, windowText, sizeof(windowText));

        printf("\nApplication = [ %s ]\n",
            (charsCopied <= 0) ? "No Title / Invisible Window" : windowText
        );

        p_Action_t p_action = Hotkeys[i_hotkey].action.p_function;

        // Return ERROR_SUCCESS as we do not want to close the application. //
        SacPrintReturnIfNull(p_action, ERROR_SUCCESS, "Action was null!\n");

        p_action(
            h_foregroundWindow, &(Hotkeys[i_hotkey].action.param)
        );
    }
    break;

    case (WMID_TOAST_NOTIFICATION):
    {
        switch (wParam)
        {

        case (TOAST_STATUS_SUCCESS):
        {
            printd("GridLock is initialized\n");

            NotifyIconData.uFlags = NIF_INFO;

            HRESULT h_result =
                StringCchCopyA(
                    NotifyIconData.szInfoTitle,
                    sizeof(NotifyIconData.szInfoTitle),
                    "Started GridLock"
                );
            SacReturnIf(FAILED(h_result),
                Message_InitPrintErrorExit("Failed to title success toast",
                    HRESULT_CODE(h_result), STATUS_TIMEDOUT
                )
            );

            h_result =
                StringCchCopyA(
                    NotifyIconData.szInfo, sizeof(NotifyIconData.szInfo),
                    "Your window organization utility is ready"
                );
            SacReturnIf(FAILED(h_result),
                Message_InitPrintErrorExit("Failed to subtitle success toast",
                    HRESULT_CODE(h_result), STATUS_TIMEDOUT
                )
            );

            Shell_NotifyIconA(NIM_MODIFY, &NotifyIconData);
        }
        break;

        case (TOAST_STATUS_FAILED):
        {
            printd("GridLock failed to initialize\n");

            NotifyIconData.uFlags = NIF_INFO;

            HRESULT h_result =
                StringCchCopyA(
                    NotifyIconData.szInfoTitle,
                    sizeof(NotifyIconData.szInfoTitle),
                    "GridLock failed to start"
                );
            SacReturnIf(FAILED(h_result),
                Message_InitPrintErrorExit("Failed to title failure toast",
                    HRESULT_CODE(h_result), STATUS_TIMEDOUT
                )
            );

            h_result =
                StringCchCopyA(
                    NotifyIconData.szInfo, sizeof(NotifyIconData.szInfo),
                    "Please check your GridLockConfig.yaml and try again"
                );
            SacReturnIf(FAILED(h_result),
                Message_InitPrintErrorExit("Failed to subtitle failure toast",
                    HRESULT_CODE(h_result), STATUS_TIMEDOUT
                )
            );

            Shell_NotifyIconA(NIM_MODIFY, &NotifyIconData);
        }
        break;

        case (TOAST_ALREADY_RUNNING):
        {
            printd("Duplicate launch attempt - flashing tray notification\n");

            NotifyIconData.uFlags = NIF_INFO;

            HRESULT h_result =
                StringCchCopyA(
                    NotifyIconData.szInfoTitle,
                    sizeof(NotifyIconData.szInfoTitle),
                    "GridLock is already running"
                );
            SacReturnIf(FAILED(h_result),
                Message_InitPrintErrorExit("Failed to title already toast",
                    HRESULT_CODE(h_result), STATUS_TIMEDOUT
                )
            );

            h_result =
                StringCchCopyA(
                    NotifyIconData.szInfo, sizeof(NotifyIconData.szInfo),
                    "To quit, check your system tray"
                );
            SacReturnIf(FAILED(h_result),
                Message_InitPrintErrorExit("Failed to subtitle already toast",
                    HRESULT_CODE(h_result), STATUS_TIMEDOUT
                )
            );

            Shell_NotifyIconA(NIM_MODIFY, &NotifyIconData);
        }
        break;

        }
        // end switch //
    }
    break;

    case (WMID_NOTIFICATION_AREA_ICON):
    {
        // Check if the user right-clicked the tray icon
        if (WM_LBUTTONUP == lParam || WM_RBUTTONUP == lParam)
        {
            POINT point;
            GetCursorPos(&point);

            // TrackPopupMenu opens our context menu at the mouse coordinates
            SetForegroundWindow(h_window);
            TrackPopupMenu(h_PopupMenu,
                TPM_BOTTOMALIGN | TPM_LEFTALIGN,
                point.x, point.y,
                0,
                h_window,
                NULL
            );
            PostMessageA(h_window, WM_NULL, 0, 0);
        }
    }
    break;

    case (WM_COMMAND):
    {
        switch (LOWORD(wParam))
        {

        case (WMID_NOTIFICATION_AREA_CONTEXT_ABOUT_GRIDLOCK):
        {
            Message_About(
                VERSION_STRING, GridLockFilepath, ConfigurationFilename
            );
        }
        break;

        case (WMID_NOTIFICATION_AREA_CONTEXT_RESTART):
        {
            Message_SetSoundLatch(SOUND_UNLATCHED);

            DeinitHotkeys();
            const bool createConfigIfMissing = false;
            status_t status = InitHotkeys(createConfigIfMissing);
            if (STATUS_SUCCESS != status)
            {
                PostMessageA(
                    h_window, WMID_TOAST_NOTIFICATION, TOAST_STATUS_FAILED, 0
                );
                Sleep(3000);
                PostQuitMessage(0);
            }

            Message_SetSoundLatch(SOUND_FREERUNNING);
            PostMessageA(
                h_Window, WMID_TOAST_NOTIFICATION, TOAST_STATUS_SUCCESS, 0
            );
        }
        break;

        case (WMID_NOTIFICATION_AREA_CONTEXT_OPEN_DOCUMENTATION):
        {
            OpenDocumentation(GridLockFilepath);
        }
        break;

        case (WMID_NOTIFICATION_AREA_CONTEXT_EDIT_CONFIGURATION):
        {
            EditConfiguration(ConfigurationFilename);
        }
        break;

        case (WMID_NOTIFICATION_AREA_CONTEXT_RESET_TO_DEFAULTS):
        {
            const bool confirm = true;
            const bool notify = true;
            (void)ResetToDefaults(confirm, notify);
        }
        break;

        case (WMID_NOTIFICATION_AREA_CONTEXT_QUIT):
        {
            PostQuitMessage(0);
        }
        break;

        default: break;

        }
        // end switch //
    }
    break;

    case (WM_DESTROY):
    {
        PostQuitMessage(0);
    }
    break;

    default:
    return DefWindowProcA(h_window, message, wParam, lParam);

    } // end switch //----------------------------------------------------------

    return 0;
}
