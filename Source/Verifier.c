/*==============================================================================
File name   : Verifier.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines functions that verify a given GridLock hotkey
              configuration and sets unset parameters as defined from the
              defaults file.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "Verifier.h"

#include "Defaults.h"
#include "Config.h"


/* Local Structures/Macros ===================================================*/


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline bool IsInt8Unset(int8_t value);
static inline bool IsInt32Unset(int32_t value);
static inline bool IsRatio32Unset(ratio32_t ratio);

static inline void DefaultIfInt8Unset(int8_t* p_value, int8_t defaultValue);
static inline void DefaultIfInt32Unset(int32_t* p_value, int32_t defaultValue);
static inline void DefaultIfRatio32Unset(ratio32_t* p_ratio,
    ratio32_t defaultRatio
);

//------------------------------------------------------------------------------

static inline status_t VerifierResizeRatioParam(InputRatio_t* p_ratio,
    const InputRatio_t* p_default, Hotkey_t* p_hotkeyToWarn
);

//------------------------------------------------------------------------------
static inline status_t ExitUserWarning(Hotkey_t* p_hotkeyToWarn,
    const char* string
);



/* Private Member Variables ==================================================*/


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline bool IsInt8Unset(int8_t value)
{
    return (VALUE_INT8_UNSET == value);
}

//------------------------------------------------------------------------------
static inline bool IsInt32Unset(int32_t value)
{
    return (VALUE_INT32_UNSET == value);
}

//------------------------------------------------------------------------------
static inline bool IsRatio32Unset(ratio32_t ratio)
{
    return (VALUE_INT32_UNSET == ratio.numerator
        || VALUE_INT32_UNSET == ratio.denominator
    );
}

//------------------------------------------------------------------------------
static inline void DefaultIfInt8Unset(int8_t* p_value, int8_t defaultValue)
{
    assert(p_value);

    if (IsInt8Unset(*p_value))
    {
        *p_value = defaultValue;
    }
}

//------------------------------------------------------------------------------
static inline void DefaultIfInt32Unset(int32_t* p_value, int32_t defaultValue)
{
    assert(p_value);

    if (IsInt32Unset(*p_value))
    {
        *p_value = defaultValue;
    }
}

//------------------------------------------------------------------------------
static inline void DefaultIfRatio32Unset(ratio32_t* p_ratio,
    ratio32_t defaultRatio)
{
    assert(p_ratio);

    if (IsRatio32Unset(*p_ratio))
    {
        p_ratio->numerator = defaultRatio.numerator;
        p_ratio->denominator = defaultRatio.denominator;
    }
}

//------------------------------------------------------------------------------
static inline status_t VerifierResizeRatioParam(InputRatio_t* p_ratio,
    const InputRatio_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_ratio);
    assert(p_default);

    if (INPUTTYPE_UNKNOWN == p_ratio->type)
    {
        p_ratio->type = p_default->type;
    }

    switch (p_ratio->type)
    {

    default:
    return ExitUserWarning(p_hotkeyToWarn, "Unrecognized resize type.");

    case (INPUTTYPE_UNKNOWN):
    case (INPUTTYPE_NONE):
    break;

    case (INPUTTYPE_SCALAR):
    {
        p_ratio->dualratio.height.numerator = 0;
        p_ratio->dualratio.height.denominator = 0;

        if (p_default->type != p_ratio->type)
        {
            if (IsInt32Unset(p_ratio->scalar.width)
                && IsInt32Unset(p_ratio->scalar.height))
            {
                return ExitUserWarning(p_hotkeyToWarn,
                    "Resize type \"scalar\" was selected,"
                    " but no width was provided."
                );
            }

            DefaultIfInt32Unset(&(p_ratio->scalar.width), 0);
            DefaultIfInt32Unset(&(p_ratio->scalar.height), 0);
        }
        else
        {
            DefaultIfInt32Unset(
                &(p_ratio->scalar.width), p_default->scalar.width
            );
            DefaultIfInt32Unset(
                &(p_ratio->scalar.height), p_default->scalar.height
            );
        }

        p_ratio->scalar.width &= ~1;
        p_ratio->scalar.height &= ~1;
        if (p_ratio->scalar.width <= 0 || p_ratio->scalar.height <= 0)
        {
            return ExitUserWarning(
                p_hotkeyToWarn,
                "Scalar resize dimensions must be positive even integers."
            );
        }
    }
    break;

    case (INPUTTYPE_DISPLAY_W):
    case (INPUTTYPE_DISPLAY_H):
    case (INPUTTYPE_WINDOW_W):
    case (INPUTTYPE_WINDOW_H):
    {
        p_ratio->dualratio.height.numerator = 0;
        p_ratio->dualratio.height.denominator = 0;

        if (p_default->type != p_ratio->type)
        {
            if (IsRatio32Unset(p_ratio->ratio))
            {
                return ExitUserWarning(p_hotkeyToWarn,
                    "Resize type Display/Window Width/height was selected,"
                    " but no ratio was provided."
                );
            }
        }
        else
        {
            DefaultIfRatio32Unset(&(p_ratio->ratio), p_default->ratio);
        }

        if (p_ratio->ratio.numerator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize ratio numerator was 0 or negative."
            );
        }
        if (p_ratio->ratio.denominator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize ratio denominator was 0 or negative."
            );
        }
    }
    break;

    case (INPUTTYPE_DISPLAY):
    case (INPUTTYPE_WINDOW):
    {
        if (p_default->type != p_ratio->type)
        {
            if (IsRatio32Unset(p_ratio->dualratio.width)
                && IsRatio32Unset(p_ratio->dualratio.height))
            {
                return ExitUserWarning(p_hotkeyToWarn,
                    "Resize type Display/Window was selected,"
                    " but neither width nor height ratios were provided."
                );
            }

            const ratio32_t zero = { .numerator = 0, .denominator = 1 };
            DefaultIfRatio32Unset(&(p_ratio->dualratio.height), zero);
            DefaultIfRatio32Unset(&(p_ratio->dualratio.width), zero);
        }
        else
        {
            DefaultIfRatio32Unset(
                &(p_ratio->dualratio.width), p_default->dualratio.width
            );
            DefaultIfRatio32Unset(
                &(p_ratio->dualratio.height), p_default->dualratio.height
            );
        }

        if (0 == p_ratio->dualratio.width.numerator
            && 0 == p_ratio->dualratio.height.numerator)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize width and height were both 0."
            );
        }

        if (p_ratio->dualratio.width.numerator < 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize width numerator was negative."
            );
        }
        if (p_ratio->dualratio.width.denominator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize width denominator was 0 or negative."
            );
        }

        if (p_ratio->dualratio.height.numerator < 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize height numerator was negative."
            );
        }
        if (p_ratio->dualratio.height.denominator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Resize height denominator was 0 or negative."
            );
        }
    }
    break;

    }
    // end switch //

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static inline status_t ExitUserWarning(Hotkey_t* p_hotkeyToWarn,
    const char* string)
{
    assert(string);

    if (NULL != p_hotkeyToWarn)
    {
        Config_UserWarning(p_hotkeyToWarn, string);
    }
    return STATUS_INVALIDPARAM;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Verifier_MinimizeParam
//------------------------------------------------------------------------------
status_t Verifier_MinimizeParam(MinimizeParam_t* p_param,
    const MinimizeParam_t* p_default)
{
    assert(p_param);
    assert(p_default);

    DefaultIfInt8Unset(&(p_param->toggle), p_default->toggle);
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_MaximizeParam
//------------------------------------------------------------------------------
status_t Verifier_MaximizeParam(MaximizeParam_t* p_param,
    const MaximizeParam_t* p_default)
{
    assert(p_param);
    assert(p_default);

    DefaultIfInt8Unset(&(p_param->toggle), p_default->toggle);
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_TileParam
//------------------------------------------------------------------------------
status_t Verifier_TileParam(TileParam_t* p_param,
    const TileParam_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);
    assert(p_default);

    InputRatio_t* p_inlay = &(p_param->inlay);
    const InputRatio_t* p_inlayDefault = &(p_default->inlay);

    if (INPUTTYPE_UNKNOWN == p_inlay->type)
    {
        p_inlay->type = p_inlayDefault->type;
    }

    switch (p_inlay->type)
    {

    default:
    return ExitUserWarning(p_hotkeyToWarn, "Unrecognized tile inlay type.");

    case (INPUTTYPE_UNKNOWN):
    case (INPUTTYPE_NONE):
    break;

    case (INPUTTYPE_SCALAR):
    {
        p_inlay->dualratio.height.numerator = 0;
        p_inlay->dualratio.height.denominator = 0;

        if (p_inlayDefault->type != p_inlay->type)
        {
            if (IsInt32Unset(p_inlay->scalar.width)
                && IsInt32Unset(p_inlay->scalar.height))
            {
                return ExitUserWarning(p_hotkeyToWarn,
                    "Inlay type \"scalar\" was selected,"
                    " but no width was provided."
                );
            }

            DefaultIfInt32Unset(&(p_inlay->scalar.width), 0);
            DefaultIfInt32Unset(&(p_inlay->scalar.height), 0);
        }
        else
        {
            DefaultIfInt32Unset(
                &(p_inlay->scalar.width), p_inlayDefault->scalar.width
            );
            DefaultIfInt32Unset(
                &(p_inlay->scalar.height), p_inlayDefault->scalar.height
            );
        }

        p_inlay->scalar.width &= ~1;
        p_inlay->scalar.height &= ~1;
        if (p_inlay->scalar.width <= 0 || p_inlay->scalar.height <= 0)
        {
            return ExitUserWarning(
                p_hotkeyToWarn,
                "Inlay scalar resize dimensions must be positive even integers."
            );
        }
    }
    break;

    case (INPUTTYPE_DISPLAY_W):
    case (INPUTTYPE_DISPLAY_H):
    {
        p_inlay->dualratio.height.numerator = 0;
        p_inlay->dualratio.height.denominator = 0;

        if (p_inlayDefault->type != p_inlay->type)
        {
            if (IsRatio32Unset(p_inlay->ratio))
            {
                return ExitUserWarning(p_hotkeyToWarn,
                    "Inlay type Display/Window Width/height was selected,"
                    " but no ratio was provided."
                );
            }
        }
        else
        {
            DefaultIfRatio32Unset(&(p_inlay->ratio), p_inlayDefault->ratio);
        }

        if (p_inlay->ratio.numerator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay ratio numerator was 0 or negative."
            );
        }
        if (p_inlay->ratio.denominator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay ratio denominator was 0 or negative."
            );
        }
    }
    break;

    case (INPUTTYPE_DISPLAY):
    {
        if (p_inlayDefault->type != p_inlay->type)
        {
            if (IsRatio32Unset(p_inlay->dualratio.width)
                && IsRatio32Unset(p_inlay->dualratio.height))
            {
                return ExitUserWarning(p_hotkeyToWarn,
                    "Inlay type Display/Window was selected,"
                    " but neither width nor height ratios were provided."
                );
            }

            const ratio32_t zero = { .numerator = 0, .denominator = 1 };
            DefaultIfRatio32Unset(&(p_inlay->dualratio.height), zero);
            DefaultIfRatio32Unset(&(p_inlay->dualratio.width), zero);
        }
        else
        {
            DefaultIfRatio32Unset(
                &(p_inlay->dualratio.width), p_inlayDefault->dualratio.width
            );
            DefaultIfRatio32Unset(
                &(p_inlay->dualratio.height), p_inlayDefault->dualratio.height
            );
        }

        if (0 == p_inlay->dualratio.width.numerator
            && 0 == p_inlay->dualratio.height.numerator)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay width and height were both 0."
            );
        }

        if (p_inlay->dualratio.width.numerator < 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay width numerator was negative."
            );
        }
        if (p_inlay->dualratio.width.denominator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay width denominator was 0 or negative."
            );
        }

        if (p_inlay->dualratio.height.numerator < 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay height numerator was negative."
            );
        }
        if (p_inlay->dualratio.height.denominator <= 0)
        {
            return ExitUserWarning(p_hotkeyToWarn,
                "Inlay height denominator was 0 or negative."
            );
        }
    }
    break;

    case (INPUTTYPE_WINDOW_W):
    case (INPUTTYPE_WINDOW_H):
    case (INPUTTYPE_WINDOW):
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Inlay sub-type \"Window\" was selected, but it is not supported"
        );
    }
    break;

    }
    // end switch //

    DefaultIfInt8Unset(&(p_param->columns), p_default->columns);
    if (IsInt8Unset(p_param->columns))
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action was selected, but no grid column was provided."
        );
    }
    if (p_param->columns <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action grid columns is invalid."
        );
    }

    DefaultIfInt8Unset(&(p_param->rows), p_default->rows);
    if (IsInt8Unset(p_param->rows))
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action was selected, but no grid row was provided."
        );
    }
    if (p_param->rows <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action grid rows is invalid."
        );
    }

    DefaultIfInt8Unset(&(p_param->index), p_default->index);
    if (IsInt8Unset(p_param->index))
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action was selected, but no grid index was provided."
        );
    }
    if (p_param->index <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action grid index is invalid."
        );
    }
    if (((int16_t)(p_param->columns) * (int16_t)(p_param->rows))
        < p_param->index)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action grid index exceeds grid bounds."
        );
    }

    DefaultIfInt8Unset(&(p_param->width), p_default->width);
    if (IsInt8Unset(p_param->width))
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action was selected, but no grid width was provided."
        );
    }
    if (p_param->width <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action grid width is invalid."
        );
    }

    DefaultIfInt8Unset(&(p_param->height), p_default->height);
    if (IsInt8Unset(p_param->height))
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action was selected, but no grid height was provided."
        );
    }
    if (p_param->height <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Tile action grid height is invalid."
        );
    }

    DefaultIfInt8Unset(&(p_param->transpose), p_default->transpose);
    DefaultIfInt8Unset(&(p_param->jump), p_default->jump);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_MoveParam
//------------------------------------------------------------------------------
status_t Verifier_MoveParam(MoveParam_t* p_param,
    const MoveParam_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);
    assert(p_default);

    DefaultIfRatio32Unset(&(p_param->horizontal), p_default->horizontal);
    DefaultIfRatio32Unset(&(p_param->vertical), p_default->vertical);

    if (p_param->horizontal.numerator < 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Horizontal move numerator was negative."
        );
    }
    if (p_param->horizontal.denominator <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Horizontal move denominator was 0 or negative."
        );
    }
    if (p_param->vertical.numerator < 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Vertical move numerator was negative."
        );
    }
    if (p_param->vertical.denominator <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Vertical move denominator was 0 or negative."
        );
    }

    DefaultIfInt8Unset(&(p_param->jump), p_default->jump);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_JumpParam
//------------------------------------------------------------------------------
status_t Verifier_JumpParam(JumpParam_t* p_param,
    const JumpParam_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);
    assert(p_default);

    if (JUMP_DISPLAY_UNKNOWN == p_param->direction)
    {
        p_param->direction = p_default->direction;
    }

    if (p_param->direction < JUMP_DISPLAY_NONE
        || JUMP_DISPLAY_DOWN < p_param->direction)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Jump display direction was invalid."
        );
    }

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_IncreaseParam
//------------------------------------------------------------------------------
status_t Verifier_IncreaseParam(IncreaseParam_t* p_param,
    const IncreaseParam_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);
    assert(p_default);

    InputRatio_t* p_ratio = &(p_param->resize);
    const InputRatio_t* p_ratioDefault = &(p_default->resize);
    return VerifierResizeRatioParam(p_ratio, p_ratioDefault, p_hotkeyToWarn);
}

//------------------------------------------------------------------------------
// Verifier_DecreaseParam
//------------------------------------------------------------------------------
status_t Verifier_DecreaseParam(DecreaseParam_t* p_param,
    const DecreaseParam_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);
    assert(p_default);

    DefaultIfInt8Unset(&(p_param->edgelock), p_default->edgelock);
    DefaultIfInt8Unset(&(p_param->restore), p_default->restore);

    InputRatio_t* p_ratio = &(p_param->resize);
    const InputRatio_t* p_ratioDefault = &(p_default->resize);
    return VerifierResizeRatioParam(p_ratio, p_ratioDefault, p_hotkeyToWarn);
}

//------------------------------------------------------------------------------
// Verifier_ProjectParam
//------------------------------------------------------------------------------
status_t Verifier_ProjectParam(ProjectParam_t* p_param,
    const ProjectParam_t* p_default, Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);
    assert(p_default);

    if (PROJECT_DISPLAY_UNKNOWN == p_param->mode)
    {
        p_param->mode = p_default->mode;
    }

    if (PROJECT_DISPLAY_NONE         != p_param->mode
        && PROJECT_DISPLAY_PRIMARY   != p_param->mode
        && PROJECT_DISPLAY_DUPLICATE != p_param->mode
        && PROJECT_DISPLAY_EXTEND    != p_param->mode
        && PROJECT_DISPLAY_SECONDARY != p_param->mode)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Project display mode was invalid."
        );
    }

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_SequenceParam
//------------------------------------------------------------------------------
status_t Verifier_SequenceParam(SequenceParam_t* p_param,
    Hotkey_t* p_hotkeyToWarn)
{
    assert(p_param);

    // Individual sub-sequence actions are verified during the parse pass //

    if (p_param->count <= 0)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Sequence contains empty list of actions."
        );
    }

    if (NULL == p_param->p_actions)
    {
        return ExitUserWarning(p_hotkeyToWarn,
            "Sequence parsing produced an invalid internal state."
        );
    }

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Verifier_HotkeyAction
//------------------------------------------------------------------------------
status_t Verifier_HotkeyAction(HotkeyAction_t* p_action, uint8_t defaultIndex,
    Hotkey_t* p_hotkeyToWarn)
{
    assert(p_action);

    if (NULL == p_action->p_function)
    {
        if (NULL != p_hotkeyToWarn)
        {
            Config_UserWarning(p_hotkeyToWarn, "Action was not specified");
        }
        return STATUS_FAILURE;
    }

    if (INVALID_ACTION == p_action->p_function)
    {
        if (NULL != p_hotkeyToWarn)
        {
            Config_UserWarning(p_hotkeyToWarn, "Action was invalid!");
        }
        return STATUS_FAILURE;
    }

    if (INVALID_DEFAULT_INDEX == defaultIndex)
    {
        if (NULL != p_hotkeyToWarn)
        {
            Config_UserWarning(p_hotkeyToWarn, "Action was not recognized!");
        }
        return STATUS_FAILURE;
    }

    if (Action_Help == p_action->p_function
        || Action_Revert == p_action->p_function)
    {
        return STATUS_SUCCESS; // Nothing to verify //
    }

    if (Action_Minimize == p_action->p_function)
    {
        return Verifier_MinimizeParam(
            &(p_action->param.asMinimize),
            &(DefaultActionMapping[defaultIndex].defaultparam.asMinimize)
        );
    }

    if (Action_Maximize == p_action->p_function)
    {
        return Verifier_MaximizeParam(
            &(p_action->param.asMaximize),
            &(DefaultActionMapping[defaultIndex].defaultparam.asMaximize)
        );
    }

    if (Action_Tile == p_action->p_function)
    {
        return Verifier_TileParam(
            &(p_action->param.asTile),
            &(DefaultActionMapping[defaultIndex].defaultparam.asTile),
            p_hotkeyToWarn
        );
    }

    if (Action_Move == p_action->p_function)
    {
        return Verifier_MoveParam(
            &(p_action->param.asMove),
            &(DefaultActionMapping[defaultIndex].defaultparam.asMove),
            p_hotkeyToWarn
        );
    }

    if (Action_Jump == p_action->p_function)
    {
        return Verifier_JumpParam(
            &(p_action->param.asJump),
            &(DefaultActionMapping[defaultIndex].defaultparam.asJump),
            p_hotkeyToWarn
        );
    }

    if (Action_Increase == p_action->p_function)
    {
        return Verifier_IncreaseParam(
            &(p_action->param.asIncrease),
            &(DefaultActionMapping[defaultIndex].defaultparam.asIncrease),
            p_hotkeyToWarn
        );
    }

    if (Action_Decrease == p_action->p_function)
    {
        return Verifier_DecreaseParam(
            &(p_action->param.asDecrease),
            &(DefaultActionMapping[defaultIndex].defaultparam.asDecrease),
            p_hotkeyToWarn
        );
    }

    if (Action_Project == p_action->p_function)
    {
        return Verifier_ProjectParam(
            &(p_action->param.asProject),
            &(DefaultActionMapping[defaultIndex].defaultparam.asProject),
            p_hotkeyToWarn
        );
    }

    if (Action_Sequence == p_action->p_function)
    {
        return Verifier_SequenceParam(
            &(p_action->param.asSequence), p_hotkeyToWarn
        );
    }

    return ExitUserWarning(p_hotkeyToWarn, "Unverified action encountered.");
}


/* Global Callback Function Implementations ==================================*/
