/*==============================================================================
File name   : Display.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the display related window actions.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#include <windows.h>

#include "Display.h"

#include "Integer.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define MAXIMUM_NUMBER_OF_DISPLAYS (16)
static_assert(MAXIMUM_NUMBER_OF_DISPLAYS <= UINT8_MAX);

//------------------------------------------------------------------------------
typedef struct
{
    bool primary;
    Box_t display;
}
DisplayInfo_t;

typedef struct
{
    uint8_t capacity;
    uint8_t length;
    DisplayInfo_t* p_info;
}
EnumDisplayMonitorsParam_t;


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
BOOL CALLBACK EnumDisplayMonitorsCallback(HMONITOR h_monitor,
    HDC h_deviceContext, LPRECT lp_rectMonitor, LPARAM lp_param
);


/* Private Member Variables ==================================================*/


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
BOOL CALLBACK EnumDisplayMonitorsCallback(HMONITOR h_monitor,
    HDC h_deviceContext, LPRECT lp_rectMonitor, LPARAM lp_param)
{
    assert(lp_param);

    (void)h_deviceContext;
    (void)lp_rectMonitor;

    EnumDisplayMonitorsParam_t* p_param = (EnumDisplayMonitorsParam_t*)lp_param;
    if (p_param->capacity <= p_param->length)
    {
        // We'll reject the rest, but let's allow it to run its course //
        return TRUE;
    }

    MONITORINFO monitorInfo;
    monitorInfo.cbSize = sizeof(MONITORINFO);
    SacPrintReturnIfNot(
        GetMonitorInfoA(h_monitor, &monitorInfo),
        TRUE,
        "Failed to get monitor info\n"
    );

    DisplayInfo_t info =
    {
        .primary = (MONITORINFOF_PRIMARY & monitorInfo.dwFlags),
        .display = { 0, 0, 0, 0 }
    };
    SacPrintReturnIfNot(Box_RectToBox(&monitorInfo.rcWork, &(info.display)),
        TRUE,
        "Failed to convert to box\n"
    );

    // Sort in display boxes //
    uint8_t index = p_param->length;
    for (uint8_t i = 0; i < p_param->length; i++)
    {
        DisplayInfo_t* p_other = &(p_param->p_info[i]);
        if (info.display.left < p_other->display.left)
        {
            index = i;
            break;
        }
        if (info.display.left == p_other->display.left
            && info.display.top < p_other->display.top)
        {
            index = i;
            break;
        }
    }

    // Shift over any to the right //
    const uint8_t countToShift = p_param->length - index;
    if (0 < countToShift)
    {
        memmove(
            &(p_param->p_info[index + 1]),
            &(p_param->p_info[index]),
            sizeof(DisplayInfo_t) * countToShift
        );
    }

    p_param->p_info[index] = info;
    (p_param->length)++;

    // Continue //
    return TRUE;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Display_GetBox
//------------------------------------------------------------------------------
status_t Display_GetBox(HWND h_window, Box_t* p_box)
{
    assert(h_window);
    assert(p_box);

    HMONITOR h_monitor = MonitorFromWindow(h_window, MONITOR_DEFAULTTONEAREST);

    SacPrintReturnIfNull(h_monitor, STATUS_INVALIDPARAM,
        "Failed to get monitor from window\n"
    );

    MONITORINFO monitorInfo = {0};
    monitorInfo.cbSize = sizeof(MONITORINFO);
    SacPrintReturnIfNot(
        GetMonitorInfoA(h_monitor, &monitorInfo),
        STATUS_FAILURE,
        "Failed to get monitor info\n"
    );

    SacPrintReturnIfNot(
        Box_RectToBox(&monitorInfo.rcWork, p_box), // work area //
        STATUS_FAILURE,
        "Failed to convert RECT to box\n"
    );
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Display_GetJumpBox
//------------------------------------------------------------------------------
status_t Display_GetJumpBox(Box_t* p_last, JumpDisplay_t direction,
    Box_t* p_next)
{
    assert(p_last);
    assert(JUMP_DISPLAY_NONE <= direction && direction <= JUMP_DISPLAY_DOWN);
    assert(p_next);

    if (JUMP_DISPLAY_NONE == direction)
    {
        *p_next = Box_Copy(p_last);
        return STATUS_SUCCESS;
    }

    DisplayInfo_t displayInfos[MAXIMUM_NUMBER_OF_DISPLAYS];
    EnumDisplayMonitorsParam_t enumDisplayMonitorsParam =
    {
        .capacity = sizeof(displayInfos) / sizeof(DisplayInfo_t),
        .length   = 0,
        .p_info   = displayInfos,
    };
    EnumDisplayMonitors(
        NULL, // encompasses all the displays on the desktop //
        NULL, // coordinates are virtual-screen coordinates //
        EnumDisplayMonitorsCallback, // MonitorEnumProc //
        (LPARAM)&enumDisplayMonitorsParam
    );

    if (JUMP_DISPLAY_NEXT == direction || JUMP_DISPLAY_PREV == direction)
    {
        int i;
        for (i = 0; i < enumDisplayMonitorsParam.length; i++)
        {
            if (Box_IsEqual(p_last, &(displayInfos[i].display)))
            {
                break;
            }
        }
        SacPrintReturnIf((enumDisplayMonitorsParam.length <= i), STATUS_FAILURE,
            "Could not find active display in display infos"
        );

        if (JUMP_DISPLAY_NEXT == direction)
        {
            i++;
            if (enumDisplayMonitorsParam.length <= i)
            {
                i -= enumDisplayMonitorsParam.length;
            }
        }
        else // if (JUMP_DISPLAY_PREV == direction) //
        {
            i--;
            if (i < 0)
            {
                i += enumDisplayMonitorsParam.length;
            }
        }
        *p_next = Box_Copy(&(displayInfos[i].display));
        return STATUS_SUCCESS;
    }

    Box_t target = Box_Copy(p_last);
    if (JUMP_DISPLAY_LEFT == direction)
    {
        target.left = Int32_Subtract(target.left, target.width);
    }
    else if (JUMP_DISPLAY_RIGHT == direction)
    {
        target.left = Int32_Add(target.left, target.width);
    }
    else if (JUMP_DISPLAY_UP == direction)
    {
        target.top = Int32_Subtract(target.top, target.height);
    }
    else if (JUMP_DISPLAY_DOWN == direction)
    {
        target.top = Int32_Add(target.top, target.height);
    }
    else
    {
        return STATUS_FAILURE;
    }

#ifndef NDEBUG
    Box_Print("Probe box", &target);
#endif

    RECT rectangle = Box_RectFromBox(&target);

    HMONITOR h_monitor =
        MonitorFromRect(&rectangle, MONITOR_DEFAULTTONEAREST);
    SacPrintReturnIfNull(h_monitor, STATUS_FAILURE,
        "Failed to get monitor from rect\n"
    );

    MONITORINFO monitorInfo = {0};
    monitorInfo.cbSize = sizeof(MONITORINFO);
    SacPrintReturnIfNot(
        GetMonitorInfoA(h_monitor, &monitorInfo),
        STATUS_FAILURE,
        "Failed to get monitor info\n"
    );

    SacPrintReturnIfNot(
        Box_RectToBox(&monitorInfo.rcWork, p_next),
        STATUS_FAILURE,
        "Failed to convert RECT to box\n"
    );
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Display_Project
//------------------------------------------------------------------------------
status_t Display_Project(ProjectDisplay_t mode)
{
    assert(PROJECT_DISPLAY_NONE      == mode
        || PROJECT_DISPLAY_PRIMARY   == mode
        || PROJECT_DISPLAY_DUPLICATE == mode
        || PROJECT_DISPLAY_EXTEND    == mode
        || PROJECT_DISPLAY_SECONDARY == mode
    );

    if (PROJECT_DISPLAY_NONE == mode)
    {
        return STATUS_SUCCESS;
    }

    SacPrintReturnIfErr(SetDisplayConfig(0, NULL, 0, NULL, mode | SDC_APPLY),
        STATUS_FAILURE,
        "Failed to project display mode\n"
    );

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Display_QueryAllDisplays
//------------------------------------------------------------------------------
status_t Display_QueryAllDisplays(void)
{
    DisplayInfo_t displayInfos[MAXIMUM_NUMBER_OF_DISPLAYS];
    EnumDisplayMonitorsParam_t enumDisplayMonitorsParam =
    {
        .capacity = sizeof(displayInfos) / sizeof(DisplayInfo_t),
        .length   = 0,
        .p_info   = displayInfos,
    };

    printf("Querying all active displays...\n\n");
    EnumDisplayMonitors(
        NULL, // encompasses all the displays on the desktop //
        NULL, // coordinates are virtual-screen coordinates //
        EnumDisplayMonitorsCallback, // MonitorEnumProc //
        (LPARAM)&enumDisplayMonitorsParam
    );

    for (uint8_t i = 0; i < enumDisplayMonitorsParam.length; i++)
    {
        printf("Display %d = [ %s: (%d , %d); %d x %d pixels ]\n",
            i,
            (enumDisplayMonitorsParam.p_info[i].primary)
                ? "Primary" : "Secondary",
            enumDisplayMonitorsParam.p_info[i].display.left,
            enumDisplayMonitorsParam.p_info[i].display.top,
            enumDisplayMonitorsParam.p_info[i].display.width,
            enumDisplayMonitorsParam.p_info[i].display.height
        );
    }

    printf("\nTotal active displays found: %d\n\n",
        enumDisplayMonitorsParam.length
    );

    return STATUS_SUCCESS;
}


/* Global Callback Function Implementations ==================================*/
