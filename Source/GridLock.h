/*==============================================================================
File name   : GridLock.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-08-30
Version     : 0.1
Description : Applications Definitions Module
              This module defines variables regarding the application globally
              or otherwise common to various other modules of GridLock.

Copyright (c) 2026
==============================================================================*/
#ifndef GRIDLOCK_H
#define GRIDLOCK_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#ifndef NDEBUG
#define printd(...) printf(__VA_ARGS__)
#else
#define printd(...)
#endif

//------------------------------------------------------------------------------
typedef enum
{
    STATUS_SUCCESS       = 0,
    STATUS_FAILURE       = 1000,
    STATUS_NOENTRY       = ENOENT,
    STATUS_INVALIDFORMAT = ENOEXEC,
    STATUS_INVALIDHANDLE = EBADF,
    STATUS_MEMEXHAUSTED  = ENOMEM,
    STATUS_ACCESSDENIED  = EACCES,
    STATUS_INVALIDADDR   = EFAULT,
    STATUS_ENTRYEXISTS   = EEXIST,
    STATUS_INVALIDPARAM  = EINVAL,
    STATUS_NOTSUPPORTED  = ENOSYS,
    STATUS_TIMEDOUT      = ETIMEDOUT,
}
status_t;

//------------------------------------------------------------------------------
#define SHELL_EXECUTE_MAX_ERROR (32) // Used to check WinApi INT_PTR results //

//------------------------------------------------------------------------------
// Software Abort Conditions
//------------------------------------------------------------------------------
#define SacReturnIf(condition, returnValue) \
    do { if (condition) { return returnValue; } } while(0)

#define SacPrintReturnIf(condition, returnValue, format, ...) \
    do { \
        if (condition) { printf(format, ##__VA_ARGS__); return returnValue; } \
    } while(0)

#define SacReturnIfNot(condition, returnValue) \
    do { if (false == condition) { return returnValue; } } while(0)

#define SacReturnIfNull(condition, returnValue) \
    do { if (NULL == condition) { return returnValue; } } while(0)

#define SacReturnIfErr(condition, returnValue) \
    do { if (STATUS_SUCCESS != condition) { return returnValue; } } while(0)

#define SacPrintReturnIfNot(condition, returnValue, format, ...) \
    do { \
        if (false == condition) { \
            printf(format, ##__VA_ARGS__); return returnValue; \
        } \
    } while(0)

#define SacPrintReturnIfNull(condition, returnValue, format, ...) \
    do { \
        if (NULL == condition) { \
            printf(format, ##__VA_ARGS__); return returnValue; \
        } \
    } while(0)

#define SacPrintReturnIfErr(condition, returnValue, format, ...) \
    do { \
        if (STATUS_SUCCESS != condition) { \
            printf(format, ##__VA_ARGS__); return returnValue; \
        } \
    } while(0)

#define SacAbortIf(condition) \
    do { if (condition) { return; } } while(0)

#define SacPrintAbortIf(condition, format, ...) \
    do { if (condition) { printf(format, ##__VA_ARGS__); return; } } while(0)

#define SacAbortIfNot(condition) \
    do { if (false == condition) { return; } } while(0)

#define SacAbortIfNull(condition) \
    do { if (NULL == condition) { return; } } while(0)

#define SacAbortIfErr(condition) \
    do { if (STATUS_SUCCESS != condition) { return; } } while(0)

#define SacPrintAbortIfNot(condition, format, ...) \
    do { \
        if (false == condition) { printf(format, ##__VA_ARGS__); return; } \
    } while(0)

#define SacPrintAbortIfNull(condition, format, ...) \
    do { \
        if (NULL == condition) { printf(format, ##__VA_ARGS__); return; } \
    } while(0)

#define SacPrintAbortIfErr(condition, format, ...) \
    do { \
        if (STATUS_SUCCESS != condition) \
        { \
            printf(format, ##__VA_ARGS__); return; \
        } \
    } while(0)

#define SacAssert(condition) \
    assert(condition)

#define SacAssertNot(condition) \
    assert(false == condition)

#define SacAssertNotNull(condition) \
    assert(NULL != condition)


/* Public Global Variables ===================================================*/


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
status_t GridLock_Help(void);


/* Public Function Implementations ===========================================*/


#endif /* GRIDLOCK_H */
