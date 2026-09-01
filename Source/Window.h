/*==============================================================================
File name   : Window.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the application window-related actions.

Copyright (c) 2026
==============================================================================*/
#ifndef WINDOW_H
#define WINDOW_H

/* Includes ==================================================================*/
#include <windows.h>

#include "GridLock.h"
#include "Integer.h"
#include "Box.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef BOOL(WINAPI* p_IsWindowArranged_t)(HWND);

//------------------------------------------------------------------------------
typedef enum
{
    BORDER_INVISIBLE = 0,
    BORDER_VISIBLE   = 1,
    BORDER_FULL      = 2,
}
BorderVisibility_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
status_t Window_InitBorderVisibility(BorderVisibility_t visibility);
status_t Window_InitIsWindowArranged(p_IsWindowArranged_t p_isWindowArranged);

//------------------------------------------------------------------------------
status_t Window_GetBox(HWND h_window, Box_t* p_box);
bool Window_IsMinimized(HWND h_window);
bool Window_IsMaximized(HWND h_window);

//------------------------------------------------------------------------------
status_t Window_Minimize(HWND h_window);
status_t Window_Maximize(HWND h_window);
status_t Window_Restore(HWND h_window);
status_t Window_SetBox(HWND h_window, const Box_t* p_box);
status_t Window_MoveBox(HWND h_window, const Box_t* p_box);
status_t Window_ResizeBox(HWND h_window, const Box_t* p_box);
status_t Window_GetMinimumSize(HWND h_window, dimension32_t* p_dimension);


/* Public Function Implementations ===========================================*/


#endif /* WINDOW_H */
