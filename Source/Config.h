/*==============================================================================
File name   : Config.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the high-level configuration interface for
              GridLock.

Copyright (c) 2026
==============================================================================*/
#ifndef CONFIG_H
#define CONFIG_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdio.h>

#include "GridLock.h"
#include "Integer.h"
#include "Utility.h"
#include "Action.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define INVALID_ACTION ((void*)(-1)) // technically not standards-compliant //

//------------------------------------------------------------------------------
typedef struct
{
    unsigned int modifiers;
    unsigned int key;
}
HotkeyKeybind_t;

typedef struct
{
    version_t version;
    dimension32_t minimum;
    BorderVisibility_t borderVisibility;
    uint16_t n_hotkeys;
    uint16_t n_sequences;
}
ConfigInitialPass_t;

//------------------------------------------------------------------------------
typedef struct
{
    HotkeyKeybind_t keybind;
    HotkeyAction_t action;
}
Hotkey_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
status_t Config_InitialPass(FILE* file, ConfigInitialPass_t* p_config);
status_t Config_ParseHotkeys(FILE* file,
    uint16_t n_hotkeys, Hotkey_t* p_hotkeys,
    uint16_t n_sequences, HotkeyAction_t* p_sequences
);

//------------------------------------------------------------------------------
const char* Config_StringPrintHotkey(const Hotkey_t* p_hotkey);
void Config_UserWarning(const Hotkey_t* p_hotkey, const char* string);



/* Public Function Implementations ===========================================*/


#endif /* CONFIG_H */
