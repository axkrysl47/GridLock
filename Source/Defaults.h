/*==============================================================================
File name   : Defaults.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the mapping defaults for a GridLock hotkey
              configuration.

Copyright (c) 2026
==============================================================================*/
#ifndef DEFAULTS_H
#define DEFAULTS_H

/* Includes ==================================================================*/
#include <stdint.h>

#include "Action.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define INVALID_DEFAULT_INDEX (UINT8_MAX)

//------------------------------------------------------------------------------
typedef struct
{
    const char* name;
    p_Action_t action;
    ActionParameter_t defaultparam;
}
DefaultActionMapping_t;


/* Public Global Variables ===================================================*/
//------------------------------------------------------------------------------
extern const DefaultActionMapping_t DefaultActionMapping[];


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
bool Defaults_GetActionIndexByName(const char* p_name, uint8_t* p_index);
const char* Defaults_GetActionNameByFunction(p_Action_t action);


/* Public Function Implementations ===========================================*/


#endif /* DEFAULTS_H */
