/*==============================================================================
File name   : Keymap.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines functions that parse a given attribute of a
              YAML GridLock configuration file.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include <windows.h>

#include "Keymap.h"

#include "Defaults.h"
#include "Utility.h"
#include "Mapping.h"


/* Local Structures/Macros ===================================================*/


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline bool IsKeymap(int8_t depth, const char* restrict key[],
    Keymap_t* restrict p_keymap
);

//------------------------------------------------------------------------------
static inline bool KeymapToggleParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, int8_t* p_param, bool isSequence
);
static inline bool KeymapResizeParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_param
);
static inline bool KeymapResizeRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence
);
static inline bool KeymapResizeScalarList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence
);


/* Private Member Variables ==================================================*/


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline bool IsKeymap(int8_t depth, const char* restrict key[],
    Keymap_t* restrict p_keymap)
{
    assert(0 <= depth);
    assert(key);
    assert(p_keymap);

    const size_t capacity = KEYMAP_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < depth; i++)
    {
        if (false == Utility_StringEquals(key[i], p_keymap->keys[i], capacity))
        {
            return false;
        }
    }

    return true;
}

//------------------------------------------------------------------------------
static inline bool KeymapToggleParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, int8_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t parametersKeymapDepth =
        (isSequence) ? SequenceParametersKeymapDepth : ParametersKeymapDepth;
    if ((parametersKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** parametersKeymap =
        (isSequence) ? SequenceParametersKeymap : ParametersKeymap;
    if (false == IsKeymap(parametersKeymapDepth, parametersKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("toggle", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseBool(p_value, p_param), false);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
static inline bool KeymapResizeParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_ratio);

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("scalar", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseInt32(p_value, &(p_ratio->scalar.width)), false
        );
        p_ratio->type = INPUTTYPE_SCALAR;
        p_ratio->scalar.height = p_ratio->scalar.width;
        return true;
    }
    if (Utility_StringEquals("type", p_keymap->keys[depth - 1], capacity))
    {
        const InputTypeMapping_t* p_mapping = Mapping_InputType(p_value);
        SacPrintReturnIfNull(p_mapping, false,
            "Mapping_InputType could not parse input type"
        );
        p_ratio->type = p_mapping->type;
        return true;
    }
    if (Utility_StringEquals("ratio", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseRatioString(p_value, &(p_ratio->ratio)), false
        );
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
static inline bool KeymapResizeRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_ratio);

    const int16_t ratioKeymapDepth =
        (isSequence) ? SequenceRatioKeymapDepth : RatioKeymapDepth;
    if ((ratioKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** ratioKeymap =
        (isSequence) ? SequenceRatioKeymap : RatioKeymap;
    if (false == IsKeymap(ratioKeymapDepth, ratioKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("width", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseRatioString(p_value, &(p_ratio->dualratio.width)),
            false
        );
        return true;
    }
    if (Utility_StringEquals("height", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseRatioString(p_value, &(p_ratio->dualratio.height)),
            false
        );
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
static inline bool KeymapResizeScalarList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_ratio);

    const int16_t scalarKeymapDepth =
        (isSequence) ? SequenceScalarKeymapDepth : ScalarKeymapDepth;
    if (scalarKeymapDepth != depth)
    {
        return false;
    }

    const char** scalarKeymap =
        (isSequence) ? SequenceScalarKeymap : ScalarKeymap;
    if (false == IsKeymap(scalarKeymapDepth, scalarKeymap, p_keymap))
    {
        return false;
    }

    if (VALUE_INT32_UNSET == p_ratio->scalar.width)
    {
        SacReturnIfNot(
            Utility_ParseInt32(p_value, &(p_ratio->scalar.width)), false
        );
        p_ratio->type = INPUTTYPE_SCALAR;
        return true;
    }
    if (VALUE_INT32_UNSET == p_ratio->scalar.height)
    {
        SacReturnIfNot(
            Utility_ParseInt32(p_value, &(p_ratio->scalar.height)), false
        );
        p_ratio->type = INPUTTYPE_SCALAR;
        return true;
    }

    return false;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Keymap_SetKey
//------------------------------------------------------------------------------
void Keymap_SetKey(int8_t depth, Keymap_t* p_keymap, const char* p_key)
{
    assert(1 <= depth && depth <= KEYMAP_MAX_DEPTH);
    assert(p_keymap);
    assert(p_key);

    Utility_StringCopy(p_keymap->keys[depth - 1], p_key, KEYMAP_MAX_WIDTH);
}

//------------------------------------------------------------------------------
// Keymap_IsHotkeyAction
//------------------------------------------------------------------------------
bool Keymap_IsHotkeyAction(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return (ActionKeymapDepth == depth
        && IsKeymap(ActionKeymapDepth, ActionKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_IsHotkeyParameter
//------------------------------------------------------------------------------
bool Keymap_IsHotkeyParameter(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return (ParametersKeymapDepth == depth
        && IsKeymap(ParametersKeymapDepth, ParametersKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_IsHotkeySequence
//------------------------------------------------------------------------------
bool Keymap_IsHotkeySequence(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return (SequenceKeymapDepth == depth
        && IsKeymap(SequenceKeymapDepth, SequenceKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_IsHotkeySequenceAction
//------------------------------------------------------------------------------
bool Keymap_IsHotkeySequenceAction(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return (SequenceActionKeymapDepth == depth
        && IsKeymap(SequenceActionKeymapDepth, SequenceActionKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_IsEndOfHotkeySequenceAction
//------------------------------------------------------------------------------
bool Keymap_IsEndOfHotkeySequenceAction(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return ((SequenceKeymapDepth + 1) == depth
        && IsKeymap(SequenceKeymapDepth, SequenceKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_IsEndOfHotkey
//------------------------------------------------------------------------------
bool Keymap_IsEndOfHotkey(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return ((HotkeyKeymapDepth + 1) == depth
        && IsKeymap(HotkeyKeymapDepth, HotkeyKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_IsEndOfEnable
//------------------------------------------------------------------------------
bool Keymap_IsEndOfEnable(int8_t depth, Keymap_t* p_keymap)
{
    assert(0 <= depth);
    assert(p_keymap);

    return (EnableKeymapDepth == depth
        && IsKeymap(EnableKeymapDepth, EnableKeymap, p_keymap)
    );
}

//------------------------------------------------------------------------------
// Keymap_VersionValue
//------------------------------------------------------------------------------
bool Keymap_VersionValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, version_t* p_version)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_version);

    if (VersionKeymapDepth != depth)
    {
        return false;
    }

    if (false == IsKeymap(VersionKeymapDepth, VersionKeymap, p_keymap))
    {
        return false;
    }

    SacReturnIfNot(Utility_ParseVersionString(p_value, p_version), false);
    return true;
}

//------------------------------------------------------------------------------
// Keymap_BorderVisibilityValue
//------------------------------------------------------------------------------
bool Keymap_BorderVisibilityValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, BorderVisibility_t* p_borderVisibility)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_borderVisibility);

    if (BorderKeymapDepth != depth)
    {
        return false;
    }

    if (false == IsKeymap(BorderKeymapDepth, BorderKeymap, p_keymap))
    {
        return false;
    }

    const BorderVisibilityMapping_t* p_mapping =
        Mapping_BorderVisibility(p_value);
    SacReturnIfNull(p_mapping, false);

    *p_borderVisibility = p_mapping->borderVisibility;
    return true;
}

//------------------------------------------------------------------------------
// Keymap_MinimumValue
//------------------------------------------------------------------------------
bool Keymap_MinimumValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, dimension32_t* p_minimum)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_minimum);

    if ((MinimumKeymapDepth + 1) != depth)
    {
        return false;
    }

    if (false == IsKeymap(MinimumKeymapDepth, MinimumKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("width", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseInt32(p_value, &(p_minimum->width)), false);
        SacReturnIf((p_minimum->width < 0), false);
        return true;
    }
    if (Utility_StringEquals("height", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseInt32(p_value, &(p_minimum->height)), false
        );
        SacReturnIf((p_minimum->height < 0), false);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_EnableDisableValue
//------------------------------------------------------------------------------
bool Keymap_EnableDisableValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, bool* p_allBelow, bool* p_this)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_allBelow);
    assert(p_this);
    assert(EnableKeymapDepth == DisableKeymapDepth);

    if (EnableKeymapDepth != depth)
    {
        return false;
    }

    const bool isEnable = IsKeymap(EnableKeymapDepth, EnableKeymap, p_keymap);
    if (false == isEnable
        && false == IsKeymap(DisableKeymapDepth, DisableKeymap, p_keymap))
    {
        return false;
    }

    if (Utility_StringEquals("below", p_value, sizeof("below"))
        || Utility_StringEquals("AllBelow", p_value, sizeof("AllBelow"))
        || Utility_StringEquals("All Below", p_value, sizeof("All Below"))
        || Utility_StringEquals("all_below", p_value, sizeof("all_below")))
    {
        *p_allBelow = isEnable;
        *p_this = isEnable;
        return true;
    }

    if (Utility_StringEquals("this", p_value, sizeof("this")))
    {
        *p_this = isEnable;
        return true;
    }

    int8_t boolean = true;
    SacReturnIfNot(Utility_ParseBool(p_value, &boolean), false);

    *p_this = (boolean) ? isEnable : !isEnable;
    return true;
}

//------------------------------------------------------------------------------
// Keymap_KeybindValue
//------------------------------------------------------------------------------
bool Keymap_KeybindValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, unsigned int* p_keybind)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_keybind);

    if (KeybindKeymapDepth != depth)
    {
        return false;
    }

    if (false == IsKeymap(KeybindKeymapDepth, KeybindKeymap, p_keymap))
    {
        return false;
    }

    const KeybindMapping_t* p_mapping = Mapping_Keybind(p_value);
    if (NULL != p_mapping)
    {
        *p_keybind = p_mapping->keycode;
        return true;
    }

    SacPrintReturnIf(('\0' == p_value[0] || '\0' != p_value[1]), false,
        "Invalid keybind (must be a named key or single character)!\n"
    );

    SHORT result = VkKeyScanA(*p_value);
    SacReturnIf((-1 == result), false);

    *p_keybind = (unsigned int)(UINT8_MAX & result);
    return true;
}

//------------------------------------------------------------------------------
// Keymap_ModifierValueList
//------------------------------------------------------------------------------
bool Keymap_ModifierValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, unsigned int* p_modifier)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_modifier);

    if (ModifiersKeymapDepth != depth)
    {
        return false;
    }

    if (false == IsKeymap(ModifiersKeymapDepth, ModifiersKeymap, p_keymap))
    {
        return false;
    }

    if (Utility_StringEquals("Ctrl", p_value, sizeof("Ctrl")))
    {
        *p_modifier |= MOD_CONTROL;
        return true;
    }
    if (Utility_StringEquals("Win", p_value, sizeof("Win")))
    {
        *p_modifier |= MOD_WIN;
        return true;
    }
    if (Utility_StringEquals("Alt", p_value, sizeof("Alt")))
    {
        *p_modifier |= MOD_ALT;
        return true;
    }
    if (Utility_StringEquals("Shift", p_value, sizeof("Shift")))
    {
        *p_modifier |= MOD_SHIFT;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_MinimizeValue
//------------------------------------------------------------------------------
bool Keymap_MinimizeValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, MinimizeParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    return KeymapToggleParameter(depth, p_keymap, p_value,
        &(p_param->toggle), isSequence
    );
}

//------------------------------------------------------------------------------
// Keymap_MaximizeValue
//------------------------------------------------------------------------------
bool Keymap_MaximizeValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, MaximizeParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    return KeymapToggleParameter(depth, p_keymap, p_value,
        &(p_param->toggle), isSequence
    );
}

//------------------------------------------------------------------------------
// Keymap_TileParameter
//------------------------------------------------------------------------------
bool Keymap_TileParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, TileParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t parametersKeymapDepth =
        (isSequence) ? SequenceParametersKeymapDepth : ParametersKeymapDepth;
    if ((parametersKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** parametersKeymap =
        (isSequence) ? SequenceParametersKeymap : ParametersKeymap;
    if (false == IsKeymap(parametersKeymapDepth, parametersKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("columns", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->columns)), false);
        return true;
    }
    if (Utility_StringEquals("rows", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->rows)), false);
        return true;
    }
    if (Utility_StringEquals("index", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->index)), false);
        return true;
    }
    if (Utility_StringEquals("width", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->width)), false);
        return true;
    }
    if (Utility_StringEquals("height", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->height)), false);
        return true;
    }
    if (Utility_StringEquals("transpose", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseBool(p_value, &(p_param->transpose)), false
        );
        return true;
    }
    if (Utility_StringEquals("jump", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseBool(p_value, &(p_param->jump)), false
        );
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_TileInlay
//------------------------------------------------------------------------------
bool Keymap_TileInlay(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_inlay, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_inlay);

    const int16_t inlayKeymapDepth =
        (isSequence) ? SequenceInlayKeymapDepth : InlayKeymapDepth;
    if ((inlayKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** inlayKeymap =
        (isSequence) ? SequenceInlayKeymap : InlayKeymap;
    if (false == IsKeymap(inlayKeymapDepth, inlayKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("type", p_keymap->keys[depth - 1], capacity))
    {
        const InputTypeMapping_t* p_mapping = Mapping_InputType(p_value);
        SacPrintReturnIfNull(p_mapping, false,
            "Mapping_InputType could not parse input type"
        );
        p_inlay->type = p_mapping->type;
        return true;
    }
    if (Utility_StringEquals("ratio", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseRatioString(p_value, &(p_inlay->ratio)), false
        );
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_TileInlayRatio
//------------------------------------------------------------------------------
bool Keymap_TileInlayRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_inlay, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_inlay);

    const int16_t inlayRatioKeymapDepth =
        (isSequence) ? SequenceInlayRatioKeymapDepth : InlayRatioKeymapDepth;
    if ((inlayRatioKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** inlayRatioKeymap =
        (isSequence) ? SequenceInlayRatioKeymap : InlayRatioKeymap;
    if (false == IsKeymap(inlayRatioKeymapDepth, inlayRatioKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("width", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseRatioString(p_value, &(p_inlay->dualratio.width)),
            false
        );
        return true;
    }
    if (Utility_StringEquals("height", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseRatioString(p_value, &(p_inlay->dualratio.height)),
            false
        );
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_TileGridList
//------------------------------------------------------------------------------
bool Keymap_TileGridList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, TileParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t gridKeymapDepth =
        (isSequence) ? SequenceGridKeymapDepth : GridKeymapDepth;
    if (gridKeymapDepth != depth)
    {
        return false;
    }

    const char** gridKeymap =
        (isSequence) ? SequenceGridKeymap : GridKeymap;
    if (false == IsKeymap(gridKeymapDepth, gridKeymap, p_keymap))
    {
        return false;
    }

    if (VALUE_INT8_UNSET == p_param->columns)
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->columns)), false);
        return true;
    }
    if (VALUE_INT8_UNSET == p_param->rows)
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->rows)), false);
        return true;
    }
    if (VALUE_INT8_UNSET == p_param->index)
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->index)), false);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_TileSizeList
//------------------------------------------------------------------------------
bool Keymap_TileSizeList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, TileParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t sizeKeymapDepth =
        (isSequence) ? SequenceSizeKeymapDepth : SizeKeymapDepth;
    if (sizeKeymapDepth != depth)
    {
        return false;
    }

    const char** sizeKeymap =
        (isSequence) ? SequenceSizeKeymap : SizeKeymap;
    if (false == IsKeymap(sizeKeymapDepth, sizeKeymap, p_keymap))
    {
        return false;
    }

    if (VALUE_INT8_UNSET == p_param->width)
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->width)), false);
        return true;
    }
    if (VALUE_INT8_UNSET == p_param->height)
    {
        SacReturnIfNot(Utility_ParseInt8(p_value, &(p_param->height)), false);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_MoveValue
//------------------------------------------------------------------------------
bool Keymap_MoveValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, MoveParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t ratioKeymapDepth =
        (isSequence) ? SequenceRatioKeymapDepth : RatioKeymapDepth;
    if ((ratioKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** ratioKeymap =
        (isSequence) ? SequenceRatioKeymap : RatioKeymap;
    if (false == IsKeymap(ratioKeymapDepth, ratioKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("horizontal", p_keymap->keys[depth - 1], capacity))
    {
        if (Utility_ParseRatioString(p_value, &(p_param->horizontal)))
        {
            SacReturnIf((p_param->horizontal.numerator < 0), false);
            return true;
        }

        SacReturnIfNot(
            Utility_ParseInt32(p_value, &(p_param->horizontal.numerator)), false
        );
        SacReturnIf((1 < p_param->horizontal.numerator), false);
        p_param->horizontal.denominator = 1;
        return true;
    }
    if (Utility_StringEquals("vertical", p_keymap->keys[depth - 1], capacity))
    {
        if (Utility_ParseRatioString(p_value, &(p_param->vertical)))
        {
            SacReturnIf((p_param->vertical.numerator < 0), false);
            return true;
        }

        SacReturnIfNot(
            Utility_ParseInt32(p_value, &(p_param->vertical.numerator)), false
        );
        SacReturnIf((1 < p_param->vertical.numerator), false);
        p_param->vertical.denominator = 1;
        return true;
    }
    if (Utility_StringEquals("jump", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(
            Utility_ParseBool(p_value, &(p_param->jump)), false
        );
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_JumpValue
//------------------------------------------------------------------------------
bool Keymap_JumpValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, JumpParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t parametersKeymapDepth =
        (isSequence) ? SequenceParametersKeymapDepth : ParametersKeymapDepth;
    if ((parametersKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** parametersKeymap =
        (isSequence) ? SequenceParametersKeymap : ParametersKeymap;
    if (false == IsKeymap(parametersKeymapDepth, parametersKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("direction", p_keymap->keys[depth - 1], capacity))
    {
        const JumpDisplayMapping_t* p_mapping =
            Mapping_JumpDisplay(p_value);
        SacReturnIfNull(p_mapping, false);
        p_param->direction = p_mapping->direction;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_IncreaseParameter
//------------------------------------------------------------------------------
bool Keymap_IncreaseParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, IncreaseParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t parametersKeymapDepth =
        (isSequence) ? SequenceParametersKeymapDepth : ParametersKeymapDepth;
    if ((parametersKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** parametersKeymap =
        (isSequence) ? SequenceParametersKeymap : ParametersKeymap;
    if (false == IsKeymap(parametersKeymapDepth, parametersKeymap, p_keymap))
    {
        return false;
    }

    return KeymapResizeParameter(depth, p_keymap, p_value, &(p_param->resize));
}

//------------------------------------------------------------------------------
// Keymap_DecreaseParameter
//------------------------------------------------------------------------------
bool Keymap_DecreaseParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, DecreaseParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t parametersKeymapDepth =
        (isSequence) ? SequenceParametersKeymapDepth : ParametersKeymapDepth;
    if ((parametersKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** parametersKeymap =
        (isSequence) ? SequenceParametersKeymap : ParametersKeymap;
    if (false == IsKeymap(parametersKeymapDepth, parametersKeymap, p_keymap))
    {
        return false;
    }

    if (KeymapResizeParameter(depth, p_keymap, p_value, &(p_param->resize)))
    {
        return true;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("edgelock", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseBool(p_value, &(p_param->edgelock)), false);
        return true;
    }
    if (Utility_StringEquals("restore", p_keymap->keys[depth - 1], capacity))
    {
        SacReturnIfNot(Utility_ParseBool(p_value, &(p_param->restore)), false);
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Keymap_IncreaseRatio
//------------------------------------------------------------------------------
bool Keymap_IncreaseRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence)
{
    return KeymapResizeRatio(depth, p_keymap, p_value, p_ratio, isSequence);
}

//------------------------------------------------------------------------------
// Keymap_DecreaseRatio
//------------------------------------------------------------------------------
bool Keymap_DecreaseRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence)
{
    return KeymapResizeRatio(depth, p_keymap, p_value, p_ratio, isSequence);
}

//------------------------------------------------------------------------------
// Keymap_IncreaseScalarList
//------------------------------------------------------------------------------
bool Keymap_IncreaseScalarList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence)
{
    return
        KeymapResizeScalarList(depth, p_keymap, p_value, p_ratio, isSequence);
}

//------------------------------------------------------------------------------
// Keymap_DecreaseScalarList
//------------------------------------------------------------------------------
bool Keymap_DecreaseScalarList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence)
{
    return
        KeymapResizeScalarList(depth, p_keymap, p_value, p_ratio, isSequence);
}

//------------------------------------------------------------------------------
bool Keymap_ProjectValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ProjectParam_t* p_param, bool isSequence)
{
    assert(0 <= depth);
    assert(p_keymap);
    assert(p_value);
    assert(p_param);

    const int16_t parametersKeymapDepth =
        (isSequence) ? SequenceParametersKeymapDepth : ParametersKeymapDepth;
    if ((parametersKeymapDepth + 1) != depth)
    {
        return false;
    }

    const char** parametersKeymap =
        (isSequence) ? SequenceParametersKeymap : ParametersKeymap;
    if (false == IsKeymap(parametersKeymapDepth, parametersKeymap, p_keymap))
    {
        return false;
    }

    const size_t capacity = KEYMAP_MAX_WIDTH;
    if (Utility_StringEquals("project", p_keymap->keys[depth - 1], capacity)
        || Utility_StringEquals("mode", p_keymap->keys[depth - 1], capacity))
    {
        const ProjectDisplayMapping_t* p_mapping =
            Mapping_ProjectDisplay(p_value);
        SacReturnIfNull(p_mapping, false);
        p_param->mode = p_mapping->mode;
        return true;
    }

    return false;
}

/* Global Callback Function Implementations ==================================*/
