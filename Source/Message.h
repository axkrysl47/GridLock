/*==============================================================================
File name   : Message.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-08-30
Version     : 1.0.0
Description : This module defines the various Message Boxes which GridLock uses
              as a user interface.

Copyright (c) 2026
==============================================================================*/
#ifndef MESSAGE_H
#define MESSAGE_H

/* Includes ==================================================================*/
#include <stdbool.h>

#include "GridLock.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef enum
{
    SOUND_FREERUNNING = 0,
    SOUND_UNLATCHED   = 1,
    SOUND_LATCHED     = 2,
}
SoundLatch_t;

typedef struct
{
    unsigned int icon;
    unsigned int error;
    unsigned int warning;
    unsigned int info;
}
MessageIconIds_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
void Message_Init(const MessageIconIds_t* p_icons, SoundLatch_t latch);
void Message_SetSoundLatch(SoundLatch_t latch);
SoundLatch_t Message_GetSoundLatch(void);

//------------------------------------------------------------------------------
status_t Message_InitPrintErrorExit(const char* prefix, int error,
    status_t status
);
status_t Message_InitConfigExit(const char* configurationFilename,
    const char* string, status_t status
);
status_t Message_InitConfigPrintErrorExit(const char* configurationFilename,
    const char* prefix, int error
);

//------------------------------------------------------------------------------
bool Message_Welcome(const char* configurationFilename);

//------------------------------------------------------------------------------
void Message_WarningHotkeyAlreadyRegistered(const char* hotkey);
void Message_ErrorHotkeyRegistration(const char* hotkey, int error);

//------------------------------------------------------------------------------
void Message_NotepadFailed(const char* configurationFilename);
void Message_OpenFailed(const char* configurationFilename);

//------------------------------------------------------------------------------
bool Message_RequestResetToDefaults(void);
bool Message_ConfirmResetToDefaults(const char* configurationFilename,
    const char* backupFilename
);
void Message_WarningBackupFailed(const char* configurationFilename);
bool Message_NotifyResetToDefaults(void);

//------------------------------------------------------------------------------
status_t Message_Help(const char* version, const char* gridlockFilepath,
    const char* configurationFilename
);
status_t Message_About(const char* version, const char* gridlockFilepath,
    const char* configurationFilename
);

//------------------------------------------------------------------------------
void Message_Warning(const char* format, ...);
void Message_Error(const char* format, ...);


/* Public Function Implementations ===========================================*/


#endif /* MESSAGE_H */
