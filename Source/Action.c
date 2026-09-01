/*==============================================================================
File name   : Action.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the various window actions that GridLock can
              perform.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "Action.h"

#include "GridLock.h"
#include "Box.h"


/* Local Structures/Macros ===================================================*/


/* Private Member Variables ==================================================*/
//------------------------------------------------------------------------------
static dimension32_t Minimum;

//------------------------------------------------------------------------------
static HWND h_LastWindow;
static Box_t ReversionBox;
static Box_t LastActionBox;
static bool LastActionWasMinimize;

//------------------------------------------------------------------------------
static Box_t LastSequenceActionBox;
static const HotkeyAction_t* p_LastSequenceList;
static uint16_t LastSequenceActionIndex;


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline int32_t MultiplyByRatio(int32_t input, const ratio32_t* p_ratio);

//------------------------------------------------------------------------------
static status_t GetInlayBox(const Box_t* p_display, const InputRatio_t* p_inlay,
    Box_t* p_box
);
static status_t GetResizeDimensions(const InputRatio_t* p_ratio,
    Box_t* p_display, Box_t* p_window, dimension32_t* p_dimension
);

//------------------------------------------------------------------------------
static JumpDisplay_t JumpIfOnEdge(const Box_t* restrict p_target,
    const Box_t* restrict p_display
);

//------------------------------------------------------------------------------
static status_t SavePreAction(HWND h_window);
static status_t SavePostAction(const Box_t* p_latest);
static status_t SaveSequenceAction(const Box_t* p_latest,
    const HotkeyAction_t* p_sequenceList, uint16_t index
);


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline int32_t MultiplyByRatio(int32_t input, const ratio32_t* p_ratio)
{
    assert(p_ratio);
    assert(0 != p_ratio->denominator);

    int64_t product = ((int64_t)input) * p_ratio->numerator;
    product = ((product < 0) == (p_ratio->denominator < 0))
        ? product + (p_ratio->denominator / 2)
        : product - (p_ratio->denominator / 2);
    product /= p_ratio->denominator;
    product = Int64_ClampMin(product, INT32_MIN);
    product = Int64_ClampMax(product, INT32_MAX);
    return (int32_t)product;
}

//------------------------------------------------------------------------------
static status_t GetInlayBox(const Box_t* p_display, const InputRatio_t* p_inlay,
    Box_t* p_box)
{
    assert(p_inlay);
    assert(p_box);

    switch (p_inlay->type)
    {
        default:
        case (INPUTTYPE_UNKNOWN):
        case (INPUTTYPE_WINDOW_W):
        case (INPUTTYPE_WINDOW_H):
        case (INPUTTYPE_WINDOW):
        {
            printd("Invalid GetInlayBox inlay type\n");
        }
        return STATUS_INVALIDPARAM;

        case (INPUTTYPE_NONE):
        {
            *p_box = *p_display;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_SCALAR):
        {
            p_box->left =
                Int32_Add(
                    p_display->left, Int32_Divide(p_inlay->scalar.width, 2)
                );
            p_box->top =
                Int32_Add(
                    p_display->top, Int32_Divide(p_inlay->scalar.height, 2)
                );
            p_box->width =
                Int32_Subtract(p_display->width, p_inlay->scalar.width);
            p_box->height =
                Int32_Subtract(p_display->height, p_inlay->scalar.height);
        }
        break;

        case (INPUTTYPE_DISPLAY_W):
        {
            // Symmetric inlay into the display, as a ratio of display width //
            p_box->width = MultiplyByRatio(p_display->width, &(p_inlay->ratio));

            p_box->height =
                Int32_Subtract(
                    p_display->height,
                    Int32_Subtract(p_display->width, p_box->width)
                );
            p_box->left =
                Int32_Add(
                    p_display->left,
                    Int32_Divide(
                        Int32_Subtract(p_display->width, p_box->width), 2
                    )
                );
            p_box->top =
                Int32_Add(
                    p_display->top,
                    Int32_Divide(
                        Int32_Subtract(p_display->height, p_box->height), 2
                    )
                );
        }
        break;

        case (INPUTTYPE_DISPLAY_H):
        {
            // Symmetric inlay into the display, as a ratio of display height //
            p_box->height =
                MultiplyByRatio(p_display->height, &(p_inlay->ratio));

            p_box->width =
                Int32_Subtract(
                    p_display->width,
                    Int32_Subtract(p_display->height, p_box->height)
                );
            p_box->left =
                Int32_Add(
                    p_display->left,
                    Int32_Divide(
                        Int32_Subtract(p_display->width, p_box->width), 2
                    )
                );
            p_box->top =
                Int32_Add(
                    p_display->top,
                    Int32_Divide(
                        Int32_Subtract(p_display->height, p_box->height), 2
                    )
                );
        }
        break;

        case (INPUTTYPE_DISPLAY):
        {
            p_box->width =
                MultiplyByRatio(p_display->width, &(p_inlay->dualratio.width));
            p_box->height =
                MultiplyByRatio(
                    p_display->height, &(p_inlay->dualratio.height)
                );

            p_box->left =
                Int32_Add(
                    p_display->left,
                    Int32_Divide(
                        Int32_Subtract(p_display->width, p_box->width), 2
                    )
                );
            p_box->top =
                Int32_Add(
                    p_display->top,
                    Int32_Divide(
                        Int32_Subtract(p_display->height, p_box->height), 2
                    )
                );
        }
        break;
    }
    // end switch //

    SacPrintReturnIf((p_box->width <= 0) || (p_box->width < Minimum.width),
        STATUS_FAILURE,
        "Resulting inlay width was below minimum\n"
    );
    SacPrintReturnIf((p_box->height <= 0) || (p_box->height < Minimum.height),
        STATUS_FAILURE,
        "Resulting inlay height was below minimum\n"
    );

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t GetResizeDimensions(const InputRatio_t* p_ratio,
    Box_t* p_display, Box_t* p_window, dimension32_t* p_dimension)
{
    assert(p_ratio);
    assert(p_display);
    assert(p_window);
    assert(p_dimension);

    switch (p_ratio->type)
    {
        default:
        case (INPUTTYPE_UNKNOWN):
        {
            printd("Invalid GetResizeDimensions resize type\n");
        }
        return STATUS_INVALIDPARAM;

        case (INPUTTYPE_NONE):
        {
            p_dimension->width  = 0;
            p_dimension->height = 0;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_SCALAR):
        {
            // Config already enforces divisibility by 2 for INPUTTYPE_SCALAR //
            p_dimension->width  = p_ratio->scalar.width;
            p_dimension->height = p_ratio->scalar.height;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_DISPLAY_W):
        {
            const int32_t width =
                MultiplyByRatio(p_display->width, &(p_ratio->ratio)) & ~1;
            p_dimension->width  = width;
            p_dimension->height = width;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_DISPLAY_H):
        {
            const int32_t height =
                MultiplyByRatio(p_display->height, &(p_ratio->ratio)) & ~1;
            p_dimension->width  = height;
            p_dimension->height = height;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_DISPLAY):
        {
            p_dimension->width =
                MultiplyByRatio(p_display->width, &(p_ratio->dualratio.width))
                & ~1;
            p_dimension->height =
                MultiplyByRatio(p_display->height, &(p_ratio->dualratio.height))
                & ~1;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_WINDOW_W):
        {
            const int32_t width =
                MultiplyByRatio(p_window->width, &(p_ratio->ratio)) & ~1;
            p_dimension->width  = width;
            p_dimension->height = width;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_WINDOW_H):
        {
            const int32_t height =
                MultiplyByRatio(p_window->height, &(p_ratio->ratio)) & ~1;
            p_dimension->width  = height;
            p_dimension->height = height;
        }
        return STATUS_SUCCESS;

        case (INPUTTYPE_WINDOW):
        {
            p_dimension->width =
                MultiplyByRatio(p_window->width, &(p_ratio->dualratio.width))
                & ~1;
            p_dimension->height =
                MultiplyByRatio(p_window->height, &(p_ratio->dualratio.height))
                & ~1;
        }
        return STATUS_SUCCESS;
    }
    // end switch //
}

//------------------------------------------------------------------------------
static JumpDisplay_t JumpIfOnEdge(const Box_t* restrict p_target,
    const Box_t* restrict p_display)
{
    BoxEdgeState_t edgeState = Box_GetEdgeState(p_target, p_display);
    if (edgeState.left && (false == edgeState.right)
        && (edgeState.top == edgeState.bottom))
    {
        return JUMP_DISPLAY_LEFT;
    }
    else if (edgeState.right && (false == edgeState.left)
        && (edgeState.top == edgeState.bottom))
    {
        return JUMP_DISPLAY_RIGHT;
    }
    else if (edgeState.top && (false == edgeState.bottom)
        && (edgeState.left == edgeState.right))
    {
        return JUMP_DISPLAY_UP;
    }
    else if (edgeState.bottom && (false == edgeState.top)
        && (edgeState.left == edgeState.right))
    {
        return JUMP_DISPLAY_DOWN;
    }

    // Nothing to do. We aren't at a specific edge. //
    return JUMP_DISPLAY_NONE;
}

//------------------------------------------------------------------------------
static status_t SavePreAction(HWND h_window)
{
    if (Window_IsMinimized(h_window) || Window_IsMaximized(h_window))
    {
        // Don't save maximized windows as the original //
        return STATUS_SUCCESS;
    }

    Box_t before;
    SacReturnIfErr(Window_GetBox(h_window, &before), STATUS_FAILURE);

    if (false == Box_IsEqual(&before, &LastActionBox))
    {
        h_LastWindow = h_window;
        ReversionBox = Box_Copy(&before);
        return STATUS_SUCCESS;
    }

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t SavePostAction(const Box_t* p_latest)
{
    assert(p_latest);

    // All actions - except Minimize - do not change the active window //
    LastActionBox = Box_Copy(p_latest);
    LastActionWasMinimize = false;
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
static status_t SaveSequenceAction(const Box_t* p_latest,
    const HotkeyAction_t* p_sequenceList, uint16_t index)
{
    assert(p_latest);
    assert(p_sequenceList);

    LastSequenceActionBox   = Box_Copy(p_latest);
    p_LastSequenceList      = p_sequenceList;
    LastSequenceActionIndex = index;
    return STATUS_SUCCESS;
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Action_Init
//------------------------------------------------------------------------------
status_t Action_Init(WindowConfig_t* p_windowConfig)
{
    assert(p_windowConfig);

    Minimum.width  = Int32_ClampMin(p_windowConfig->minimum.width, 0);
    Minimum.height = Int32_ClampMin(p_windowConfig->minimum.height, 0);
    SacReturnIfErr(
        Window_InitBorderVisibility(p_windowConfig->borderVisibility),
        STATUS_FAILURE
    );
    SacReturnIfErr(
        Window_InitIsWindowArranged(p_windowConfig->p_IsWindowArranged),
        STATUS_FAILURE
    );

    h_LastWindow          = NULL;
    ReversionBox          = (Box_t){0};
    LastActionBox         = (Box_t){0};
    LastActionWasMinimize = false;

    LastSequenceActionBox   = (Box_t){0};
    p_LastSequenceList      = NULL;
    LastSequenceActionIndex = 0;

#ifndef NDEBUG
    SacReturnIfErr(Display_QueryAllDisplays(), STATUS_FAILURE);
#endif

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Help
//------------------------------------------------------------------------------
status_t Action_Help(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    (void)h_window;
    (void)p_param;

    printf("Action = [ Help ]\n");

    return GridLock_Help();
}

//------------------------------------------------------------------------------
// Action_Revert
//------------------------------------------------------------------------------
status_t Action_Revert(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);

    (void)p_param;

    printf("Action = [ Revert ]\n");

    if (NULL == h_LastWindow)
    {
        // We have not done anything with GridLock yet, nothing to revert //
        printf("Revert = [ Nothing to revert ]\n");
        return STATUS_SUCCESS;
    }

    // All actions - except Minimize - change the active window //
    // Handle the special case where Minimize was the last action //
    if (LastActionWasMinimize)
    {
        SacReturnIfErr(Window_Restore(h_LastWindow), STATUS_FAILURE);

        Box_t current;
        SacReturnIfErr(Window_GetBox(h_LastWindow, &current), STATUS_FAILURE);
        SavePostAction(&current);

        return STATUS_SUCCESS;
    }

    if (h_LastWindow != h_window)
    {
        // Not our saved window, so we can't revert //
        printf("Revert = [ Window lost ]\n");
        return STATUS_SUCCESS;
    }

    printf("Revert = [ Reverting ]\n");
    if (Window_IsMinimized(h_window) || Window_IsMaximized(h_window))
    {
        SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
    }

    SacReturnIfErr(Window_SetBox(h_window, &ReversionBox), STATUS_FAILURE);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Minimize
//------------------------------------------------------------------------------
status_t Action_Minimize(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const MinimizeParam_t* p_minimize = &(p_param->asMinimize);

    printf("Action = [ Minimize ]\n");

    // A previous minimize action will switch the active window //
    // If toggle is enabled, we must fetch the minimize window and restore it //
    if (p_minimize->toggle
        && NULL != h_LastWindow && Window_IsMinimized(h_LastWindow))
    {
        SacReturnIfErr(Window_Restore(h_LastWindow), STATUS_FAILURE);

        Box_t current;
        SacReturnIfErr(Window_GetBox(h_LastWindow, &current), STATUS_FAILURE);
        SavePostAction(&current);
        return STATUS_SUCCESS;
    }

    SavePreAction(h_window);

    if (Window_IsMinimized(h_window))
    {
        if (false == p_minimize->toggle)
        {
            return STATUS_SUCCESS;
        }

        SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);

        Box_t current;
        SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
        SavePostAction(&current);
        return STATUS_SUCCESS;
    }

    // Minimize the window //
    // Note that this is the only action which changes the active window //
    SacReturnIfErr(Window_Minimize(h_window), STATUS_FAILURE);

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
    SavePostAction(&current);
    LastActionWasMinimize = true;
    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Maximize
//------------------------------------------------------------------------------
status_t Action_Maximize(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const MaximizeParam_t* p_maximize = &(p_param->asMaximize);

    SavePreAction(h_window);

    printf("Action = [ Maximize ]\n");

    if (Window_IsMaximized(h_window))
    {
        if (false == p_maximize->toggle)
        {
            return STATUS_SUCCESS;
        }

        SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);

        Box_t current;
        SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
        SavePostAction(&current);
        return STATUS_SUCCESS;
    }

    SacReturnIfErr(Window_Maximize(h_window), STATUS_FAILURE);

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
    SavePostAction(&current);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Tile
//------------------------------------------------------------------------------
status_t Action_Tile(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);
    assert(0 < p_param->asTile.columns);
    assert(0 < p_param->asTile.rows);
    assert(0 < p_param->asTile.index);
    assert(
        (int32_t)(p_param->asTile.index)
        <= ((int32_t)(p_param->asTile.columns)
            * (int32_t)(p_param->asTile.rows))
    );
    assert(0 < p_param->asTile.width);
    assert(0 < p_param->asTile.height);

    const TileParam_t* p_tile = &(p_param->asTile);

    SavePreAction(h_window);

    printf(
        "Action = [ Tile ]\n"
        "Parameters = [ InlayType = %s, Columns = %d, Rows = %d, Index = %d,"
        " Width = %d, Height = %d, Transpose = %s, Jump = %s ]\n",
        InputTypeToString(p_tile->inlay.type),
        p_tile->columns, p_tile->rows, p_tile->index,
        p_tile->width, p_tile->height,
        (p_tile->transpose) ? "true" : "false",
        (p_tile->jump) ? "true" : "false"
    );

    Box_t display;
    SacReturnIfErr(Display_GetBox(h_window, &display), STATUS_FAILURE);

    const bool transpose =
        (p_tile->transpose && (display.width < display.height));

    InputRatio_t input = p_tile->inlay;
    if (transpose)
    {
        switch (input.type)
        {
            default: break;

            case (INPUTTYPE_SCALAR):
            {
                const int32_t intermediate = input.scalar.width;
                input.scalar.width = input.scalar.height;
                input.scalar.height = intermediate;
            }
            break;

            case (INPUTTYPE_DISPLAY_W):
            {
                input.type = INPUTTYPE_DISPLAY_H;
            }
            break;

            case (INPUTTYPE_DISPLAY_H):
            {
                input.type = INPUTTYPE_DISPLAY_W;
            }
            break;

            case (INPUTTYPE_DISPLAY):
            {
                const ratio32_t intermediate = input.dualratio.width;
                input.dualratio.width = input.dualratio.height;
                input.dualratio.height = intermediate;
            }
            break;
        }
        // end switch //
    }

    Box_t inlay;
    SacReturnIfErr(GetInlayBox(&display, &input, &inlay), STATUS_FAILURE);

    const int32_t columns = (transpose) ? p_tile->rows : p_tile->columns;
    const int32_t rows    = (transpose) ? p_tile->columns : p_tile->rows;

    Box_t target;
    target.width = (inlay.width / columns);
    target.height = (inlay.height / rows);

    // Zero-indexed index //
    // Decompose using the original column count so that cell N in the //
    // configured grid maps to a valid (X, Y) in the transposed grid //
    const int32_t index = (p_tile->index - 1);
    const int32_t indexHorizontal =
        (transpose) ? (index / p_tile->columns) : (index % p_tile->columns);
    const int32_t indexVertical =
        (transpose) ? (index % p_tile->columns) : (index / p_tile->columns);

    const int32_t widen =    (transpose) ? p_tile->height : p_tile->width;
    const int32_t heighten = (transpose) ? p_tile->width : p_tile->height;

    SacPrintReturnIf(
        columns <= indexHorizontal + (widen - 1), STATUS_INVALIDPARAM,
        "index + width exceeds display boundaries\n"
    );
    SacPrintReturnIf(
        rows <= indexVertical + (heighten - 1), STATUS_INVALIDPARAM,
        "index + height exceeds display boundaries\n"
    );

    target.left =
        Int32_Add(inlay.left, Int32_Multiply(indexHorizontal, target.width));
    target.top =
        Int32_Add(inlay.top, Int32_Multiply(indexVertical, target.height));
    target.width  = Int32_Multiply(target.width, widen);
    target.height = Int32_Multiply(target.height, heighten);

    SacPrintReturnIf((target.width <= 0), STATUS_INVALIDPARAM,
        "Resulting tile would have no width\n"
    );
    SacPrintReturnIf((target.height <= 0), STATUS_INVALIDPARAM,
        "Resulting tile would have no height\n"
    );

    // Check if this move is a no-op - we then move to the next display //
    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
    if (Box_IsEqual(&target, &current))
    {
        if (false == p_tile->jump)
        {
            return STATUS_SUCCESS;
        }

        JumpDisplay_t direction = JumpIfOnEdge(&target, &display);
        if (JUMP_DISPLAY_NONE == direction)
        {
            return STATUS_SUCCESS;
        }

        Box_t nextDisplay;
        SacReturnIfErr(
            Display_GetJumpBox(&display, direction, &nextDisplay),
            STATUS_FAILURE
        );
#ifndef NDEBUG
        Box_Print("Next display", &nextDisplay);
#endif

        Box_ShiftInBounds(&target, &nextDisplay);
        SacReturnIfNot(Box_ClipToBounds(&target, &nextDisplay), STATUS_FAILURE);
    }

    SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
    SacReturnIfErr(Window_SetBox(h_window, &target), STATUS_FAILURE);
    SavePostAction(&target);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Move
//------------------------------------------------------------------------------
status_t Action_Move(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const MoveParam_t* p_move = &(p_param->asMove);

    SavePreAction(h_window);

    printf(
        "Action = [ Move ]\n"
        "Parameters = [ Horizontal = %d / %d, Vertical = %d / %d, Jump = %s"
        " ]\n",
        p_move->horizontal.numerator, p_move->horizontal.denominator,
        p_move->vertical.numerator, p_move->vertical.denominator,
        (p_move->jump) ? "true" : "false"
    );

    Box_t display;
    SacReturnIfErr(Display_GetBox(h_window, &display), STATUS_FAILURE);

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);

    int32_t horizontal = MultiplyByRatio(display.width, &(p_move->horizontal));
    int32_t vertical   = MultiplyByRatio(display.height, &(p_move->vertical));

    Box_t target;
    target.left =
        Int32_Add(
            display.left,
            Int32_Subtract(horizontal, Int32_Divide(current.width, 2))
        );
    target.top =
        Int32_Add(
            display.top,
            Int32_Subtract(vertical, Int32_Divide(current.height, 2))
        );
    target.width  = current.width;
    target.height = current.height;

    Box_ShiftInBounds(&target, &display);

    // Check if this move is a no-op - we then move to the next display //
    if (Box_IsEqual(&target, &current))
    {
        if (false == p_move->jump)
        {
            return STATUS_SUCCESS;
        }

        JumpDisplay_t direction = JumpIfOnEdge(&target, &display);
        if (JUMP_DISPLAY_NONE == direction)
        {
            return STATUS_SUCCESS;
        }

        Box_t nextDisplay;
        SacPrintReturnIfErr(
            Display_GetJumpBox(&display, direction, &nextDisplay),
            STATUS_FAILURE,
            "Failed to get jump display box\n"
        );
#ifndef NDEBUG
        Box_Print("Next display", &nextDisplay);
#endif

        Box_ShiftInBounds(&target, &nextDisplay);
        SacPrintReturnIfNot(
            Box_ClipToBounds(&target, &nextDisplay),
            STATUS_FAILURE,
            "Failed to clip to bounds\n"
        );
    }

#ifndef NDEBUG
        Box_Print("Target ", &target);
#endif

    SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
    SacReturnIfErr(Window_MoveBox(h_window, &target), STATUS_FAILURE);
    SavePostAction(&target);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Jump
//------------------------------------------------------------------------------
status_t Action_Jump(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const JumpParam_t* p_jump = &(p_param->asJump);

    SavePreAction(h_window);

    printf(
        "Action = [ Jump ]\n"
        "Parameters = [ Direction = %s ]\n",
        JumpDirectionToString(p_jump->direction)
    );

    Box_t display;
    SacReturnIfErr(Display_GetBox(h_window, &display), STATUS_FAILURE);

    Box_t nextDisplay;
    SacReturnIfErr(
        Display_GetJumpBox(
            &display, p_jump->direction, &nextDisplay
        ),
        STATUS_FAILURE
    );

    if (Box_IsEqual(&display, &nextDisplay))
    {
        return STATUS_SUCCESS;
    }

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);

    Box_t target;
    target.left =
        Int32_Add(
            nextDisplay.left,
            Int32_Subtract(
                Int32_Divide(nextDisplay.width, 2),
                Int32_Divide(current.width, 2)
            )
        );
    target.top =
        Int32_Add(
            nextDisplay.top,
            Int32_Subtract(
                Int32_Divide(nextDisplay.height, 2),
                Int32_Divide(current.height, 2)
            )
        );
    target.width  = current.width;
    target.height = current.height;
    Box_ShiftInBounds(&target, &nextDisplay);
    SacPrintReturnIfNot(Box_ClipToBounds(&target, &nextDisplay), STATUS_FAILURE,
        "Box_ClipToBounds would have produced no intersection.\n"
    );

    SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
    SacReturnIfErr(Window_MoveBox(h_window, &target), STATUS_FAILURE);
    SavePostAction(&target);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Increase
//------------------------------------------------------------------------------
status_t Action_Increase(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const IncreaseParam_t* p_increase = &(p_param->asIncrease);

    SavePreAction(h_window);

    printf(
        "Action = [ Increase ]\n"
        "Parameters = [ InlayType = %s, Resize = <%d / %d> <%d / %d> ]\n",
        InputTypeToString(p_increase->resize.type),
        p_increase->resize.ratio.numerator,
        p_increase->resize.ratio.denominator,
        p_increase->resize.dualratio.height.numerator,
        p_increase->resize.dualratio.height.denominator
    );

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);

    if (Window_IsMaximized(h_window))
    {
        // Can't maximize further //
        return STATUS_SUCCESS;
    }

    if (Window_IsMinimized(h_window))
    {
        SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
    }

    Box_t display;
    SacReturnIfErr(Display_GetBox(h_window, &display), STATUS_FAILURE);

    if (display.width <= current.width && display.height <= current.height)
    {
        // Already at full screen, so maximize //
        SacReturnIfErr(Window_Maximize(h_window), STATUS_FAILURE);
        SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);

        return STATUS_SUCCESS;
    }

    dimension32_t difference =
    {
        .width  = 0,
        .height = 0,
    };
    SacReturnIfErr(
        GetResizeDimensions(&(p_increase->resize), &display, &current,
            &difference
        ),
        STATUS_FAILURE
    );

    Box_t target =
    {
        .left =
            Int32_Subtract(
                current.left, Int32_Divide(difference.width, 2)
            ),
        .top =
            Int32_Subtract(
                current.top, Int32_Divide(difference.height, 2)
            ),
        .width  = Int32_Add(current.width, difference.width),
        .height = Int32_Add(current.height, difference.height)
    };

    SacPrintReturnIfNot(Box_ClipToBounds(&target, &display), STATUS_FAILURE,
        "Box_ClipToBounds would have produced no intersection.\n"
    );

    if ((display.width <= target.width) && (display.height <= target.height))
    {
        // Set to full screen without maximizing //
        target = Box_Copy(&display);
        SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
        SacReturnIfErr(Window_SetBox(h_window, &target), STATUS_FAILURE);
        SavePostAction(&target);

        return STATUS_SUCCESS;
    }

    Box_ShiftInBounds(&target, &display);
    SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
    SacReturnIfErr(Window_SetBox(h_window, &target), STATUS_FAILURE);
    SavePostAction(&target);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Decrease
//------------------------------------------------------------------------------
status_t Action_Decrease(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const DecreaseParam_t* p_decrease = &(p_param->asDecrease);

    SavePreAction(h_window);

    printf(
        "Action = [ Decrease ]\n"
        "Parameters = [ InlayType = %s, Resize = <%d / %d> <%d / %d>,"
        " edgelock = %s, restore = %s ]\n",
        InputTypeToString(p_decrease->resize.type),
        p_decrease->resize.ratio.numerator,
        p_decrease->resize.ratio.denominator,
        p_decrease->resize.dualratio.height.numerator,
        p_decrease->resize.dualratio.height.denominator,
        (p_decrease->edgelock) ? "true" : "false",
        (p_decrease->restore) ? "true" : "false"
    );

    if (Window_IsMinimized(h_window))
    {
        // Can't minimize further //
        return STATUS_SUCCESS;
    }

    if (Window_IsMaximized(h_window))
    {
        SacReturnIfErr(Window_Restore(h_window), STATUS_FAILURE);
        if (p_decrease->restore)
        {
            Box_t current;
            SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
            SavePostAction(&current);
            return STATUS_SUCCESS;
        }

        Box_t display;
        SacReturnIfErr(Display_GetBox(h_window, &display), STATUS_FAILURE);
        SacReturnIfErr(Window_SetBox(h_window, &display), STATUS_FAILURE);
        SavePostAction(&display);
        return STATUS_SUCCESS;
    }

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);

    dimension32_t minimum;
    const status_t valid = Window_GetMinimumSize(h_window, &minimum);
    if (STATUS_SUCCESS != valid)
    {
        minimum.width = 0;
        minimum.height = 0;
    }
    minimum.width = Int32_ClampMin(minimum.width, Minimum.width);
    minimum.height = Int32_ClampMin(minimum.height, Minimum.height);

    if (current.width <= minimum.width && current.height <= minimum.height)
    {
        return STATUS_SUCCESS;
    }

    Box_t display;
    SacReturnIfErr(Display_GetBox(h_window, &display), STATUS_FAILURE);

    dimension32_t difference =
    {
        .width = 0,
        .height = 0,
    };
    SacReturnIfErr(
        GetResizeDimensions(&(p_decrease->resize), &display, &current,
            &difference
        ),
        STATUS_FAILURE
    );

    difference.width =
        Int32_Subtract(
            current.width,
            Int32_ClampMin(
                Int32_Subtract(current.width, difference.width),
                minimum.width
            )
        );
    difference.height =
        Int32_Subtract(
            current.height,
            Int32_ClampMin(
                Int32_Subtract(current.height, difference.height),
                minimum.height
            )
        );

    if (false == p_decrease->edgelock)
    {
        Box_t target =
        {
            .left = Int32_Add(current.left, Int32_Divide(difference.width, 2)),
            .top = Int32_Add(current.top, Int32_Divide(difference.height, 2)),
            .width = Int32_Subtract(current.width, difference.width),
            .height = Int32_Subtract(current.height, difference.height)
        };
        SacReturnIfErr(Window_SetBox(h_window, &target), STATUS_FAILURE);
        SavePostAction(&target);
        return STATUS_SUCCESS;
    }

    // If all or none of the edges are locked, decrease from each edge //
    const BoxEdgeState_t edgeState = Box_GetEdgeState(&current, &display);
    if (edgeState.left == edgeState.top
        && edgeState.left == edgeState.right
        && edgeState.left == edgeState.bottom)
    {
        Box_t target =
        {
            .left = Int32_Add(current.left, Int32_Divide(difference.width, 2)),
            .top = Int32_Add(current.top, Int32_Divide(difference.height, 2)),
            .width = Int32_Subtract(current.width, difference.width),
            .height = Int32_Subtract(current.height, difference.height)
        };
        SacReturnIfErr(Window_SetBox(h_window, &target), STATUS_FAILURE);
        SavePostAction(&target);
        return STATUS_SUCCESS;
    }

    // Otherwise, only decrease each non-edgelocked window edge by half of //
    // its axis's contribution. //
    Box_t target = Box_Copy(&current);
    if (false == edgeState.left && false == edgeState.right)
    {
        target.left = Int32_Add(target.left, Int32_Divide(difference.width, 2));
        target.width = Int32_Subtract(target.width, difference.width);
    }
    else if (edgeState.left && false == edgeState.right)
    {
        target.width =
            Int32_Subtract(target.width, Int32_Divide(difference.width, 2));
    }
    else if (false == edgeState.left && edgeState.right)
    {
        target.left = Int32_Add(target.left, Int32_Divide(difference.width, 2));
        target.width =
            Int32_Subtract(target.width, Int32_Divide(difference.width, 2));
    }

    if (false == edgeState.top && false == edgeState.bottom)
    {
        target.top = Int32_Add(target.top, Int32_Divide(difference.height, 2));
        target.height = Int32_Subtract(target.height, difference.height);
    }
    else if (edgeState.top && false == edgeState.bottom)
    {
        target.height =
            Int32_Subtract(target.height, Int32_Divide(difference.height, 2));
    }
    else if (false == edgeState.top && edgeState.bottom)
    {
        target.top = Int32_Add(target.top, Int32_Divide(difference.height, 2));
        target.height =
            Int32_Subtract(target.height, Int32_Divide(difference.height, 2));
    }

    Box_ShiftInBounds(&target, &display);
    SacReturnIfErr(Window_SetBox(h_window, &target), STATUS_FAILURE);
    SavePostAction(&target);

    return STATUS_SUCCESS;
}

//------------------------------------------------------------------------------
// Action_Project
//------------------------------------------------------------------------------
status_t Action_Project(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const ProjectParam_t* p_project = &(p_param->asProject);

    (void)h_window;

    printf(
        "Action = [ Project ]\n"
        "Parameters = [ Mode = %s ]\n",
        ProjectModeToString(p_project->mode)
    );

    return Display_Project(p_project->mode);
}

//------------------------------------------------------------------------------
// Action_Sequence
//------------------------------------------------------------------------------
status_t Action_Sequence(HWND h_window, const ActionParameter_t* p_param)
{
    assert(h_window);
    assert(p_param);

    const SequenceParam_t* p_sequence = &(p_param->asSequence);

    printf(
        "Action = [ Sequence ]\n"
        "Parameters = [ Count = %d ]\n",
        p_sequence->count
    );

    const uint16_t count = p_sequence->count;
    if (count <= 0)
    {
        return STATUS_SUCCESS;
    }

    const HotkeyAction_t* p_actions = p_sequence->p_actions;
    SacPrintReturnIfNull(p_actions, STATUS_FAILURE, "Null p_actions\n");

    Box_t current;
    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);

    uint16_t index = 0;
    if (p_actions == p_LastSequenceList
        && Box_IsEqual(&current, &LastSequenceActionBox))
    {
        index = LastSequenceActionIndex + 1;
        if (count <= index)
        {
            index = 0;
        }
    }

    const HotkeyAction_t* p_action = &(p_actions[index]);
    SacPrintReturnIfNull(p_action->p_function, STATUS_FAILURE,
        "Null sequence action\n"
    );

    SacPrintReturnIfErr(
        p_action->p_function(h_window, &(p_action->param)),
        STATUS_FAILURE,
        "Sequence action failed!\n"
    );

    SacReturnIfErr(Window_GetBox(h_window, &current), STATUS_FAILURE);
    SacReturnIfErr(SaveSequenceAction(&current, p_actions, index),
        STATUS_FAILURE
    );

    return STATUS_SUCCESS;
}


/* Global Callback Function Implementations ==================================*/
