/*==============================================================================
File name   : Display.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the display related window actions.

Copyright (c) 2026
==============================================================================*/
#ifndef DISPLAY_H
#define DISPLAY_H

/* Includes ==================================================================*/
#include <windows.h>

#include "GridLock.h"
#include "Box.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef enum
{
    JUMP_DISPLAY_UNKNOWN = 0,
    JUMP_DISPLAY_NONE    = 1,
    JUMP_DISPLAY_NEXT    = 2, // Flatten the list //
    JUMP_DISPLAY_PREV    = 3, // Flatten the list //
    JUMP_DISPLAY_LEFT    = 4,
    JUMP_DISPLAY_RIGHT   = 5,
    JUMP_DISPLAY_UP      = 6,
    JUMP_DISPLAY_DOWN    = 7,
}
JumpDisplay_t;

typedef enum
{
    PROJECT_DISPLAY_UNKNOWN   = 0xFF, // 0xFF due to bitmask enum //
    PROJECT_DISPLAY_NONE      = 0,
    PROJECT_DISPLAY_PRIMARY   = SDC_TOPOLOGY_INTERNAL,
    PROJECT_DISPLAY_DUPLICATE = SDC_TOPOLOGY_CLONE,
    PROJECT_DISPLAY_EXTEND    = SDC_TOPOLOGY_EXTEND,
    PROJECT_DISPLAY_SECONDARY = SDC_TOPOLOGY_EXTERNAL,
}
ProjectDisplay_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
status_t Display_GetBox(HWND h_window, Box_t* p_box);
status_t Display_GetJumpBox(Box_t* p_last, JumpDisplay_t direction,
    Box_t* p_next
);
status_t Display_Project(ProjectDisplay_t mode);

//------------------------------------------------------------------------------
status_t Display_QueryAllDisplays(void);


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
static inline const char* JumpDirectionToString(JumpDisplay_t direction)
{
    switch (direction)
    {
        case (JUMP_DISPLAY_UNKNOWN): return "Unknown";
        case (JUMP_DISPLAY_NONE   ): return "None";
        case (JUMP_DISPLAY_NEXT   ): return "Next";
        case (JUMP_DISPLAY_PREV   ): return "Previous";
        case (JUMP_DISPLAY_LEFT   ): return "Left";
        case (JUMP_DISPLAY_RIGHT  ): return "Right";
        case (JUMP_DISPLAY_UP     ): return "Up";
        case (JUMP_DISPLAY_DOWN   ): return "Down";
        default:                      return "Invalid!";
    }
    // end switch //
}

//------------------------------------------------------------------------------
static inline const char* ProjectModeToString(ProjectDisplay_t mode)
{
    switch (mode)
    {
        case (PROJECT_DISPLAY_UNKNOWN  ): return "Unknown";
        case (PROJECT_DISPLAY_NONE     ): return "None";
        case (PROJECT_DISPLAY_PRIMARY  ): return "Primary";
        case (PROJECT_DISPLAY_DUPLICATE): return "Duplicate";
        case (PROJECT_DISPLAY_EXTEND   ): return "Extend";
        case (PROJECT_DISPLAY_SECONDARY): return "Secondary";
        default:                          return "Invalid!";
    }
    // end switch //
}

#endif /* DISPLAY_H */
