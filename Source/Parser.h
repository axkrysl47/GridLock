/*==============================================================================
File name   : Parser.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines functions that parse a given section of a YAML
              GridLock configuration file.

Copyright (c) 2026
==============================================================================*/
#ifndef PARSER_H
#define PARSER_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>

#include "GridLock.h"
#include "Keymap.h"
#include "Config.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef struct
{
    HotkeyAction_t* p_sequence;
    Hotkey_t* p_hotkey;
    HotkeyAction_t* p_sequenceStart;
}
ParserHotkeySequenceInput_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
bool Parser_InitialPassValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ConfigInitialPass_t* p_config
);
bool Parser_EnableDisableValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, bool* p_allBelow, bool* p_this
);
bool Parser_HotkeyValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, Hotkey_t* p_hotkey
);
bool Parser_HotkeyValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, Hotkey_t* p_hotkey
);
bool Parser_SequenceValue(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_sequence
);
bool Parser_SequenceValueList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, HotkeyAction_t* p_sequence
);

//------------------------------------------------------------------------------
bool Parser_HotkeyAction(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, Hotkey_t* p_hotkey, uint8_t* p_defaultIndex, bool warn
);
bool Parser_HotkeySequenceAction(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ParserHotkeySequenceInput_t* p_input,
    uint8_t* p_defaultIndex, bool warn
);
bool Parser_HotkeySequenceList(int8_t depth, Keymap_t* p_keymap,
    const char* p_value, ParserHotkeySequenceInput_t* p_input,
    uint8_t* p_defaultIndex, bool warn
);


/* Public Function Implementations ===========================================*/


#endif /* PARSER_H */
