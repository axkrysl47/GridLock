/*==============================================================================
File name   : Config.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the high-level configuration interface for
              GridLock.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>

#include <yaml.h>
#include <windows.h>

#include "Config.h"

#include "Defaults.h"
#include "Parser.h"
#include "Verifier.h"
#include "Message.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define BreakIfFound(condition) \
    if (condition) { break; }

#define printdIf(condition, ...) \
    do { if (condition) { printd(__VA_ARGS__); } } while(0)

//------------------------------------------------------------------------------
typedef enum
{
    INIT,
    KEY,
    VALUE,
    KEY_VALUE_LIST_KEY,
    KEY_VALUE_LIST_VALUE,
    VALUE_LIST_VALUE,
    DONE
}
ParserState_t;


/* Private Member Variables ==================================================*/
//------------------------------------------------------------------------------
static char HotkeyPrintBuffer[
    sizeof(
        "Modifiers = [ Ctrl Win Alt Shift ]\n"
        "Key = [  ]\n"
        "Action = [ Invalid window action ]\n"
    )
    + 256
];
static char KeymapPrintBuffer[KEYMAP_MAX_DEPTH * KEYMAP_MAX_WIDTH + 1];


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline void ClearHotkey(Hotkey_t* p_hotkey);
static inline void ClearHotkeyAction(HotkeyAction_t* p_action);

//------------------------------------------------------------------------------
static inline bool IsEmptyHotkey(Hotkey_t* p_hotkey);
static inline bool IsEndOfHotkey(int8_t depth, Keymap_t* p_keymap,
    Hotkey_t* p_hotkey
);

//------------------------------------------------------------------------------
static inline bool VerifyHotkeyAction(HotkeyAction_t* p_action,
    uint8_t defaultIndex, bool enableAllBelow, bool enableThis,
    Hotkey_t* p_hotkeyToWarn
);

//------------------------------------------------------------------------------
static inline void UserWarningUnknownKeymap(int8_t depth, Keymap_t* p_keymap,
    const char* p_value
);
static inline const char* StringPrintYamlKeymap(int8_t depth,
    Keymap_t* p_keymap
);
static inline const char* StringPrintHotkeyKeybind(
    const HotkeyKeybind_t* p_keybind
);
static inline status_t UserErrorFailedParse(int8_t depth, Keymap_t* p_keymap,
    const HotkeyKeybind_t* p_keybind, const char* string
);

//------------------------------------------------------------------------------
static status_t ParseConfig(FILE* file, ConfigInitialPass_t* p_config,
    uint16_t n_hotkeys, Hotkey_t* p_hotkeys,
    uint16_t n_sequences, HotkeyAction_t* p_sequences, bool warn
);


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline void ClearHotkey(Hotkey_t* p_hotkey)
{
    assert(p_hotkey);

    p_hotkey->keybind.modifiers = 0u;
    p_hotkey->keybind.key = 0u;
    p_hotkey->action.p_function = NULL;
    memset(&(p_hotkey->action.param), 0u, sizeof(p_hotkey->action.param));
}

//------------------------------------------------------------------------------
static inline void ClearHotkeyAction(HotkeyAction_t* p_action)
{
    assert(p_action);

    p_action->p_function = NULL;
    memset(&(p_action->param), 0u, sizeof(p_action->param));
}

//------------------------------------------------------------------------------
static inline bool IsEmptyHotkey(Hotkey_t* p_hotkey)
{
    assert(p_hotkey);

    if (p_hotkey->keybind.modifiers != 0u) { return false; }
    if (p_hotkey->keybind.key != 0u) { return false; }
    if (p_hotkey->action.p_function != NULL) { return false; }

    const char* p_bytes = (const char*)(&(p_hotkey->action.param));
    for (size_t i = 0; i < sizeof(p_hotkey->action.param); i++)
    {
        if (0u != p_bytes[i]) { return false; }
    }

    return true;
}

//------------------------------------------------------------------------------
static inline bool IsEndOfHotkey(int8_t depth, Keymap_t* p_keymap,
    Hotkey_t* p_hotkey)
{
    assert(p_keymap);
    assert(p_hotkey);

    if (Keymap_IsEndOfEnable(depth, p_keymap)
        && IsEmptyHotkey(p_hotkey))
    {
        return false;
    }

    return Keymap_IsEndOfHotkey(depth, p_keymap);
}

//------------------------------------------------------------------------------
static inline bool VerifyHotkeyAction(HotkeyAction_t* p_action,
    uint8_t defaultIndex, bool enableAllBelow, bool enableThis,
    Hotkey_t* p_hotkeyToWarn)
{
    assert(p_action);

    const bool warn = (NULL != p_hotkeyToWarn);
    if (enableAllBelow && (false == enableThis))
    {
        printdIf(warn, "<Hotkey sequence was manually disabled>\n");
        return false;
    }

    if ((false == enableAllBelow) && (false == enableThis))
    {
        printdIf(warn, "<Hotkey sequence was disabled below all>\n");
        return false;
    }

    SacReturnIfErr(
        Verifier_HotkeyAction(p_action, defaultIndex, p_hotkeyToWarn),
        false
    );

    return true;
}

//------------------------------------------------------------------------------
static inline void UserWarningUnknownKeymap(int8_t depth, Keymap_t* p_keymap,
    const char* p_value)
{
    assert(p_keymap);
    assert(p_value);

    Message_Warning(
        "Unrecognized field %s at depth %d with value:\n"
        "\n"
        "\"%s\"\n"
        "\n"
        "%s",
        p_keymap->keys[depth - 1], depth, p_value,
        StringPrintYamlKeymap(depth, p_keymap)
    );
}

//------------------------------------------------------------------------------
static inline const char* StringPrintYamlKeymap(int8_t depth,
    Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    char* p_buffer = KeymapPrintBuffer;
    const char* p_buffer_end = KeymapPrintBuffer + sizeof(KeymapPrintBuffer);

    for (int_fast8_t i = 0; i < depth; i++)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);

        int written = ('-' == p_keymap->keys[i][0])
            ? snprintf(p_buffer, remaining, " - ")
            : snprintf(p_buffer, remaining, "%s\n", p_keymap->keys[i]);

        if (written < 0 || remaining <= (size_t)written)
        {
            return KeymapPrintBuffer;
        }

        p_buffer += written;
    }

    return KeymapPrintBuffer;
}

//------------------------------------------------------------------------------
static inline const char* StringPrintHotkeyKeybind(
    const HotkeyKeybind_t* p_keybind)
{
    assert(p_keybind);

    Utility_StringCopy(
        HotkeyPrintBuffer, "Modifiers = [ ", sizeof(HotkeyPrintBuffer)
    );

    const char* p_buffer_start =
        HotkeyPrintBuffer + sizeof("Modifiers = [ ") - 1;
    char* p_buffer = (char*)p_buffer_start;
    const char* p_buffer_end = HotkeyPrintBuffer + sizeof(HotkeyPrintBuffer);

    if (MOD_CONTROL & p_keybind->modifiers)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written = snprintf(p_buffer, remaining, "Ctrl");
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }
    if (MOD_WIN & p_keybind->modifiers)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written =
            snprintf(p_buffer, remaining, "%sWin",
                (p_buffer_start == p_buffer) ? "" : " "
            );
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }
    if (MOD_ALT & p_keybind->modifiers)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written =
            snprintf(p_buffer, remaining, "%sAlt",
                (p_buffer_start == p_buffer) ? "" : " "
            );
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }
    if (MOD_SHIFT & p_keybind->modifiers)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written =
            snprintf(p_buffer, remaining, "%sShift",
                (p_buffer_start == p_buffer) ? "" : " "
            );
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }
    if (p_buffer_start == p_buffer)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written = snprintf(p_buffer, remaining, "None");
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }
    if (~(MOD_CONTROL | MOD_WIN | MOD_ALT | MOD_SHIFT) & p_keybind->modifiers)
    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written =
            snprintf(p_buffer, remaining, "%s[Invalid Modifier]",
                (p_buffer_start == p_buffer) ? "" : " "
            );
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }

    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written = snprintf(p_buffer, remaining, " ]\nKey = [ ");
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }

    unsigned int scanCode = MapVirtualKeyA(p_keybind->key, MAPVK_VK_TO_VSC);
    long lParamValue = (scanCode << 16);
    if ((VK_PRIOR <= p_keybind->key) && (p_keybind->key <= VK_DOWN))
    {
        lParamValue |= (1 << 24);
    }

    {
        const int remaining = (int)(p_buffer_end - p_buffer);
        int written = GetKeyNameTextA(lParamValue, p_buffer, remaining);
        if (0 == written)
        {
            written = snprintf(p_buffer, remaining,
                "Error - unable to translate key name"
            );
            if (written < 0 || remaining <= written)
            {
                return HotkeyPrintBuffer;
            }
        }

        p_buffer += written;
    }


    {
        const size_t remaining = (size_t)(p_buffer_end - p_buffer);
        int written = snprintf(p_buffer, remaining, " ]\n");
        if (written < 0 || remaining <= (size_t)written)
        {
            return HotkeyPrintBuffer;
        }

        p_buffer += written;
    }

    return HotkeyPrintBuffer;
}

//------------------------------------------------------------------------------
static inline status_t UserErrorFailedParse(int8_t depth, Keymap_t* p_keymap,
    const HotkeyKeybind_t* p_keybind, const char* string)
{
    assert(p_keymap);
    assert(p_keybind);
    assert(string);

    Message_Error(
        "Error while parsing configuration file.\n"
        "%s\n"
        "\n"
        "Error occurred around configuration:\n"
        "%s\n"
        "Error occurred around hotkey keybind:\n"
        "%s\n"
        "You can update your configuration file and try again.\n",
        string,
        StringPrintYamlKeymap(depth, p_keymap),
        StringPrintHotkeyKeybind(p_keybind)
    );
    return STATUS_FAILURE;
}

//------------------------------------------------------------------------------
static status_t ParseConfig(FILE* file, ConfigInitialPass_t* p_config,
    uint16_t n_hotkeys, Hotkey_t* p_hotkeys,
    uint16_t n_sequences, HotkeyAction_t* p_sequences, bool warn)
{
    assert(file);

    yaml_parser_t parser;
    int status = yaml_parser_initialize(&parser);
    SacPrintReturnIf(
        (0 == status), STATUS_FAILURE, "Failed to initialize parser\n"
    );

    yaml_parser_set_input_file(&parser, file);

    Hotkey_t hotkey;
    ClearHotkey(&hotkey);

    HotkeyAction_t sequenceAction;
    ClearHotkeyAction(&sequenceAction);

    Keymap_t keymap = {0}; // initialized to all '\0' //
    int8_t depth = 0;
    uint16_t i_hotkey = 0;
    uint16_t i_sequenceStart = 0;
    uint16_t i_sequence = 0;
    uint8_t defaultIndex = 0;
    ParserState_t state = INIT;
    bool enableAllBelow = true;
    bool enableThis = true;

    // Used for failure analysis. //
    // Holds the last seen data, even if a new hotkey has been traversed. //
    HotkeyKeybind_t tempKeybind = { .modifiers = 0, .key = 0 };
    bool tempKeybindModifiersSet = false;
    bool tempKeybindKeySet = false;
    HotkeyKeybind_t lastKeybind = { .modifiers = 0, .key = 0 };

    while (DONE != state)
    {
        yaml_event_t event;
        status = yaml_parser_parse(&parser, &event);
        if (1 != status)
        {
            printd("Parser error: %d\n", parser.error);
            yaml_parser_delete(&parser);
            return UserErrorFailedParse(depth, &keymap, &lastKeybind,
                "This usually results from invalid YAML syntax."
            );
        }

        switch (event.type)
        {

        case (YAML_STREAM_START_EVENT):
        case (YAML_DOCUMENT_START_EVENT):
        case (YAML_DOCUMENT_END_EVENT):
        break;

        case (YAML_NO_EVENT):
        case (YAML_STREAM_END_EVENT):
        {
            state = DONE;
        }
        break;

        case (YAML_ALIAS_EVENT):
        {
            printdIf(warn, "%*sAlias event\n", depth, "");
        }
        break;

        case (YAML_SCALAR_EVENT):
        {
            const char* p_value = (const char*)event.data.scalar.value;

            switch (state)
            {

            default:
            {
                printdIf(warn, "Invalid state!\n");
                yaml_event_delete(&event);
                yaml_parser_delete(&parser);
            }
            return UserErrorFailedParse(depth, &keymap, &lastKeybind,
                "GridLock parser logic failed."
            );

            case (KEY):
            {
                Keymap_SetKey(depth, &keymap, p_value);
                state = VALUE;

                if (warn && NULL == hotkey.action.p_function
                    && Keymap_IsHotkeyParameter(depth, &keymap))
                {
                    Config_UserWarning(&hotkey,
                        "Parameters must appear after the action declaration."
                    );
                }
            }
            break;

            case (VALUE):
            {
                printdIf(warn, "%*s%s = %.32s\n",
                    depth, "", keymap.keys[depth - 1], p_value
                );

                do // Using a do-while loop to allow for clean breaks //
                {
                    if (NULL != p_config)
                    {
                        BreakIfFound(
                            Parser_InitialPassValue(
                                depth, &keymap, p_value, p_config
                            )
                        );
                    }

                    BreakIfFound(
                        Parser_EnableDisableValue(
                            depth, &keymap, p_value, &enableAllBelow,
                            &enableThis
                        )
                    );

                    ParserHotkeySequenceInput_t input =
                    {
                        .p_sequence = &sequenceAction,
                        .p_hotkey = &hotkey,
                        .p_sequenceStart = (NULL == p_sequences)
                            ? &sequenceAction : &(p_sequences[i_sequence])
                    };
                    BreakIfFound(
                        Parser_HotkeySequenceAction(
                            depth, &keymap, p_value, &input, &defaultIndex,
                            warn
                        )
                    );

                    BreakIfFound(
                        Parser_HotkeyAction(
                            depth, &keymap, p_value, &hotkey, &defaultIndex,
                            warn
                        )
                    );

                    BreakIfFound(
                        Parser_SequenceValue(
                            depth, &keymap, p_value, &sequenceAction
                        )
                    );

                    if (Keymap_KeybindValue(
                            depth, &keymap, p_value, &(hotkey.keybind.key)))
                    {
                        tempKeybind.key = hotkey.keybind.key;
                        tempKeybindKeySet = true;
                        if (tempKeybindModifiersSet && tempKeybindKeySet)
                        {
                            lastKeybind = tempKeybind;
                            tempKeybindModifiersSet = false;
                            tempKeybindKeySet = false;
                        }
                        break;
                    }

                    BreakIfFound(
                        Parser_HotkeyValue(depth, &keymap, p_value, &hotkey)
                    );

                    if (warn)
                    {
                        UserWarningUnknownKeymap(depth, &keymap, p_value);
                    }
                }
                while (false); // Never actually loop //

                state = KEY;
            }
            break;

            case (KEY_VALUE_LIST_KEY):
            {
                Keymap_SetKey(depth, &keymap, p_value);
                state = KEY_VALUE_LIST_VALUE;
            }
            break;

            case (KEY_VALUE_LIST_VALUE):
            {
                printdIf(warn, "%*s%s = %.32s\n",
                    depth, "", keymap.keys[depth - 1], p_value
                );
                state = KEY_VALUE_LIST_KEY;
            }
            break;

            case (VALUE_LIST_VALUE):
            {
                printdIf(warn, "%*s%s = %.32s\n",
                    depth, "", keymap.keys[depth - 1], p_value
                );

                ParserHotkeySequenceInput_t input =
                {
                    .p_sequence = &sequenceAction,
                    .p_hotkey = &hotkey,
                    .p_sequenceStart = (NULL == p_sequences)
                        ? &sequenceAction : &(p_sequences[i_sequence])
                };
                if (Parser_HotkeySequenceList(
                        depth, &keymap, p_value, &input, &defaultIndex, warn))
                {
                    if (false == VerifyHotkeyAction(
                            &sequenceAction, defaultIndex,
                            enableAllBelow, enableThis,
                            (warn) ? &hotkey : NULL
                        ))
                    {
                        if (warn)
                        {
                            Config_UserWarning(&hotkey,
                                "Sequence was invalid, skipping"
                            );
                        }
                    }
                    else
                    {
                        if (n_sequences <= i_sequence)
                        {
                            yaml_event_delete(&event);
                            yaml_parser_delete(&parser);
                            return UserErrorFailedParse(
                                depth, &keymap, &lastKeybind,
                                "Sequence count exceeded allocated capacity."
                            );
                        }

                        if (UINT16_MAX == hotkey.action.param.asSequence.count)
                        {
                            if (warn)
                            {
                                Config_UserWarning(&hotkey,
                                    "A sequence action can only store up to"
                                    " 65,535 sequences; skipping"
                                );
                            }
                        }
                        else
                        {
                            hotkey.action.param.asSequence.count++;
                            if (NULL != p_sequences)
                            {
                                p_sequences[i_sequence] = sequenceAction;
                            }
                            i_sequence++;
                            printdIf(warn, "<Sequence Count = %d>\n", i_sequence);
                        }
                    }

                    ClearHotkeyAction(&sequenceAction);
                    break;
                }

                BreakIfFound(
                    Parser_SequenceValueList(
                        depth, &keymap, p_value, &sequenceAction
                    )
                );

                if (Keymap_ModifierValueList(
                        depth, &keymap, p_value, &(hotkey.keybind.modifiers)))
                {
                    tempKeybind.modifiers = hotkey.keybind.modifiers;
                    tempKeybindModifiersSet = true;
                    if (tempKeybindModifiersSet && tempKeybindKeySet)
                    {
                        lastKeybind = tempKeybind;
                        tempKeybindModifiersSet = false;
                        tempKeybindKeySet = false;
                    }
                    break;
                }

                BreakIfFound(
                    Parser_HotkeyValueList(depth, &keymap, p_value, &hotkey)
                );

                if (warn)
                {
                    UserWarningUnknownKeymap(depth, &keymap, p_value);
                }
            }
            break;

            }
            // end switch //
        }
        break;

        case (YAML_SEQUENCE_START_EVENT):
        {
            if (KEYMAP_MAX_DEPTH <= (depth + 1))
            {
                printd("Invalid YAML! Exceeded maximum depth array limit.\n");
                yaml_event_delete(&event);
                yaml_parser_delete(&parser);
                return UserErrorFailedParse(depth, &keymap, &lastKeybind,
                    "YAML depth exceeded the maximum limit."
                );
            }

            if (KEY == state)
            {
                state = KEY_VALUE_LIST_KEY;
                printdIf(warn, "%*sList of key-values for %s [\n",
                    depth, "", keymap.keys[depth - 1]
                );
            }
            else
            {
                state = VALUE_LIST_VALUE;
                printdIf(warn, "%*sList of values for %s [\n",
                    depth, "", keymap.keys[depth - 1]
                );
            }

            depth++;
            keymap.keys[depth - 1][0] = '-';
            keymap.keys[depth - 1][1] = '\0';
        }
        break;

        case (YAML_SEQUENCE_END_EVENT):
        {
            printdIf(warn, "%*s]\n", depth - 1, "");
            keymap.keys[depth - 1][0] = '\0';
            depth--;
            state = ((0 < depth) && ('-' == keymap.keys[depth - 1][0]))
                ? VALUE_LIST_VALUE : KEY;
        }
        break;

        case (YAML_MAPPING_START_EVENT):
        {
            if (KEYMAP_MAX_DEPTH <= (depth + 1))
            {
                printd("Invalid YAML! Exceeded maximum depth array limit.\n");
                yaml_event_delete(&event);
                yaml_parser_delete(&parser);
                return UserErrorFailedParse(depth, &keymap, &lastKeybind,
                    "YAML depth exceeded the maximum limit."
                );
            }

            if ((VALUE_LIST_VALUE == state) || (KEY_VALUE_LIST_VALUE == state))
            {
                printdIf(warn, "%*sSequence of {\n", depth, "");
            }
            else
            {
                if (0 < depth)
                {
                    printdIf(warn, "%*sMap of %s {\n",
                        depth, "", keymap.keys[depth - 1]
                    );
                }
                else
                {
                    printdIf(warn, "%*sMap {\n", depth, "");
                }
            }

            state = KEY;
            keymap.keys[depth][0] = '\0';
            depth++;
        }
        break;

        case (YAML_MAPPING_END_EVENT):
        {
            if (Keymap_IsEndOfHotkeySequenceAction(depth, &keymap))
            {
                if (false == VerifyHotkeyAction(
                        &sequenceAction, defaultIndex,
                        enableAllBelow, enableThis,
                        &hotkey))
                {
                    if (warn)
                    {
                        Config_UserWarning(&hotkey,
                            "Sequence was invalid, skipping"
                        );
                    }
                }
                else
                {
                    if (n_sequences <= i_sequence)
                    {
                        yaml_event_delete(&event);
                        yaml_parser_delete(&parser);
                        return UserErrorFailedParse(
                            depth, &keymap, &lastKeybind,
                            "Sequence count exceeded allocated capacity."
                        );
                    }

                    if (UINT16_MAX == hotkey.action.param.asSequence.count)
                    {
                        if (warn)
                        {
                            Config_UserWarning(&hotkey,
                                "A sequence action can only store up to 65,535"
                                " sequences; skipping"
                            );
                        }
                    }
                    else
                    {
                        hotkey.action.param.asSequence.count++;
                        if (NULL != p_sequences)
                        {
                            p_sequences[i_sequence] = sequenceAction;
                        }
                        i_sequence++;
                        printdIf(warn, "<Sequence Count = %d>\n", i_sequence);
                    }
                }

                ClearHotkeyAction(&sequenceAction);
            }

            if (IsEndOfHotkey(depth, &keymap, &hotkey))
            {
                if (false == VerifyHotkeyAction(
                        &hotkey.action,
                        defaultIndex,
                        enableAllBelow, enableThis,
                        (warn) ? &hotkey : NULL
                    ))
                {
                    i_sequence = i_sequenceStart;
                }
                else // Hotkey verified //
                {
                    if (n_hotkeys <= i_hotkey)
                    {
                        Config_UserWarning(
                            &hotkey,
                            "Exceeded maximum allowed hotkey capacity!"
                        );
                        yaml_event_delete(&event);
                        yaml_parser_delete(&parser);
                        return UserErrorFailedParse(
                            depth, &keymap, &lastKeybind,
                            "Hotkey count exceeded allocated capacity."
                        );
                    }

                    if (NULL != p_hotkeys)
                    {
                        p_hotkeys[i_hotkey] = hotkey;
                    }
                    i_hotkey++;
                    i_sequenceStart = i_sequence;
                    printdIf(warn, "<Hotkey Count = %d>\n", i_hotkey);
                }

                enableThis = enableAllBelow;
                ClearHotkey(&hotkey);
            }

            depth--;
            printdIf(warn, "%*s}\n", depth, "");
            keymap.keys[depth][0] = '\0';
            state = ((0 < depth) && ('-' == keymap.keys[depth - 1][0]))
                ? VALUE_LIST_VALUE : KEY;
        }
        break;

        default:
        {
            printdIf(warn, "%*sUnknown YAML event = %d!\n",
                depth, "", event.type
            );
        }
        break;

        }
        // end switch //

        yaml_event_delete(&event);
    }

    yaml_parser_delete(&parser);

    if ((NULL != p_hotkeys) && (n_hotkeys != i_hotkey))
    {
        printd("Second parsing didn't match length of first pass...\n");
        Message_Error(
            "GridLock encountered an internal error with hotkey count\n\n"
        );
        return STATUS_FAILURE;
    }
    if ((NULL != p_sequences) && (n_sequences != i_sequence))
    {
        printd(
            "Second sequence parsing didn't match length of first pass...\n"
        );
        Message_Error(
            "GridLock encountered an internal error with sequence count\n\n"
        );
        return STATUS_FAILURE;
    }

    if (NULL != p_config)
    {
        p_config->n_hotkeys = i_hotkey;
        p_config->n_sequences = i_sequence;
    }

    return STATUS_SUCCESS;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Config_InitialPass
//------------------------------------------------------------------------------
status_t Config_InitialPass(FILE* file, ConfigInitialPass_t* p_config)
{
    assert(file);
    assert(p_config);

    printd("Initial pass of configuration file\n");
    const bool warn = true;
    return
        ParseConfig(file, p_config, UINT16_MAX, NULL, UINT16_MAX, NULL, warn);
}

//------------------------------------------------------------------------------
// Config_ParseHotkeys
//------------------------------------------------------------------------------
status_t Config_ParseHotkeys(FILE* file,
    uint16_t n_hotkeys, Hotkey_t* p_hotkeys,
    uint16_t n_sequences, HotkeyAction_t* p_sequences)
{
    assert(file);
    assert(p_hotkeys);
    assert((0 == n_sequences) || p_sequences);

    const bool warn = false;
    status_t status = ParseConfig(
        file, NULL, n_hotkeys, p_hotkeys, n_sequences, p_sequences, warn
    );
    SacPrintReturnIfErr(status, status, "Final parse of config failed.\n");

    printd("Hotkey parsing complete\n\n");
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Config_StringPrintHotkey
//------------------------------------------------------------------------------
const char* Config_StringPrintHotkey(const Hotkey_t* p_hotkey)
{
    assert(p_hotkey);

#ifdef NDEBUG
    StringPrintHotkeyKeybind(&(p_hotkey->keybind));
#else
    const char* p_string_start = StringPrintHotkeyKeybind(&(p_hotkey->keybind));
    assert(p_string_start == HotkeyPrintBuffer);
#endif

    char* p_string = HotkeyPrintBuffer;
    while ('\0' != *p_string)
    {
        p_string++;
    }

    // snprintf result is ignored, only because no reaction is taken //
    const size_t remaining =
        (size_t)((HotkeyPrintBuffer + sizeof(HotkeyPrintBuffer)) - p_string);
    snprintf(p_string, remaining, "Action = [ %s ]\n",
        Defaults_GetActionNameByFunction(p_hotkey->action.p_function)
    );

    return HotkeyPrintBuffer;
}

//------------------------------------------------------------------------------
// Config_UserWarning
//------------------------------------------------------------------------------
void Config_UserWarning(const Hotkey_t* p_hotkey, const char* string)
{
    assert(p_hotkey);
    assert(string);

    // Assumes that Message consumes the string parameter immediately //

    Message_Warning(
        "Invalid configuration of\n"
        "\n"
        "%s"
        "\n"
        "%s\n"
        "You can update your GridLockConfig.yaml file and try again.\n",
        Config_StringPrintHotkey(p_hotkey), string
    );
}


/* Global Callback Function Implementations ==================================*/
