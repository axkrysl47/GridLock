/*==============================================================================
File name   : Keymap.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines functions that parse a given attribute of a
              YAML GridLock configuration file.

Copyright (c) 2026
==============================================================================*/
#ifndef KEYMAP_H
#define KEYMAP_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>

#include "Config.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define KEYMAP_MAX_DEPTH (10)
#define KEYMAP_MAX_WIDTH (32)

typedef struct
{
    char keys[KEYMAP_MAX_DEPTH][KEYMAP_MAX_WIDTH];
}
Keymap_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
void Keymap_SetKey(int8_t depth, Keymap_t* p_keymap, const char* p_key);

//------------------------------------------------------------------------------
bool Keymap_IsHotkeyAction(int8_t depth, Keymap_t* p_keymap);
bool Keymap_IsHotkeyParameter(int8_t depth, Keymap_t* p_keymap);
bool Keymap_IsHotkeySequence(int8_t depth, Keymap_t* p_keymap);
bool Keymap_IsHotkeySequenceAction(int8_t depth, Keymap_t* p_keymap);
bool Keymap_IsEndOfHotkeySequenceAction(int8_t depth, Keymap_t* p_keymap);
bool Keymap_IsEndOfHotkey(int8_t depth, Keymap_t* p_keymap);
bool Keymap_IsEndOfEnable(int8_t depth, Keymap_t* p_keymap);

//------------------------------------------------------------------------------
bool Keymap_VersionValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, version_t* p_version
);
bool Keymap_BorderVisibilityValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, BorderVisibility_t* p_borderVisibility
);
bool Keymap_MinimumValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, dimension32_t* p_minimum
);

//------------------------------------------------------------------------------
bool Keymap_EnableDisableValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, bool* p_allBelow, bool* p_this
);

//------------------------------------------------------------------------------
bool Keymap_KeybindValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, unsigned int* p_keybind
);
bool Keymap_ModifierValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, unsigned int* p_modifier
);

//------------------------------------------------------------------------------
bool Keymap_MinimizeValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, MinimizeParam_t* p_param, bool isSequence
);

//------------------------------------------------------------------------------
bool Keymap_MaximizeValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, MaximizeParam_t* p_param, bool isSequence
);

//------------------------------------------------------------------------------
bool Keymap_TileParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, TileParam_t* p_param, bool isSequence
);
bool Keymap_TileInlay(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_inlay, bool isSequence
);
bool Keymap_TileInlayRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_inlay, bool isSequence
);

bool Keymap_TileGridList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, TileParam_t* p_param, bool isSequence
);
bool Keymap_TileSizeList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, TileParam_t* p_param, bool isSequence
);

//------------------------------------------------------------------------------
bool Keymap_MoveValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, MoveParam_t* p_param, bool isSequence
);

//------------------------------------------------------------------------------
bool Keymap_JumpValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, JumpParam_t* p_param, bool isSequence
);

//------------------------------------------------------------------------------
bool Keymap_IncreaseParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, IncreaseParam_t* p_param, bool isSequence
);
bool Keymap_DecreaseParameter(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, DecreaseParam_t* p_param, bool isSequence
);
bool Keymap_IncreaseRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence
);
bool Keymap_DecreaseRatio(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence
);
bool Keymap_IncreaseScalarList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence
);
bool Keymap_DecreaseScalarList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, InputRatio_t* p_ratio, bool isSequence
);

//------------------------------------------------------------------------------
bool Keymap_ProjectValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ProjectParam_t* p_param, bool isSequence
);


/* Public Function Implementations ===========================================*/


#endif /* KEYMAP_H */
