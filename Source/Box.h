/*==============================================================================
File name   : Box.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the Box_t struct which is used to define
              window positions and sizes within GridLock.

Copyright (c) 2026
==============================================================================*/
#ifndef BOX_H
#define BOX_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>

#include <windows.h>


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef struct
{
    int32_t left;
    int32_t top;
    int32_t width;
    int32_t height;
}
Box_t; // not member compatible with RECT! //

typedef struct
{
    // true if the box is on or outside the edge of the given boundary //
    bool left;
    bool right;
    bool top;
    bool bottom;
}
BoxEdgeState_t;


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
bool Box_RectToBox(const RECT *p_rect, Box_t* p_box);
RECT Box_RectFromBox(const Box_t* p_box);
Box_t Box_Copy(const Box_t* p_box);

//------------------------------------------------------------------------------
bool Box_IsEqual(const Box_t* p_first, const Box_t* p_second);
BoxEdgeState_t Box_GetEdgeState(const Box_t* p_box, const Box_t* p_boundary);

//------------------------------------------------------------------------------
void Box_SymmetricIncrement(Box_t* p_box);
void Box_SymmetricDecrement(Box_t* p_box);
void Box_Add(Box_t* restrict p_sum, const Box_t* restrict p_addend1,
    const Box_t* restrict p_addend2
);
void Box_Subtract(Box_t* restrict p_difference, const Box_t* restrict p_minuend,
    const Box_t* restrict p_subtrahend
);

//------------------------------------------------------------------------------
void Box_ShiftInBounds(Box_t* restrict p_box, const Box_t* restrict p_boundary);
bool Box_ClipToBounds(Box_t* restrict p_box, const Box_t* restrict p_boundary);
void Box_RotateInPlace(Box_t* p_box);

//------------------------------------------------------------------------------
void Box_Print(const char* restrict prefix, const Box_t* restrict p_box);


/* Public Function Implementations ===========================================*/


#endif /* BOX_H */
