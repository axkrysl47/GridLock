/*==============================================================================
File name   : Parser.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines functions that parse a given section of a YAML
              GridLock configuration file.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "Parser.h"

#include "Defaults.h"
#include "Verifier.h"
#include "Config.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define ReturnTrueIfFound(condition) \
    do { if (condition) { return true; } } while(0)


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline void ResetHotkeyAction(HotkeyAction_t* p_action);

//------------------------------------------------------------------------------
static inline bool ParseActionValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_action, bool isSequence
);
static inline bool ParseActionValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_action, bool isSequence
);
static inline bool ParseSequenceAction(const char* p_value,
    ParserHotkeySequenceInput_t* p_input, uint8_t* p_defaultIndex, bool warn
);

/* Private Member Variables ==================================================*/


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline void ResetHotkeyAction(HotkeyAction_t* p_action)
{
    assert(p_action);

    if (NULL == p_action->p_function)
    {
        return;
    }

    if (Action_Help == p_action->p_function
        || Action_Revert == p_action->p_function)
    {
        // Action_Help and Action_Revert carry no parameters //
        return;
    }

    if (Action_Minimize == p_action->p_function)
    {
        p_action->param.asMinimize.toggle = VALUE_INT8_UNSET;
        return;
    }

    if (Action_Maximize == p_action->p_function)
    {
        p_action->param.asMaximize.toggle = VALUE_INT8_UNSET;
        return;
    }

    if (Action_Tile == p_action->p_function)
    {
        p_action->param.asTile.inlay.type = INPUTTYPE_UNKNOWN;
        p_action->param.asTile.inlay.dualratio.width.numerator =
            VALUE_INT32_UNSET;
        p_action->param.asTile.inlay.dualratio.width.denominator =
            VALUE_INT32_UNSET;
        p_action->param.asTile.inlay.dualratio.height.numerator =
            VALUE_INT32_UNSET;
        p_action->param.asTile.inlay.dualratio.height.denominator =
            VALUE_INT32_UNSET;
        p_action->param.asTile.columns   = VALUE_INT8_UNSET;
        p_action->param.asTile.rows      = VALUE_INT8_UNSET;
        p_action->param.asTile.index     = VALUE_INT8_UNSET;
        p_action->param.asTile.width     = VALUE_INT8_UNSET;
        p_action->param.asTile.height    = VALUE_INT8_UNSET;
        p_action->param.asTile.transpose = VALUE_INT8_UNSET;
        p_action->param.asTile.jump      = VALUE_INT8_UNSET;
        return;
    }

    if (Action_Move == p_action->p_function)
    {
        p_action->param.asMove.horizontal.numerator   = VALUE_INT32_UNSET;
        p_action->param.asMove.horizontal.denominator = VALUE_INT32_UNSET;
        p_action->param.asMove.vertical.numerator     = VALUE_INT32_UNSET;
        p_action->param.asMove.vertical.denominator   = VALUE_INT32_UNSET;
        p_action->param.asMove.jump                   = VALUE_INT8_UNSET;
        return;
    }

    if (Action_Jump == p_action->p_function)
    {
        p_action->param.asJump.direction = JUMP_DISPLAY_UNKNOWN;
        return;
    }

    if (Action_Increase == p_action->p_function)
    {
        p_action->param.asIncrease.resize.type = INPUTTYPE_UNKNOWN;
        p_action->param.asIncrease.resize.dualratio.width.numerator =
            VALUE_INT32_UNSET;
        p_action->param.asIncrease.resize.dualratio.width.denominator =
            VALUE_INT32_UNSET;
        p_action->param.asIncrease.resize.dualratio.height.numerator =
            VALUE_INT32_UNSET;
        p_action->param.asIncrease.resize.dualratio.height.denominator =
            VALUE_INT32_UNSET;
        return;
    }

    if (Action_Decrease == p_action->p_function)
    {
        p_action->param.asDecrease.resize.type = INPUTTYPE_UNKNOWN;
        p_action->param.asDecrease.resize.dualratio.width.numerator =
            VALUE_INT32_UNSET;
        p_action->param.asDecrease.resize.dualratio.width.denominator =
            VALUE_INT32_UNSET;
        p_action->param.asDecrease.resize.dualratio.height.numerator =
            VALUE_INT32_UNSET;
        p_action->param.asDecrease.resize.dualratio.height.denominator =
            VALUE_INT32_UNSET;
        p_action->param.asDecrease.edgelock = VALUE_INT8_UNSET;
        p_action->param.asDecrease.restore  = VALUE_INT8_UNSET;
        return;
    }

    if (Action_Project == p_action->p_function)
    {
        p_action->param.asProject.mode = PROJECT_DISPLAY_UNKNOWN;
        return;
    }

    if (Action_Sequence == p_action->p_function)
    {
        p_action->param.asSequence.count = 0;
        p_action->param.asSequence.p_actions = NULL;
        return;
    }
}

//------------------------------------------------------------------------------
static inline bool ParseActionValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_action, bool isSequence)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_action);

    if (Action_Minimize == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_MinimizeValue(depth, p_keymap, p_value,
                &(p_action->param.asMinimize), isSequence
            )
        );
    }

    if (Action_Maximize == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_MaximizeValue(depth, p_keymap, p_value,
                &(p_action->param.asMaximize), isSequence
            )
        );
    }

    if (Action_Tile == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_TileParameter(depth, p_keymap, p_value,
                &(p_action->param.asTile), isSequence
            )
        );
        ReturnTrueIfFound(
            Keymap_TileInlay(depth, p_keymap, p_value,
                &(p_action->param.asTile.inlay), isSequence
            )
        );
        ReturnTrueIfFound(
            Keymap_TileInlayRatio(depth, p_keymap, p_value,
                &(p_action->param.asTile.inlay), isSequence
            )
        );
    }

    if (Action_Move == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_MoveValue(depth, p_keymap, p_value,
                &(p_action->param.asMove), isSequence
            )
        );
    }

    if (Action_Jump == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_JumpValue(depth, p_keymap, p_value,
                &(p_action->param.asJump), isSequence
            )
        );
    }

    if (Action_Increase == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_IncreaseParameter(depth, p_keymap, p_value,
                &(p_action->param.asIncrease), isSequence
            )
        );
        ReturnTrueIfFound(
            Keymap_IncreaseRatio(depth, p_keymap, p_value,
                &(p_action->param.asIncrease.resize), isSequence
            )
        );
    }

    if (Action_Decrease == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_DecreaseParameter(depth, p_keymap, p_value,
                &(p_action->param.asDecrease), isSequence
            )
        );
        ReturnTrueIfFound(
            Keymap_DecreaseRatio(depth, p_keymap, p_value,
                &(p_action->param.asDecrease.resize), isSequence
            )
        );
    }

    if (Action_Project == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_ProjectValue(depth, p_keymap, p_value,
                &(p_action->param.asProject), isSequence
            )
        );
    }

    return false;
}

//------------------------------------------------------------------------------
static inline bool ParseActionValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_action, bool isSequence)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_action);

    if (Action_Tile == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_TileGridList(depth, p_keymap, p_value,
                &(p_action->param.asTile), isSequence
            )
        );
        ReturnTrueIfFound(
            Keymap_TileSizeList(depth, p_keymap, p_value,
                &(p_action->param.asTile), isSequence
            )
        );
    }

    if (Action_Increase == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_IncreaseScalarList(depth, p_keymap, p_value,
                &(p_action->param.asIncrease.resize), isSequence
            )
        );
    }

    if (Action_Decrease == p_action->p_function)
    {
        ReturnTrueIfFound(
            Keymap_DecreaseScalarList(depth, p_keymap, p_value,
                &(p_action->param.asDecrease.resize), isSequence
            )
        );
    }

    return false;
}

//------------------------------------------------------------------------------
static inline bool ParseSequenceAction(const char* p_value,
    ParserHotkeySequenceInput_t* p_input, uint8_t* p_defaultIndex, bool warn)
{
    assert(p_value);
    assert(p_input);
    assert(p_input->p_sequence);
    assert(p_input->p_hotkey);
    assert(p_input->p_sequenceStart);
    assert(p_defaultIndex);

    if (NULL != p_input->p_hotkey->action.p_function
        && Action_Sequence != p_input->p_hotkey->action.p_function)
    {
        p_input->p_hotkey->action.p_function = INVALID_ACTION;
        if (warn)
        {
            Config_UserWarning(p_input->p_hotkey,
                "You cannot mix an action & a sequence on the same hotkey");
        }
        return true;
    }

    if (NULL == p_input->p_hotkey->action.p_function)
    {
        p_input->p_hotkey->action.p_function = Action_Sequence;
        p_input->p_hotkey->action.param.asSequence.p_actions =
            p_input->p_sequenceStart;
        p_input->p_hotkey->action.param.asSequence.count = 0;
    }

    uint8_t index = 0;
    if (false == Defaults_GetActionIndexByName(p_value, &index))
    {
        if (warn)
        {
            Config_UserWarning(p_input->p_hotkey,
                "Unknown sequenced action name"
            );
        }
        *p_defaultIndex = INVALID_DEFAULT_INDEX;
        return true;
    }

    *p_defaultIndex = index;
    p_input->p_sequence->p_function = DefaultActionMapping[index].action;
    ResetHotkeyAction(p_input->p_sequence);
    return true;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Parser_InitialPassValue
//------------------------------------------------------------------------------
bool Parser_InitialPassValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ConfigInitialPass_t* p_config)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_config);

    ReturnTrueIfFound(
        Keymap_VersionValue(depth, p_keymap, p_value, &(p_config->version))
    );
    ReturnTrueIfFound(
        Keymap_BorderVisibilityValue(depth, p_keymap, p_value,
            &(p_config->borderVisibility)
        )
    );

    ReturnTrueIfFound(
        Keymap_MinimumValue(depth, p_keymap, p_value,
            &(p_config->minimum)
        )
    );

    return false;
}

//------------------------------------------------------------------------------
// Parser_EnableDisableValue
//------------------------------------------------------------------------------
bool Parser_EnableDisableValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, bool* p_allBelow, bool* p_this)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_allBelow);
    assert(p_this);

    ReturnTrueIfFound(
        Keymap_EnableDisableValue(depth, p_keymap, p_value, p_allBelow, p_this)
    );

    return false;
}

//------------------------------------------------------------------------------
// Parser_HotkeyValue
//------------------------------------------------------------------------------
bool Parser_HotkeyValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, Hotkey_t* p_hotkey)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_hotkey);

    const bool isSequence = false;
    return ParseActionValue(
        depth, p_keymap, p_value, &(p_hotkey->action), isSequence
    );
}

//------------------------------------------------------------------------------
// Parser_HotkeyValueList
//------------------------------------------------------------------------------
bool Parser_HotkeyValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, Hotkey_t* p_hotkey)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_hotkey);

    const bool isSequence = false;
    return ParseActionValueList(
        depth, p_keymap, p_value, &(p_hotkey->action), isSequence
    );
}

//------------------------------------------------------------------------------
// Parser_SequenceValue
//------------------------------------------------------------------------------
bool Parser_SequenceValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_sequence)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_sequence);

    const bool isSequence = true;
    return ParseActionValue(depth, p_keymap, p_value, p_sequence, isSequence);
}

//------------------------------------------------------------------------------
// Parser_SequenceValueList
//------------------------------------------------------------------------------
bool Parser_SequenceValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_sequence)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_sequence);

    const bool isSequence = true;
    return ParseActionValueList(
        depth, p_keymap, p_value, p_sequence, isSequence
    );
}

//------------------------------------------------------------------------------
// Parser_HotkeyAction
//------------------------------------------------------------------------------
bool Parser_HotkeyAction(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, Hotkey_t* p_hotkey, uint8_t* p_defaultIndex, bool warn)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_hotkey);
    assert(p_defaultIndex);

    if (Keymap_IsHotkeyAction(depth, p_keymap))
    {
        uint8_t index = 0;
        if (false == Defaults_GetActionIndexByName(p_value, &index))
        {
            if (warn)
            {
                Config_UserWarning(p_hotkey, "Unknown window action name");
            }
            *p_defaultIndex = INVALID_DEFAULT_INDEX;
            return true;
        }

        if (NULL != p_hotkey->action.p_function)
        {
            const bool wasSequence =
                (Action_Sequence == p_hotkey->action.p_function);
            const bool isSequence =
                (Action_Sequence == DefaultActionMapping[index].action);
            if (wasSequence != isSequence)
            {
                p_hotkey->action.p_function = INVALID_ACTION;
            }

            if (warn)
            {
                Config_UserWarning(p_hotkey,
                    "Hotkey already has an action, ignoring duplicate"
                );
            }
            return true;
        }

        *p_defaultIndex = index;
        p_hotkey->action.p_function = DefaultActionMapping[index].action;
        ResetHotkeyAction(&(p_hotkey->action));
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Parser_HotkeySequenceAction
//------------------------------------------------------------------------------
bool Parser_HotkeySequenceAction(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ParserHotkeySequenceInput_t* p_input,
    uint8_t* p_defaultIndex, bool warn)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_input);
    assert(p_defaultIndex);

    if (false == Keymap_IsHotkeySequenceAction(depth, p_keymap))
    {
        return false;
    }

    return ParseSequenceAction(p_value, p_input, p_defaultIndex, warn);
}

//------------------------------------------------------------------------------
// Parser_HotkeySequenceList
//------------------------------------------------------------------------------
bool Parser_HotkeySequenceList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ParserHotkeySequenceInput_t* p_input,
    uint8_t* p_defaultIndex, bool warn)
{
    assert(p_keymap);
    assert(p_value);
    assert(p_input);
    assert(p_defaultIndex);

    if (false == Keymap_IsHotkeySequence(depth, p_keymap))
    {
        return false;
    }

    return ParseSequenceAction(p_value, p_input, p_defaultIndex, warn);
}


/* Global Callback Function Implementations ==================================*/
