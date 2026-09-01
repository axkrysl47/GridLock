/*==============================================================================
File name   : Utility.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines string utility functions for the GridLock
              Window Organization Utility.

Copyright (c) 2026
==============================================================================*/
#ifndef UTILITY_H
#define UTILITY_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "GridLock.h"
#include "Integer.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
typedef struct
{
    uint8_t major;
    uint8_t minor;
    uint8_t revision;
}
version_t;
#define VERSION_COUNT (3)


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
static inline void Utility_StringCopy(char* restrict destination,
    const char* restrict source, size_t size
);
static inline bool Utility_StringEquals(const char* restrict input0,
    const char* restrict input1, size_t size
);

//------------------------------------------------------------------------------
static inline bool Utility_ParseBool(const char* restrict string,
    int8_t* restrict p_int8
);
static inline bool Utility_ParseInt8(const char* restrict string,
    int8_t* restrict p_int8
);
static inline bool Utility_ParseInt32(const char* restrict string,
    int32_t* restrict p_int32
);
static inline bool Utility_ParseRatioString(const char* restrict string,
    ratio32_t* restrict p_ratio
);
static inline bool Utility_ParseVersionString(const char* restrict string,
    version_t* restrict p_version
);


/* Public Global Variables ===================================================*/


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Utility_StringCopy
//------------------------------------------------------------------------------
static inline void Utility_StringCopy(char* restrict destination,
    const char* restrict source, size_t size)
{
    assert(destination);
    assert(source);

    if (0 == size)
    {
        return;
    }

    const size_t length = size - 1;
    for (size_t i = 0; i < length; i++)
    {
        const char c = source[i];
        destination[i] = c;
        if ('\0' == c)
        {
            return;
        }
    }

    destination[length] = '\0';
}

//------------------------------------------------------------------------------
// Utility_StringEquals
//------------------------------------------------------------------------------
static inline bool Utility_StringEquals(const char* restrict input0,
    const char* restrict input1, size_t capacity)
{
    assert(input0);
    assert(input1);
    assert(0 < capacity);

    // Besides letters and numbers, only '-', '_', and ' ' are allowed //
    // Further, this string equality function is case insensitive //
    const size_t length = capacity - 1;
    for (size_t i = 0; i < length; i++)
    {
        char c0 = input0[i];
        char c1 = input1[i];

        if ('\0' == c0 && '\0' == c1) { return true;  }
        if ('\0' == c0 || '\0' == c1) { return false; }

        if ('-' == c0  &&  '-' == c1) { continue;     }
        if ('-' == c0  ||  '-' == c1) { return false; }

        if ('_' == c0  &&  '_' == c1) { continue;     }
        if ('_' == c0  ||  '_' == c1) { return false; }

        if (' ' == c0  &&  ' ' == c1) { continue;     }
        if (' ' == c0  ||  ' ' == c1) { return false; }

        if ('0' <= c0 && c0 <= '9')
        {
            if (c0 != c1)
            {
                return false;
            }
            continue;
        }

        if ('A' <= c0 && c0 <= 'Z')
        {
            c0 += (char)('a' - 'A');
        }
        if (c0 < 'a' || 'z' < c0)
        {
            return false;
        }

        if ('A' <= c1 && c1 <= 'Z')
        {
            c1 += (char)('a' - 'A');
        }
        if (c1 < 'a' || 'z' < c1)
        {
            return false;
        }

        if (c0 != c1)
        {
            return false;
        }
    }
    return ('\0' == input0[length] && '\0' == input1[length]);
}

//------------------------------------------------------------------------------
// Utility_ParseBool
//------------------------------------------------------------------------------
static inline bool Utility_ParseBool(const char* restrict string,
    int8_t* restrict p_int8)
{
    assert(string);
    assert(p_int8);

    if (Utility_StringEquals("enabled", string, sizeof("enabled"))
        || Utility_StringEquals("true", string, sizeof("true"))
        || Utility_StringEquals("t", string, sizeof("t"))
        || Utility_StringEquals("yes", string, sizeof("yes"))
        || Utility_StringEquals("y", string, sizeof("y"))
        || Utility_StringEquals("on", string, sizeof("on"))
        || Utility_StringEquals("1", string, sizeof("1")))
    {
        *p_int8 = true;
        return true;
    }
    if (Utility_StringEquals("disabled", string, sizeof("disabled"))
        || Utility_StringEquals("false", string, sizeof("false"))
        || Utility_StringEquals("f", string, sizeof("f"))
        || Utility_StringEquals("no", string, sizeof("no"))
        || Utility_StringEquals("n", string, sizeof("n"))
        || Utility_StringEquals("off", string, sizeof("off"))
        || Utility_StringEquals("0", string, sizeof("0")))
    {
        *p_int8 = false;
        return true;
    }

    return false;
}

//------------------------------------------------------------------------------
// Utility_ParseInt8
//------------------------------------------------------------------------------
static inline bool Utility_ParseInt8(const char* restrict string,
    int8_t* restrict p_int8)
{
    assert(string);
    assert(p_int8);

    const char* p_string = string;
    while (' ' == *p_string)
    {
        p_string++;
    }

    int16_t sign = 1;
    if ('+' == *p_string)
    {
        p_string++;
    }
    else if ('-' == *p_string)
    {
        sign = -1;
        p_string++;
    }

    int16_t value = 0;
    const int_fast8_t i_max = 4;
    int_fast8_t i; for (i = 0; i < i_max; i++)
    {
        const char c = *p_string;
        if (' ' == c || '\0' == c)
        {
            break;
        }
        else if ('0' == c)
        {
            // Incremented p_string is never written //
            const char next = *(p_string + 1);
            SacPrintReturnIf((0 == i && '0' <= next && next <= '9'), false,
                "Cannot start integer with a 0\n"
            );
            value *= 10; value += 0;
        }
        else if ('1' <= c && c <= '9')
        {
            value *= 10; value += (c - '0');
        }
        else
        {
            printd("Invalid character found in integer\n");
            return false;
        }

        p_string++;
    }
    SacPrintReturnIf(0 == i, false, "No integer was provided.\n");

    SacPrintReturnIf((i_max == i), false, "Exceeded max digits\n");
    value *= sign;
    SacPrintReturnIf(
        (value < INT8_MIN), false, "Exceeded min int8_t value\n"
    );
    SacPrintReturnIf(
        (INT8_MAX < value), false, "Exceeded max int8_t value\n"
    );

    SacPrintReturnIf((VALUE_INT8_UNSET == value), false,
        "Sentinel value was parsed, rejecting\n"
    );

    while ('\0' != *p_string)
    {
        SacPrintReturnIf((' ' != *p_string), false,
            "Unexpected characters in integer\n"
        );
        p_string++;
    }

    *p_int8 = (int8_t)value;
    return true;
}

//------------------------------------------------------------------------------
// Utility_ParseInt32
//------------------------------------------------------------------------------
static inline bool Utility_ParseInt32(const char* restrict string,
    int32_t* restrict p_int32)
{
    assert(string);
    assert(p_int32);

    const char* p_string = string;
    while (' ' == *p_string)
    {
        p_string++;
    }

    int64_t sign = 1;
    if ('+' == *p_string)
    {
        p_string++;
    }
    else if ('-' == *p_string)
    {
        sign = -1;
        p_string++;
    }

    int64_t value = 0;
    const int_fast8_t i_max = 11;
    int_fast8_t i; for (i = 0; i < i_max; i++)
    {
        const char c = *p_string;
        if (' ' == c || '\0' == c)
        {
            break;
        }
        else if ('0' == c)
        {
            // Incremented p_string is never written //
            const char next = *(p_string + 1);
            SacPrintReturnIf((0 == i && '0' <= next && next <= '9'), false,
                "Cannot start integer with a 0\n"
            );
            value *= 10; value += 0;
        }
        else if ('1' <= c && c <= '9')
        {
            value *= 10; value += (c - '0');
        }
        else
        {
            printd("Invalid character found in integer\n");
            return false;
        }

        p_string++;
    }
    SacPrintReturnIf(0 == i, false, "No integer was provided.\n");
    SacPrintReturnIf((i_max == i), false, "Exceeded max digits");
    value *= sign;
    SacPrintReturnIf(
        (value < INT32_MIN), false, "Exceeded min int32_t value\n"
    );
    SacPrintReturnIf(
        (INT32_MAX < value), false, "Exceeded max int32_t value\n"
    );

    while ('\0' != *p_string)
    {
        SacPrintReturnIf((' ' != *p_string), false,
            "Unexpected characters in integer\n"
        );
        p_string++;
    }

    SacPrintReturnIf((VALUE_INT32_UNSET == value), false,
        "Sentinel value was parsed, rejecting\n"
    );

    *p_int32 = (int32_t)value;
    return true;
}

//------------------------------------------------------------------------------
// Utility_ParseRatioString
//------------------------------------------------------------------------------
static inline bool Utility_ParseRatioString(const char* restrict string,
    ratio32_t* restrict p_ratio)
{
    assert(string);
    assert(p_ratio);

    const char* p_string = string;
    while (' ' == *p_string)
    {
        p_string++;
    }

    int64_t sign = 1;
    if ('+' == *p_string)
    {
        p_string++;
    }
    else if ('-' == *p_string)
    {
        sign = -1;
        p_string++;
    }

    bool dividerFound = false;
    int64_t numerator = 0;
    const int_fast8_t i_max = 11;
    int_fast8_t i; for (i = 0; i < i_max; i++)
    {
        const char c = *p_string;
        if (':' == c || '/' == c)
        {
            p_string++;
            dividerFound = true;
            break;
        }
        else if (' ' == c)
        {
            break;
        }
        else if ('0' == c)
        {
            // Incremented p_string is never written //
            const char next = *(p_string + 1);
            SacPrintReturnIf((0 == i && '0' <= next && next <= '9'), false,
                "Cannot start integer with a 0\n"
            );
            numerator *= 10; numerator += 0;
        }
        else if ('1' <= c && c <= '9')
        {
            numerator *= 10; numerator += (c - '0');
        }
        else if ('\0' == c)
        {
            printd("Missing denominator in ratio\n");
            return false;
        }
        else
        {
            printd("Invalid character found in numerator\n");
            return false;
        }

        p_string++;
    }
    SacPrintReturnIf((0 == i), false, "Numerator was not provided\n");
    SacPrintReturnIf((i_max == i), false, "Numerator exceeded max digits\n");
    numerator *= sign;
    SacPrintReturnIf(
        (INT32_MAX < numerator), false, "Numerator exceeded max int32_t value\n"
    );
    SacPrintReturnIf(
        (numerator < INT32_MIN), false, "Numerator exceeded min int32_t value\n"
    );
    SacPrintReturnIf((VALUE_INT32_UNSET == numerator), false,
        "Sentinel value was parsed, rejecting\n"
    );

    while (false == dividerFound)
    {
        const char c = *p_string;

        SacPrintReturnIf(('\0' == c), false, "Missing divider in ratio\n");
        if (c == ' ')             { p_string++; continue; }
        if (c == ':' || c == '/') { p_string++; dividerFound = true; break; }

        printd("Invalid character found after numerator\n");
        return false;
    }

    while (' ' == *p_string)
    {
        p_string++;
    }

    int64_t denominator = 0;
    for (i = 0; i < i_max; i++)
    {
        const char c = *p_string;
        if (' ' == c || '\0' == c)
        {
            break;
        }
        else if ('0' == c)
        {
            const char next = *(p_string + 1);
            SacPrintReturnIf((0 == i && '0' <= next && next <= '9'), false,
                "Cannot start integer with a 0\n"
            );
            denominator *= 10; denominator += 0;
        }
        else if ('1' <= c && c <= '9')
        {
            denominator *= 10; denominator += (c - '0');
        }
        else
        {
            printd("Invalid character found in denominator\n");
            return false;
        }

        p_string++;
    }
    while ('\0' != *p_string)
    {
        SacPrintReturnIf((' ' != *p_string), false,
            "Unexpected characters in integer\n"
        );
        p_string++;
    }

    SacPrintReturnIf((i_max == i), false, "Denominator exceeded max digits\n");
    SacPrintReturnIf((0 == denominator), false, "Denominator was zero\n");
    SacPrintReturnIf((INT32_MAX < denominator), false,
        "Denominator exceeded max int32_t value\n"
    );
    SacPrintReturnIf((denominator < INT32_MIN), false,
        "Denominator exceeded min int32_t value\n"
    );
    SacPrintReturnIf((VALUE_INT32_UNSET == denominator), false,
        "Sentinel value was parsed, rejecting\n"
    );

    p_ratio->numerator = numerator;
    p_ratio->denominator = denominator;
    return true;
}

//------------------------------------------------------------------------------
// Utility_ParseVersionString
//------------------------------------------------------------------------------
static inline bool Utility_ParseVersionString(const char* restrict string,
    version_t* restrict p_version)
{
    assert(string);
    assert(p_version);

    // Note that this function expects at least a "#.#.#" string //

    const char* p_string = string;
    while (' ' == *p_string)
    {
        p_string++;
    }

    if ('v' == *p_string)
    {
        p_string++;
    }

    const int_fast8_t i_max = 4;
    uint16_t version[VERSION_COUNT];
    for (int_fast8_t index = 0; index < VERSION_COUNT; index++)
    {
        version[index] = 0u;
        int_fast8_t i; for (i = 0; i < i_max; i++)
        {
            const char c = *p_string;

            if ('\0' == c)
            {
                break;
            }

            if (index < VERSION_COUNT - 1 && '.' == c)
            {
                p_string++;
                break;
            }
            else if ('0' == c)
            {
                // Incremented p_string is never written //
                const char next = *(p_string + 1);
                SacPrintReturnIf((0 == i && '0' <= next && next <= '9'), false,
                    "Cannot start version integer with a 0\n"
                );
                version[index] *= 10; version[index] += 0;
            }
            else if ('1' <= c && c <= '9')
            {
                version[index] *= 10; version[index] += (c - '0');
            }
            else
            {
                printd("Invalid character found in version\n");
                return false;
            }

            p_string++;
        }
        SacPrintReturnIf(0 == i, false, "No version integer was provided.\n");
        SacPrintReturnIf((i_max == i), false, "Exceeded max digits\n");
        SacPrintReturnIf(
            (UINT8_MAX < version[index]), false,
            "Exceeded max uint8_t version\n"
        );
    }

    p_version->major = (uint8_t)version[0];
    p_version->minor = (uint8_t)version[1];
    p_version->revision = (uint8_t)version[2];
    return true;
}


#endif /* UTILITY_H */
