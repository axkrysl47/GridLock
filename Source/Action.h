/*==============================================================================
File name   : Action.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the various window actions that GridLock can
              perform.

Copyright (c) 2026
==============================================================================*/
#ifndef ACTION_H
#define ACTION_H

/* Includes ==================================================================*/
#include <stdint.h>

#include <windows.h>

#include "GridLock.h"
#include "Integer.h"
#include "Display.h"
#include "Window.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef struct HotkeyAction_t HotkeyAction_t; // Forward Declaration //

//------------------------------------------------------------------------------
typedef struct
{
    dimension32_t minimum;
    BorderVisibility_t borderVisibility;
    p_IsWindowArranged_t p_IsWindowArranged;
}
WindowConfig_t;

//------------------------------------------------------------------------------
typedef enum
{
    INPUTTYPE_UNKNOWN   = 0,
    INPUTTYPE_NONE      = 1,
    INPUTTYPE_SCALAR    = 2,
    INPUTTYPE_DISPLAY_W = 3,
    INPUTTYPE_DISPLAY_H = 4,
    INPUTTYPE_DISPLAY   = 5,
    INPUTTYPE_WINDOW_W  = 6,
    INPUTTYPE_WINDOW_H  = 7,
    INPUTTYPE_WINDOW    = 8,
}
InputType_t;

typedef struct
{
    InputType_t type;
    union
    {
        struct { int32_t width; int32_t height; } scalar;
        ratio32_t ratio;
        struct { ratio32_t width; ratio32_t height; } dualratio;
    };
}
InputRatio_t;

//------------------------------------------------------------------------------
typedef struct
{
    int8_t toggle;
}
MinimizeParam_t;

typedef struct
{
    int8_t toggle;
}
MaximizeParam_t;

typedef struct
{
    InputRatio_t inlay;

    int8_t columns;
    int8_t rows;

    int8_t index;

    int8_t width;
    int8_t height;

    int8_t transpose;
    int8_t jump;
}
TileParam_t;

typedef struct
{
    ratio32_t horizontal;
    ratio32_t vertical;
    int8_t jump;
}
MoveParam_t;

typedef struct
{
    JumpDisplay_t direction;
}
JumpParam_t;

typedef struct
{
    InputRatio_t resize;
}
IncreaseParam_t;

typedef struct
{
    InputRatio_t resize;
    int8_t edgelock;
    int8_t restore;
}
DecreaseParam_t;

typedef struct
{
    ProjectDisplay_t mode;
}
ProjectParam_t;

typedef struct
{
    uint16_t count;
    const HotkeyAction_t* p_actions;
}
SequenceParam_t;

//------------------------------------------------------------------------------
typedef union
{
    int32_t asUnused;
    MinimizeParam_t asMinimize;
    MaximizeParam_t asMaximize;
    TileParam_t asTile;
    MoveParam_t asMove;
    JumpParam_t asJump;
    IncreaseParam_t asIncrease;
    DecreaseParam_t asDecrease;
    ProjectParam_t asProject;
    SequenceParam_t asSequence;
}
ActionParameter_t;
typedef status_t (*p_Action_t)(HWND, const ActionParameter_t*);

//------------------------------------------------------------------------------
typedef struct HotkeyAction_t // Forward declared above //
{
    p_Action_t p_function;
    ActionParameter_t param;
}
HotkeyAction_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
status_t Action_Init(WindowConfig_t* p_windowConfig);

//------------------------------------------------------------------------------
// p_Action_t Functions
//------------------------------------------------------------------------------
status_t Action_Help(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Revert(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Minimize(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Maximize(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Tile(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Move(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Jump(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Increase(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Decrease(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Project(HWND h_window, const ActionParameter_t* p_param);
status_t Action_Sequence(HWND h_window, const ActionParameter_t* p_param);


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
static inline const char* InputTypeToString(InputType_t inputType)
{
    switch (inputType)
    {
        case (INPUTTYPE_UNKNOWN  ): return "Unknown";
        case (INPUTTYPE_NONE     ): return "None";
        case (INPUTTYPE_SCALAR   ): return "Scalar";
        case (INPUTTYPE_DISPLAY_W): return "DisplayWidth";
        case (INPUTTYPE_DISPLAY_H): return "DisplayHeight";
        case (INPUTTYPE_DISPLAY  ): return "Display";
        case (INPUTTYPE_WINDOW_W ): return "WindowWidth";
        case (INPUTTYPE_WINDOW_H ): return "WindowHeight";
        case (INPUTTYPE_WINDOW   ): return "Window";
        default:                    return "Invalid!";
    }
    // end switch //
}


#endif /* ACTION_H */
