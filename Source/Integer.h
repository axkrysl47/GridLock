/*==============================================================================
File name   : Integer.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines integer utility functions for the GridLock
              Window Organization Utility.

Copyright (c) 2026
==============================================================================*/
#ifndef INTEGER_H
#define INTEGER_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <assert.h>

#include "GridLock.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define VALUE_INT8_UNSET  (INT8_MIN)
#define VALUE_INT32_UNSET (INT32_MIN)

//------------------------------------------------------------------------------
typedef struct
{
    int32_t width;
    int32_t height;
}
dimension32_t;

typedef struct
{
    int32_t numerator;
    int32_t denominator;
}
ratio32_t;


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
static inline int32_t Int32_Add(int32_t addend0, int32_t addend1);
static inline int32_t Int32_Subtract(int32_t minuend, int32_t subtrahend);
static inline int32_t Int32_Multiply(int32_t multiplicand0,
    int32_t multiplicand1
);
static inline int32_t Int32_Divide(int32_t dividend, int32_t divisor);

//------------------------------------------------------------------------------
static inline int32_t Int32_GetMin(int32_t input0, int32_t input1);
static inline int32_t Int32_GetMax(int32_t input0, int32_t input1);
static inline int32_t Int32_ClampMin(int32_t input, int32_t minimum);
static inline int32_t Int32_ClampMax(int32_t input, int32_t maximum);
static inline int64_t Int64_ClampMin(int64_t input, int64_t minimum);
static inline int64_t Int64_ClampMax(int64_t input, int64_t maximum);
static inline int64_t Int64_Clamp(int64_t input, int64_t min, int64_t max);


/* Public Global Variables ===================================================*/


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Int32_Add
//------------------------------------------------------------------------------
static inline int32_t Int32_Add(int32_t addend0, int32_t addend1)
{
    return Int64_Clamp(
        (int64_t)addend0 + (int64_t)addend1, INT32_MIN, INT32_MAX
    );
}

//------------------------------------------------------------------------------
// Int32_Subtract
//------------------------------------------------------------------------------
static inline int32_t Int32_Subtract(int32_t minuend, int32_t subtrahend)
{
    return Int64_Clamp(
        (int64_t)minuend - (int64_t)subtrahend, INT32_MIN, INT32_MAX
    );
}

//------------------------------------------------------------------------------
// Int32_Multiply
//------------------------------------------------------------------------------
static inline int32_t Int32_Multiply(int32_t multiplicand0,
    int32_t multiplicand1)
{
    return Int64_Clamp(
        (int64_t)multiplicand0 * (int64_t)multiplicand1, INT32_MIN, INT32_MAX
    );
}

//------------------------------------------------------------------------------
// Int32_Divide
//------------------------------------------------------------------------------
static inline int32_t Int32_Divide(int32_t dividend, int32_t divisor)
{
    assert(0 < divisor);

    return ((dividend < 0))
        ? (int32_t)(((int64_t)dividend - (divisor / 2)) / divisor)
        : (int32_t)(((int64_t)dividend + (divisor / 2)) / divisor);
}

//------------------------------------------------------------------------------
// Int32_GetMin
//------------------------------------------------------------------------------
static inline int32_t Int32_GetMin(int32_t input0, int32_t input1)
{
    return (input0 < input1) ? input0 : input1;
}

//------------------------------------------------------------------------------
// Int32_GetMax
//------------------------------------------------------------------------------
static inline int32_t Int32_GetMax(int32_t input0, int32_t input1)
{
    return (input0 < input1) ? input1 : input0;
}

//------------------------------------------------------------------------------
// Int32_ClampMin
//------------------------------------------------------------------------------
static inline int32_t Int32_ClampMin(int32_t input, int32_t minimum)
{
    return (input < minimum) ? minimum : input;
}

//------------------------------------------------------------------------------
// Int32_ClampMax
//------------------------------------------------------------------------------
static inline int32_t Int32_ClampMax(int32_t input, int32_t maximum)
{
    return (maximum < input) ? maximum : input;
}

//------------------------------------------------------------------------------
// Int64_ClampMin
//------------------------------------------------------------------------------
static inline int64_t Int64_ClampMin(int64_t input, int64_t minimum)
{
    return (input < minimum) ? minimum : input;
}

//------------------------------------------------------------------------------
// Int64_ClampMax
//------------------------------------------------------------------------------
static inline int64_t Int64_ClampMax(int64_t input, int64_t maximum)
{
    return (maximum < input) ? maximum : input;
}

//------------------------------------------------------------------------------
// Int64_Clamp
//------------------------------------------------------------------------------
static inline int64_t Int64_Clamp(int64_t input, int64_t min, int64_t max)
{
    assert(min < max);
    return Int64_ClampMax(Int64_ClampMin(input, min), max);
}


#endif /* INTEGER_H */
