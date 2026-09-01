/*==============================================================================
File name   : Window.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the application window-related actions.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <dwmapi.h>
#include <assert.h>

#include "Window.h"


/* Local Structures/Macros ===================================================*/


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static status_t GetBaseBox(HWND h_window, Box_t* p_base);
static status_t GetPaneBox(HWND h_window, Box_t* p_pane);

//------------------------------------------------------------------------------
static status_t PaneToBase(HWND h_window, const Box_t* restrict p_input,
    Box_t* restrict p_output
);
static status_t BaseToPane(HWND h_window, const Box_t* restrict p_input,
    Box_t* restrict p_output
);


/* Private Member Variables ==================================================*/
//------------------------------------------------------------------------------
static p_IsWindowArranged_t p_IsWindowArranged;
static BorderVisibility_t BorderVisibility;


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static status_t GetBaseBox(HWND h_window, Box_t* p_base)
{
    assert(h_window);
    assert(p_base);

    RECT baseRect;
    SacPrintReturnIfNot(GetWindowRect(h_window, &baseRect), STATUS_FAILURE,
        "Failed to get standard window rect\n"
    );

    SacPrintReturnIfNot(Box_RectToBox(&baseRect, p_base), STATUS_FAILURE,
        "Failed to convert RECT to box\n"
    );
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t GetPaneBox(HWND h_window, Box_t* p_pane)
{
    assert(h_window);
    assert(p_pane);

    RECT paneRect;
    HRESULT result =
        DwmGetWindowAttribute(
            h_window, DWMWA_EXTENDED_FRAME_BOUNDS, &paneRect, sizeof(paneRect)
        );
    SacPrintReturnIf((0 != result), GetBaseBox(h_window, p_pane),
        "Failed to get DWM window attribute, falling back to rect\n"
    );

    SacPrintReturnIfNot(Box_RectToBox(&paneRect, p_pane), STATUS_FAILURE,
        "Failed to convert RECT to box\n"
    );
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t PaneToBase(HWND h_window, const Box_t* restrict p_input,
    Box_t* restrict p_output)
{
    assert(h_window);
    assert(p_input);
    assert(p_output);

    if (BORDER_INVISIBLE == BorderVisibility)
    {
        Box_t base;
        SacReturnIfErr(GetBaseBox(h_window, &base), STATUS_FAILURE);
        Box_t pane;
        SacReturnIfErr(GetPaneBox(h_window, &pane), STATUS_FAILURE);

        Box_t framePadding;
        Box_Subtract(&framePadding, &base, &pane);
        Box_Add(p_output, p_input, &framePadding);
        Box_SymmetricIncrement(p_output);
        return STATUS_SUCCESS;
    }

    if (BORDER_VISIBLE == BorderVisibility)
    {
        Box_t base;
        SacReturnIfErr(GetBaseBox(h_window, &base), STATUS_FAILURE);
        Box_t pane;
        SacReturnIfErr(GetPaneBox(h_window, &pane), STATUS_FAILURE);

        Box_t framePadding;
        Box_Subtract(&framePadding, &base, &pane);
        Box_Add(p_output, p_input, &framePadding);
        return STATUS_SUCCESS;
    }

    // if (BORDER_FULL == BorderVisibility) //
    *p_output = Box_Copy(p_input);
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t BaseToPane(HWND h_window, const Box_t* restrict p_input,
    Box_t* restrict p_output)
{
    assert(h_window);
    assert(p_input);
    assert(p_output);

    if (BORDER_INVISIBLE == BorderVisibility)
    {
        Box_t base;
        SacReturnIfErr(GetBaseBox(h_window, &base), STATUS_FAILURE);
        Box_t pane;
        SacReturnIfErr(GetPaneBox(h_window, &pane), STATUS_FAILURE);

        Box_t framePadding;
        Box_Subtract(&framePadding, &pane, &base);
        Box_Add(p_output, p_input, &framePadding);
        Box_SymmetricDecrement(p_output);
        return STATUS_SUCCESS;
    }

    if (BORDER_VISIBLE == BorderVisibility)
    {
        Box_t base;
        SacReturnIfErr(GetBaseBox(h_window, &base), STATUS_FAILURE);
        Box_t pane;
        SacReturnIfErr(GetPaneBox(h_window, &pane), STATUS_FAILURE);

        Box_t framePadding;
        Box_Subtract(&framePadding, &pane, &base);
        Box_Add(p_output, p_input, &framePadding);
        return STATUS_SUCCESS;
    }

    // if (BORDER_FULL == BorderVisibility) //
    *p_output = Box_Copy(p_input);
    return STATUS_SUCCESS;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Window_InitBorderVisibility
//------------------------------------------------------------------------------
status_t Window_InitBorderVisibility(BorderVisibility_t visibility)
{
    assert(BORDER_INVISIBLE == visibility
        || BORDER_VISIBLE   == visibility
        || BORDER_FULL      == visibility
    );

    BorderVisibility = visibility;
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_InitIsWindowArranged
//------------------------------------------------------------------------------
status_t Window_InitIsWindowArranged(p_IsWindowArranged_t p_isWindowArranged)
{
    assert(p_isWindowArranged);

    p_IsWindowArranged = p_isWindowArranged;
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_GetBox
//------------------------------------------------------------------------------
status_t Window_GetBox(HWND h_window, Box_t* p_box)
{
    assert(h_window);
    assert(p_box);

    if (BORDER_INVISIBLE == BorderVisibility)
    {
        SacPrintReturnIfErr(GetPaneBox(h_window, p_box), STATUS_FAILURE,
            "Could not get pane box\n"
        );
        Box_SymmetricDecrement(p_box);
        return STATUS_SUCCESS;
    }

    if (BORDER_VISIBLE == BorderVisibility)
    {
        SacPrintReturnIfErr(GetPaneBox(h_window, p_box), STATUS_FAILURE,
            "Could not get pane box\n"
        );
        return STATUS_SUCCESS;
    }

    // if (BORDER_FULL == BorderVisibility) //
    RECT rect;
    SacPrintReturnIfNot(GetWindowRect(h_window, &rect), STATUS_FAILURE,
        "Failed to get standard window rect\n"
    );

    SacPrintReturnIfNot(Box_RectToBox(&rect, p_box), STATUS_FAILURE,
        "Failed to convert RECT to box\n"
    );
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_IsMinimized
//------------------------------------------------------------------------------
bool Window_IsMinimized(HWND h_window)
{
    assert(h_window);

    LONG_PTR style = GetWindowLongPtrA(h_window, GWL_STYLE);
    return 0 != (WS_MINIMIZE & style);
}

//------------------------------------------------------------------------------
// Window_IsMaximized
//------------------------------------------------------------------------------
bool Window_IsMaximized(HWND h_window)
{
    assert(h_window);

    LONG_PTR style = GetWindowLongPtrA(h_window, GWL_STYLE);
    return 0 != (WS_MAXIMIZE & style);
}

//------------------------------------------------------------------------------
// Window_Minimize
//------------------------------------------------------------------------------
status_t Window_Minimize(HWND h_window)
{
    assert(h_window);

    if (false == Window_IsMinimized(h_window))
    {
        ShowWindow(h_window, SW_MINIMIZE);
    }
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_Maximize
//------------------------------------------------------------------------------
status_t Window_Maximize(HWND h_window)
{
    assert(h_window);

    if (false == Window_IsMaximized(h_window))
    {
        ShowWindow(h_window, SW_MAXIMIZE);
    }
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_Restore
//------------------------------------------------------------------------------
status_t Window_Restore(HWND h_window)
{
    assert(h_window);

    if (Window_IsMinimized(h_window) || Window_IsMaximized(h_window))
    {
        ShowWindow(h_window, SW_RESTORE);
        return STATUS_SUCCESS;
    }

    SacReturnIfNull(p_IsWindowArranged, STATUS_SUCCESS);
    BOOL isArranged = p_IsWindowArranged(h_window);
    if (isArranged)
    {
        ShowWindow(h_window, SW_RESTORE);
    }

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_SetBox
//------------------------------------------------------------------------------
status_t Window_SetBox(HWND h_window, const Box_t* p_box)
{
    assert(h_window);
    assert(p_box);

    Box_t base;
    SacReturnIfErr(
        PaneToBase(h_window, p_box, &base), STATUS_FAILURE
    );

    SacPrintReturnIfNot(
        SetWindowPos(h_window,
            HWND_TOP,
            base.left, base.top, base.width, base.height,
            SWP_NOZORDER | SWP_SHOWWINDOW
        ),
        STATUS_FAILURE,
        "Failed to set window position!\n"
    );
    printf("Window = [ (%d , %d); %d x %d pixels ]\n",
        p_box->left, p_box->top, p_box->width, p_box->height
    );

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_MoveBox
//------------------------------------------------------------------------------
status_t Window_MoveBox(HWND h_window, const Box_t* p_box)
{
    assert(h_window);
    assert(p_box);

    Box_t base;
    SacReturnIfErr(
        PaneToBase(h_window, p_box, &base), STATUS_FAILURE
    );

    SacPrintReturnIfNot(
        SetWindowPos(h_window,
            HWND_TOP,
            base.left, base.top, base.width, base.height,
            SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW
        ),
        STATUS_FAILURE,
        "Failed to move window position!\n"
    );
    printf("Window = [ (%d , %d) pixels ]\n", p_box->left, p_box->top);
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_ResizeBox
//------------------------------------------------------------------------------
status_t Window_ResizeBox(HWND h_window, const Box_t* p_box)
{
    assert(h_window);
    assert(p_box);

    Box_t base;
    SacReturnIfErr(
        PaneToBase(h_window, p_box, &base), STATUS_FAILURE
    );

    SacPrintReturnIfNot(
        SetWindowPos(h_window,
            HWND_TOP,
            base.left, base.top, base.width, base.height,
            SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW
        ),
        STATUS_FAILURE,
        "Failed to resize window!\n"
    );
    printf("Window = [ %d x %d pixels ]\n", p_box->width, p_box->height);
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Window_GetMinimumSize
//------------------------------------------------------------------------------
status_t Window_GetMinimumSize(HWND h_window, dimension32_t* p_dimension)
{
    assert(h_window);
    assert(p_dimension);

    MINMAXINFO minMaxInfo = {0};
    minMaxInfo.ptMinTrackSize.x = GetSystemMetrics(SM_CXMINTRACK);
    minMaxInfo.ptMinTrackSize.y = GetSystemMetrics(SM_CYMINTRACK);
    LRESULT result =
        SendMessageA(h_window, WM_GETMINMAXINFO, 0, (LPARAM)&minMaxInfo);
    SacPrintReturnIf((0 != result), STATUS_FAILURE,
        "Could not get minmaxinfo\n"
    );

    Box_t base =
    {
        .left   = 0,
        .top    = 0,
        .width  = minMaxInfo.ptMinTrackSize.x,
        .height = minMaxInfo.ptMinTrackSize.y,
    };
    printd("Minimum size = [ %d x %d pixels ]\n", base.width, base.height);

    Box_t output;
    SacReturnIfErr(
        BaseToPane(h_window, &base, &output), STATUS_FAILURE
    );
    p_dimension->width = output.width;
    p_dimension->height  = output.height;
    return STATUS_SUCCESS;
}


/* Global Callback Function Implementations ==================================*/
