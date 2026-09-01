/*==============================================================================
File name   : Message.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-08-30
Version     : 0.1
Description : This module defines the various Message Boxes which GridLock uses
              as a user interface.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "Message.h"

#include "Utility.h"


/* Local Structures/Macros ===================================================*/


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline SoundLatch_t HandleSound(SoundLatch_t latch, int sound);

//------------------------------------------------------------------------------
static inline void MessageShow(const char* caption, unsigned int iconId,
    int soundType, const char* format, va_list args
);
static inline status_t MessageShowAbout(const char* caption,
    unsigned int iconId, bool okCancel, const char* version,
    const char* gridlockFilepath, const char* configurationFilename
);


/* Private Member Variables ==================================================*/
//------------------------------------------------------------------------------
static MessageIconIds_t IconIds;
static SoundLatch_t SoundLatch;

//------------------------------------------------------------------------------
static char PrintBuffer[1024];


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline SoundLatch_t HandleSound(SoundLatch_t latch, int sound)
{
    switch (latch)
    {
        default:
        case (SOUND_FREERUNNING): MessageBeep(sound); return SOUND_FREERUNNING;
        case (SOUND_UNLATCHED)  : MessageBeep(sound); return SOUND_LATCHED;
        case (SOUND_LATCHED)    :                     return SOUND_LATCHED;
    }
    // end switch //
}

//------------------------------------------------------------------------------
static inline void MessageShow(const char* caption, unsigned int iconId,
    int soundType, const char* format, va_list args)
{
    assert(format);

    int length = vsnprintf(PrintBuffer, sizeof(PrintBuffer), format, args);
    if (length < 0 || (int)(sizeof(PrintBuffer)) <= length)
    {
        const char* failString =
            "An internal error occurred. This is not expected.\n";
        Utility_StringCopy(PrintBuffer, failString, sizeof(PrintBuffer));
        length = strnlen(failString, sizeof(PrintBuffer));
    }

    const char* exitOptionString =
        "\n"
        "Select OK to continue, or Cancel to exit GridLock";
    if ((int)(sizeof(PrintBuffer))
        <= length + (int)strnlen(exitOptionString, sizeof(PrintBuffer)))
    {
        printd("Buffer overrun!");
    }
    Utility_StringCopy(
        &(PrintBuffer[length]), exitOptionString, sizeof(PrintBuffer) - length
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = caption,
        .dwStyle = MB_OKCANCEL | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(iconId),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, soundType);
    int selection = MessageBoxIndirect(&messageBoxParams);
    if (IDCANCEL == selection)
    {
        PostQuitMessage(0);
    }
}

//------------------------------------------------------------------------------
static inline status_t MessageShowAbout(const char* caption,
    unsigned int iconId, bool okCancel, const char* version,
    const char* gridlockFilepath, const char* configurationFilename)
{
    int written =
        snprintf(PrintBuffer, sizeof(PrintBuffer),
            "GridLock - A window organization utility for Windows\n"
            "Version %s\n"
            "\n"
            "Created by Alexander Krysl (axkrysl47)\n"
            "\n"
            "Launched from\n"
            "\"%s\"\n"
            "\n"
            "Configuration file\n"
            "\"%s\"",
            version, gridlockFilepath, configurationFilename
        );
    if (written < 0 || sizeof(PrintBuffer) <= (size_t)written)
    {
        printd("MessageShowAbout was truncated\n");
    }

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = caption,
        .dwStyle = (okCancel ? MB_OKCANCEL : MB_OK) | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(iconId),
        // ...existing fields...
    };

    int selection = MessageBoxIndirect(&messageBoxParams);
    if (okCancel && IDCANCEL == selection)
    {
        PostQuitMessage(0);
    }

    return STATUS_SUCCESS;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Message_Init
//------------------------------------------------------------------------------
void Message_Init(const MessageIconIds_t* p_icons, SoundLatch_t latch)
{
    assert(p_icons);

    IconIds = *p_icons;
    SoundLatch = latch;
}

//------------------------------------------------------------------------------
// Message_SetSoundLatch
//------------------------------------------------------------------------------
void Message_SetSoundLatch(SoundLatch_t latch)
{
    assert(SOUND_FREERUNNING <= latch && latch <= SOUND_LATCHED);
    SoundLatch = latch;
}

//------------------------------------------------------------------------------
// Message_GetSoundLatch
//------------------------------------------------------------------------------
SoundLatch_t Message_GetSoundLatch(void)
{
    return SoundLatch;
}

//------------------------------------------------------------------------------
// Message_InitPrintErrorExit
//------------------------------------------------------------------------------
status_t Message_InitPrintErrorExit(const char* prefix, int error,
    status_t status)
{
    assert(prefix);

    int written = snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock encountered an error and failed to start.\n"
        "\n"
        "Please check your installation and try again.\n"
        "\n"
        "Error:\n"
        "%s: ",
        prefix
    );
    if ((written < 0) || (sizeof(PrintBuffer) <= (size_t)written))
    {
        printd("PrintBuffer snprintf failure!\n");
        written = 0;
        PrintBuffer[0] = '\0';
    }

    char* p_printBuffer = PrintBuffer + written;
    size_t remaining = sizeof(PrintBuffer) - written;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        p_printBuffer,
        (DWORD)remaining,
        NULL
    );
    if (size <= 0)
    {
        snprintf(p_printBuffer, remaining,
            "Unknown Windows Error Code %d\n", error
        );
    }
    PrintBuffer[sizeof(PrintBuffer) - 1] = '\0';

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Error - GridLock",
        .dwStyle = MB_OK | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.error),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, MB_ICONERROR);
    MessageBoxIndirect(&messageBoxParams);

    return status;
}

//------------------------------------------------------------------------------
// Message_InitConfigExit
//------------------------------------------------------------------------------
status_t Message_InitConfigExit(const char* configurationFilename,
    const char* string, status_t status)
{
    assert(configurationFilename);
    assert(string);

    int written = snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock failed to start.\n"
        "\n"
        "Please check your\n"
        "\"%s\"\n"
        "file in your installation folder and try again.\n"
        "\n"
        "Error:\n"
        "%s\n",
        configurationFilename,
        string
    );
    if ((written < 0) || (sizeof(PrintBuffer) <= (size_t)written))
    {
        printd("PrintBuffer snprintf failure!\n");
        written = 0;
        PrintBuffer[0] = '\0';
    }
    PrintBuffer[sizeof(PrintBuffer) - 1] = '\0';

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Error - GridLock",
        .dwStyle = MB_OK | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.error),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, MB_ICONERROR);
    MessageBoxIndirect(&messageBoxParams);

    return status;
}

//------------------------------------------------------------------------------
// Message_InitConfigPrintErrorExit
//------------------------------------------------------------------------------
status_t Message_InitConfigPrintErrorExit(const char* configurationFilename,
    const char* prefix, int error)
{
    assert(configurationFilename);
    assert(prefix);

    int written = snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock encountered an error and failed to start.\n"
        "\n"
        "Please check your\n"
        "\"%s\"\n"
        "file in your installation folder and try again.\n"
        "\n"
        "Error:\n"
        "%s: ",
        configurationFilename,
        prefix
    );
    if ((written < 0) || (sizeof(PrintBuffer) <= (size_t)written))
    {
        printd("PrintBuffer snprintf failure!\n");
        written = 0;
        PrintBuffer[0] = '\0';
    }

    char* p_printBuffer = PrintBuffer + written;
    size_t remaining = sizeof(PrintBuffer) - written;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        p_printBuffer,
        remaining,
        NULL
    );
    if (size <= 0)
    {
        snprintf(p_printBuffer, remaining,
            "Unknown Windows Error Code %d\n", error
        );
    }
    PrintBuffer[sizeof(PrintBuffer) - 1] = '\0';

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Error - GridLock",
        .dwStyle = MB_OK | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.error),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, MB_ICONERROR);
    MessageBoxIndirect(&messageBoxParams);

    return (status_t)error;
}

//------------------------------------------------------------------------------
// Message_Welcome
//------------------------------------------------------------------------------
bool Message_Welcome(const char* configurationFilename)
{
    assert(configurationFilename);

    snprintf(PrintBuffer, sizeof(PrintBuffer),
        "Welcome to GridLock!\n"
        "A window organization utility for Windows\n"
        "\n"
        "GridLock detected a fresh installation.\n"
        "\n"
        "Initializing a default configuration:\n"
        "\"%s\"\n"
        "\n"
        "Select OK to continue, or Cancel to exit.",
        configurationFilename
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Welcome - GridLock",
        .dwStyle = MB_OKCANCEL | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.icon),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    int selection = MessageBoxIndirect(&messageBoxParams);
    if (IDCANCEL == selection)
    {
        PostQuitMessage(0);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Message_WarningHotkeyAlreadyRegistered
//------------------------------------------------------------------------------
void Message_WarningHotkeyAlreadyRegistered(const char* hotkey)
{
    assert(hotkey);

    Message_Warning(
        "The following hotkey was already registered:\n"
        "\n"
        "%s"
        "\n"
        "by Windows or another program.\n"
        "You can choose a new keybind in your GridLockConfig.yaml file.\n",
        hotkey
    );
}

//------------------------------------------------------------------------------
// Message_ErrorHotkeyRegistration
//------------------------------------------------------------------------------
void Message_ErrorHotkeyRegistration(const char* hotkey, int error)
{
    assert(hotkey);

    int written = snprintf(PrintBuffer, sizeof(PrintBuffer),
        "The following hotkey failed to be registered:\n"
        "\n"
        "%s"
        "\n"
        "This is not expected.\n"
        "You can choose a new keybind in your GridLockConfig.yaml file.\n"
        "\n"
        "Error: ",
        hotkey
    );
    if ((written < 0) || (sizeof(PrintBuffer) <= (size_t)written))
    {
        printd("PrintBuffer snprintf failure!\n");
        written = 0;
        PrintBuffer[0] = '\0';
    }

    char* p_printBuffer = PrintBuffer + written;
    size_t remaining = sizeof(PrintBuffer) - written;
    DWORD size = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        error,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        p_printBuffer,
        remaining,
        NULL
    );
    if (size <= 0)
    {
        snprintf(p_printBuffer, remaining,
            "Unknown Windows Error Code %d\n", error
        );
    }
    PrintBuffer[sizeof(PrintBuffer) - 1] = '\0';

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Error - GridLock",
        .dwStyle = MB_OK | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.error),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, MB_ICONERROR);
    MessageBoxIndirect(&messageBoxParams);
}

//------------------------------------------------------------------------------
// Message_NotepadFailed
//------------------------------------------------------------------------------
void Message_NotepadFailed(const char* configurationFilename)
{
    assert(configurationFilename);

    snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock could not open file \"%s\" with Notepad",
        configurationFilename
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Error - GridLock",
        .dwStyle = MB_OK | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.error),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, MB_ICONERROR);
    MessageBoxIndirect(&messageBoxParams);
}

//------------------------------------------------------------------------------
// Message_OpenFailed
//------------------------------------------------------------------------------
void Message_OpenFailed(const char* configurationFilename)
{
    assert(configurationFilename);

    snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock could not open file %s",
        configurationFilename
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Error - GridLock",
        .dwStyle = MB_OK | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.error),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    SoundLatch = HandleSound(SoundLatch, MB_ICONERROR);
    MessageBoxIndirect(&messageBoxParams);
}

//------------------------------------------------------------------------------
// Message_ConfirmResetToDefaults
//------------------------------------------------------------------------------
bool Message_RequestResetToDefaults(void)
{
    snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock failed to load your configuration file.\n"
        "\n"
        "Would you like to reset to defaults?\n"
        "\n"
        "Select OK to reset to defaults, or Cancel to exit GridLock."
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Request Reset to Defaults - GridLock",
        .dwStyle = MB_OKCANCEL | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.warning),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    int selection = MessageBoxIndirect(&messageBoxParams);
    if (IDCANCEL == selection)
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Message_ConfirmResetToDefaults
//------------------------------------------------------------------------------
bool Message_ConfirmResetToDefaults(const char* configurationFilename,
    const char* backupFilename)
{
    assert(configurationFilename);
    assert(backupFilename);

    snprintf(PrintBuffer, sizeof(PrintBuffer),
        "For GridLock to reset to defaults, it will overwrite the following configuration file:\n"
        "\"%s\"\n"
        "\n"
        "Don't worry - your active configuration will be saved as:\n"
        "\"%s\"\n"
        "\n"
        "Note - if a file already exists in the backup location, GridLock will overwrite that.\n"
        "\n"
        "Select OK to continue, or Cancel to abort this operation.",
        configurationFilename, backupFilename
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Confirm Reset to Defaults - GridLock",
        .dwStyle = MB_OKCANCEL | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.info),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    int selection = MessageBoxIndirect(&messageBoxParams);
    if (IDCANCEL == selection)
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Message_WarningBackupFailed
//------------------------------------------------------------------------------
void Message_WarningBackupFailed(const char* configurationFilename)
{
    assert(configurationFilename);

    Message_Warning(
        "GridLock could not backup your current configuration:\n"
        "\n"
        "\"%s\"\n"
        "\n"
        "Make sure that the file is closed and try again.\n",
        configurationFilename
    );
}

//------------------------------------------------------------------------------
// Message_NotifyResetToDefaults
//------------------------------------------------------------------------------
bool Message_NotifyResetToDefaults(void)
{
    snprintf(PrintBuffer, sizeof(PrintBuffer),
        "GridLock successfully saved a backup of your active configuration and"
        " reset it to defaults.\n"
        "\n"
        "A restart of GridLock is needed for the changes to take effect.\n"
        "\n"
        "Select OK to restart, or Cancel to continue without restarting."
    );

    MSGBOXPARAMS messageBoxParams =
    {
        .cbSize = sizeof(MSGBOXPARAMS),
        .hwndOwner = NULL,
        .hInstance = GetModuleHandle(NULL),
        .lpszText = PrintBuffer,
        .lpszCaption = "Confirm Reset to Defaults - GridLock",
        .dwStyle = MB_OKCANCEL | MB_USERICON,
        .lpszIcon = MAKEINTRESOURCEA(IconIds.icon),
        .dwContextHelpId = 0,
        .lpfnMsgBoxCallback = NULL,
        .dwLanguageId = MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    };

    int selection = MessageBoxIndirect(&messageBoxParams);
    if (IDCANCEL == selection)
    {
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// Message_Help
//------------------------------------------------------------------------------
status_t Message_Help(const char* version, const char* gridlockFilepath,
    const char* configurationFilename)
{
    assert(version);
    assert(gridlockFilepath);
    assert(configurationFilename);

    return MessageShowAbout("Help - GridLock", IconIds.icon, true,
        version, gridlockFilepath, configurationFilename
    );
}

//------------------------------------------------------------------------------
// Message_About
//------------------------------------------------------------------------------
status_t Message_About(const char* version, const char* gridlockFilepath,
    const char* configurationFilename)
{
    assert(version);
    assert(gridlockFilepath);
    assert(configurationFilename);

    return MessageShowAbout("About - GridLock", IconIds.icon, false,
        version, gridlockFilepath, configurationFilename
    );
}

//------------------------------------------------------------------------------
// Message_Warning
//------------------------------------------------------------------------------
void Message_Warning(const char* format, ...)
{
    assert(format);

    va_list args;
    va_start(args, format);
    MessageShow(
        "Warning - GridLock", IconIds.warning, MB_ICONWARNING, format, args
    );
    va_end(args);
}

//------------------------------------------------------------------------------
// Message_Error
//------------------------------------------------------------------------------
void Message_Error(const char* format, ...)
{
    assert(format);

    va_list args;
    va_start(args, format);
    MessageShow("Error - GridLock", IconIds.error, MB_ICONERROR, format, args);
    va_end(args);
}


/* Global Callback Function Implementations ==================================*/
