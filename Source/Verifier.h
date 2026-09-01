/*==============================================================================
File name   : Verifier.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines functions that verify a given GridLock hotkey
              configuration and sets unset parameters as defined from the
              defaults file.

Copyright (c) 2026
==============================================================================*/
#ifndef VERIFIER_H
#define VERIFIER_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>

#include "GridLock.h"
#include "Config.h"


/* Local Structures/Macros ===================================================*/


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
status_t Verifier_MinimizeParam(MinimizeParam_t* p_param,
    const MinimizeParam_t* p_default
);
status_t Verifier_MaximizeParam(MaximizeParam_t* p_param,
    const MaximizeParam_t* p_default
);
status_t Verifier_TileParam(TileParam_t* p_param,
    const TileParam_t* p_default, Hotkey_t* p_hotkeyToWarn
);
status_t Verifier_MoveParam(MoveParam_t* p_param,
    const MoveParam_t* p_default, Hotkey_t* p_hotkeyToWarn
);
status_t Verifier_JumpParam(JumpParam_t* p_param,
    const JumpParam_t* p_default, Hotkey_t* p_hotkeyToWarn
);
status_t Verifier_IncreaseParam(IncreaseParam_t* p_param,
    const IncreaseParam_t* p_default, Hotkey_t* p_hotkeyToWarn
);
status_t Verifier_DecreaseParam(DecreaseParam_t* p_param,
    const DecreaseParam_t* p_default, Hotkey_t* p_hotkeyToWarn
);
status_t Verifier_ProjectParam(ProjectParam_t* p_param,
    const ProjectParam_t* p_default, Hotkey_t* p_hotkeyToWarn
);
status_t Verifier_SequenceParam(SequenceParam_t* p_param,
    Hotkey_t* p_hotkeyToWarn
);

//------------------------------------------------------------------------------
status_t Verifier_HotkeyAction(HotkeyAction_t* p_action, uint8_t defaultIndex,
    Hotkey_t* p_hotkeyToWarn
);


/* Public Function Implementations ===========================================*/


#endif /* VERIFIER_H */
