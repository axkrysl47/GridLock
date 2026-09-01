/*==============================================================================
File name   : Box.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the Box_t struct which is used to define
              window positions and sizes within GridLock.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <assert.h>

#include "Box.h"

#include "GridLock.h"
#include "Integer.h"


/* Local Structures/Macros ===================================================*/


/* Private Helper Function Prototypes ========================================*/
//------------------------------------------------------------------------------
static inline int32_t BoxRight(const Box_t* p_box);
static inline int32_t BoxBottom(const Box_t* p_box);


/* Private Member Variables ==================================================*/


/* Private Function Implementations ==========================================*/
//------------------------------------------------------------------------------
static inline int32_t BoxRight(const Box_t* p_box)
{
    assert(p_box);

    return Int32_Add(p_box->left, p_box->width);
}

//------------------------------------------------------------------------------
static inline int32_t BoxBottom(const Box_t* p_box)
{
    assert(p_box);

    return Int32_Add(p_box->top, p_box->height);
}


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Box_RectToBox
//------------------------------------------------------------------------------
bool Box_RectToBox(const RECT *p_rect, Box_t* p_box)
{
    assert(p_rect);
    assert(p_box);

    SacReturnIf(p_rect->right < p_rect->left, false);
    SacReturnIf(p_rect->bottom < p_rect->top, false);

    p_box->left   = p_rect->left;
    p_box->top    = p_rect->top;
    p_box->width  = p_rect->right  - p_rect->left;
    p_box->height = p_rect->bottom - p_rect->top;
    return true;
}

//------------------------------------------------------------------------------
// Box_RectFromBox
//------------------------------------------------------------------------------
RECT Box_RectFromBox(const Box_t* p_box)
{
    assert(p_box);

    RECT rectangle =
    {
        .left   = p_box->left,
        .top    = p_box->top,
        .right  = Int32_Add(p_box->left, p_box->width),
        .bottom = Int32_Add(p_box->top, p_box->height),
    };
    return rectangle;
}

//------------------------------------------------------------------------------
// Box_Copy
//------------------------------------------------------------------------------
Box_t Box_Copy(const Box_t* p_box)
{
    assert(p_box);

    Box_t copy =
    {
        .left   = p_box->left,
        .top    = p_box->top,
        .width  = p_box->width,
        .height = p_box->height,
    };
    return copy;
}

//------------------------------------------------------------------------------
// Box_IsEqual
//------------------------------------------------------------------------------
bool Box_IsEqual(const Box_t* p_first, const Box_t* p_second)
{
    assert(p_first);
    assert(p_second);

    return (p_first->left   == p_second->left)
        && (p_first->top    == p_second->top)
        && (p_first->width  == p_second->width)
        && (p_first->height == p_second->height);
}

//------------------------------------------------------------------------------
// Box_GetEdgeState
//------------------------------------------------------------------------------
BoxEdgeState_t Box_GetEdgeState(const Box_t* p_box, const Box_t* p_boundary)
{
    assert(p_box);
    assert(p_boundary);

    BoxEdgeState_t state =
    {
        .left   = (p_box->left <= p_boundary->left),
        .top    = (p_box->top  <= p_boundary->top),
        .right  = (BoxRight(p_boundary)  <= BoxRight(p_box)),
        .bottom = (BoxBottom(p_boundary) <= BoxBottom(p_box))
    };
    return state;
}

//------------------------------------------------------------------------------
// Box_SymmetricIncrement
//------------------------------------------------------------------------------
void Box_SymmetricIncrement(Box_t* p_box)
{
    assert(p_box);

    p_box->left   = Int32_Subtract(p_box->left  , 1);
    p_box->top    = Int32_Subtract(p_box->top   , 1);
    p_box->width  = Int32_Add(     p_box->width , 2);
    p_box->height = Int32_Add(     p_box->height, 2);
}

//------------------------------------------------------------------------------
// Box_SymmetricDecrement
//------------------------------------------------------------------------------
void Box_SymmetricDecrement(Box_t* p_box)
{
    assert(p_box);

    if (p_box->width < 2)
    {
        p_box->width  = 0;
    }
    else
    {
        p_box->left   = Int32_Add(p_box->left, 1);
        p_box->width  = Int32_Subtract(p_box->width , 2);
    }

    if (p_box->height < 2)
    {
        p_box->height = 0;
    }
    else
    {
        p_box->top    = Int32_Add(p_box->top, 1);
        p_box->height = Int32_Subtract(p_box->height, 2);
    }

}

//------------------------------------------------------------------------------
// Box_Add
//------------------------------------------------------------------------------
void Box_Add(Box_t* restrict p_sum, const Box_t* restrict p_addend1,
    const Box_t* restrict p_addend2)
{
    assert(p_sum);
    assert(p_addend1);
    assert(p_addend2);

    p_sum->left   = Int32_Add(p_addend1->left  , p_addend2->left  );
    p_sum->top    = Int32_Add(p_addend1->top   , p_addend2->top   );
    p_sum->width  = Int32_Add(p_addend1->width , p_addend2->width );
    p_sum->height = Int32_Add(p_addend1->height, p_addend2->height);
}

//------------------------------------------------------------------------------
// Box_Subtract
//------------------------------------------------------------------------------
void Box_Subtract(Box_t* restrict p_difference, const Box_t* restrict p_minuend,
    const Box_t* restrict p_subtrahend)
{
    assert(p_difference);
    assert(p_minuend);
    assert(p_subtrahend);

    p_difference->left =
        Int32_Subtract(p_minuend->left  , p_subtrahend->left);
    p_difference->top =
        Int32_Subtract(p_minuend->top   , p_subtrahend->top);
    p_difference->width =
        Int32_Subtract(p_minuend->width , p_subtrahend->width);
    p_difference->height =
        Int32_Subtract(p_minuend->height, p_subtrahend->height);
}

//------------------------------------------------------------------------------
// Box_ShiftInBounds
//------------------------------------------------------------------------------
void Box_ShiftInBounds(Box_t* restrict p_box, const Box_t* restrict p_boundary)
{
    assert(p_box);
    assert(p_boundary);

    p_box->left =
        Int32_ClampMax(
            Int32_Add(p_box->left, p_box->width),
            Int32_Add(p_boundary->left, p_boundary->width)
        )
        - p_box->width;
    p_box->top =
        Int32_ClampMax(
            Int32_Add(p_box->top, p_box->height),
            Int32_Add(p_boundary->top, p_boundary->height)
        )
        - p_box->height;

    // Do left/uppermost last, so the top left corner takes precedence //
    p_box->left = Int32_ClampMin(p_box->left, p_boundary->left);
    p_box->top = Int32_ClampMin(p_box->top, p_boundary->top);
}

//------------------------------------------------------------------------------
// Box_ClipToBounds
//------------------------------------------------------------------------------
bool Box_ClipToBounds(Box_t* restrict p_box, const Box_t* restrict p_boundary)
{
    assert(p_box);
    assert(p_boundary);

    const int32_t left =
        Int32_GetMax(p_box->left, p_boundary->left);
    const int32_t right =
        Int32_GetMin(BoxRight(p_box), BoxRight(p_boundary));

    const int32_t top =
        Int32_GetMax(p_box->top, p_boundary->top);
    const int32_t bottom =
        Int32_GetMin(BoxBottom(p_box), BoxBottom(p_boundary));

    if (right <= left || bottom <= top)
    {
        printd("Could not clip to bounds, no intersection: %d, %d; %d, %d\n",
            left, right, top, bottom
        );
        return false;
    }

    p_box->left   = left;
    p_box->top    = top;
    p_box->width  = Int32_Subtract(right, left);
    p_box->height = Int32_Subtract(bottom, top);
    return true;
}

//------------------------------------------------------------------------------
// Box_RotateInPlace
//------------------------------------------------------------------------------
void Box_RotateInPlace(Box_t* p_box)
{
    assert(p_box);

    const int32_t width  = p_box->width;
    const int32_t height = p_box->height;
    p_box->width  = height;
    p_box->height = width;

    p_box->left += Int32_Divide(width - height, 2);
    p_box->top  += Int32_Divide(height - width, 2);
}

//------------------------------------------------------------------------------
// Box_Print
//------------------------------------------------------------------------------
void Box_Print(const char* restrict prefix, const Box_t* restrict p_box)
{
#ifdef NDEBUG

    (void)prefix;
    (void)p_box;

#else

    assert(prefix);
    assert(p_box);

    printd("%s = [ %d %d %d %d ]\n",
        prefix, p_box->left, p_box->top, p_box->width, p_box->height
    );

#endif
}

/* Global Callback Function Implementations ==================================*/
