/*==============================================================================
File name   : Defaults.c
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines the mapping defaults for a GridLock hotkey
              configuration.

Copyright (c) 2026
==============================================================================*/

/* Includes ==================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <assert.h>

#include "Defaults.h"

#include "Utility.h"
#include "Action.h"
#include "Display.h"


/* Local Structures/Macros ===================================================*/


/* Private Helper Function Prototypes ========================================*/


/* Private Member Variables ==================================================*/


/* Private Function Implementations ==========================================*/


/* Public Member Variables ===================================================*/
//------------------------------------------------------------------------------
const DefaultActionMapping_t DefaultActionMapping[] =
{
    {
        "Help", Action_Help, .defaultparam.asUnused = 0
    },
    {
        "Revert", Action_Revert, .defaultparam.asUnused = 0
    },
    {
        "Minimize", Action_Minimize, .defaultparam.asMinimize.toggle = true
    },
    {
        "Maximize", Action_Maximize, .defaultparam.asMaximize.toggle = true
    },
    {
        "Tile", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = VALUE_INT8_UNSET,
            .rows      = VALUE_INT8_UNSET,
            .index     = VALUE_INT8_UNSET,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "Move", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 2 }, .vertical = { 1, 2 }, .jump = true
        }
    },
    {
        "Jump", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_NEXT
        }
    },
    {
        "Increase", Action_Increase, .defaultparam.asIncrease =
        {
            .resize =
            {
                .type   = INPUTTYPE_SCALAR,
                .scalar = { .width = 60, .height = 60 }
            }
        }
    },
    {
        "Decrease", Action_Decrease, .defaultparam.asDecrease =
        {
            .resize =
            {
                .type   = INPUTTYPE_SCALAR,
                .scalar = { .width = 60, .height = 60 }
            },
            .edgelock = true,
            .restore  = false
        }
    },
    {
        "Project", Action_Project, .defaultparam.asProject =
        {
            .mode = PROJECT_DISPLAY_UNKNOWN
        }
    },
    // Named sub actions //
    {
        "Fill", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 1,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "MediumFill", Action_Tile, .defaultparam.asTile =
        {
            .inlay =
            {
                .type = INPUTTYPE_DISPLAY,
                .dualratio.width  = { 8, 10 },
                .dualratio.height = { 95, 100 }
            },
            .columns   = 1,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "AlmostFill", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 1,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "LeftSidebar", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 2,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "LeftSidebarWide", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 3,
            .rows      = 1,
            .index     = 1,
            .width     = 2,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "RightSidebar", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 2,
            .rows      = 1,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "RightSidebarWide", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 3,
            .rows      = 1,
            .index     = 2,
            .width     = 2,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "CenterPane", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 4,
            .rows      = 1,
            .index     = 2,
            .width     = 2,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "CenterPaneWide", Action_Tile, .defaultparam.asTile =
        {
            .inlay = { .type  = INPUTTYPE_DISPLAY_W, .ratio = { 975, 1000 } },
            .columns   = 6,
            .rows      = 1,
            .index     = 2,
            .width     = 4,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    // Halves //
    {
        "LeftHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "RightHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 1,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "TopHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 1,
            .rows      = 2,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "BottomHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 1,
            .rows      = 2,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "MiddleHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay =
            {
                .type = INPUTTYPE_DISPLAY,
                .dualratio.width  = { 1, 2 },
                .dualratio.height = { 1, 1 },
            },
            .columns   = 1,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "LeftOrTopHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    {
        "RightOrBottomHalf", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 1,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    // Thirds //
    {
        "LeftThird", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "MiddleThird", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "RightThird", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 3,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "LeftTwoThirds", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 1,
            .width     = 2,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "RightTwoThirds", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 2,
            .width     = 2,
            .height    = 1,
            .transpose = false,
            .jump      = true
        }
    },
    {
        "LeftOrTopThird", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    {
        "MiddleTallOrMiddleWideThird", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    {
        "RightOrBottomThird", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 3,
            .width     = 1,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    {
        "LeftOrTopTwoThirds", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 1,
            .width     = 2,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    {
        "RightOrBottomTwoThirds", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 1,
            .index     = 2,
            .width     = 2,
            .height    = 1,
            .transpose = true,
            .jump      = false
        }
    },
    // Corners
    {
        "TopLeftCorner", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 2,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "TopRightCorner", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 2,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "BottomLeftCorner", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 2,
            .index     = 3,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "BottomRightCorner", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 2,
            .rows      = 2,
            .index     = 4,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    // Sixths //
    {
        "TopLeftSixth", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 2,
            .index     = 1,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "TopMiddleSixth", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 2,
            .index     = 2,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "TopRightSixth", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 2,
            .index     = 3,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "BottomLeftSixth", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 2,
            .index     = 4,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "BottomMiddleSixth", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 2,
            .index     = 5,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "BottomRightSixth", Action_Tile, .defaultparam.asTile =
        {
            .inlay     = { .type = INPUTTYPE_NONE },
            .columns   = 3,
            .rows      = 2,
            .index     = 6,
            .width     = 1,
            .height    = 1,
            .transpose = false,
            .jump      = false
        }
    },
    {
        "Center", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 2 }, .vertical = { 1, 2 }, .jump = false
        }
    },
    {
        "MoveCenter", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 2 }, .vertical = { 1, 2 }, .jump = false
        }
    },
    {
        "MoveLeft", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 0, 1 }, .vertical = { 1, 2 }, .jump = true
        }
    },
    {
        "MoveRight", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 1 }, .vertical = { 1, 2 }, .jump = true
        }
    },
    {
        "MoveUp", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 2 }, .vertical = { 0, 1 }, .jump = true
        }
    },
    {
        "MoveDown", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 2 }, .vertical = { 1, 1 }, .jump = true
        }
    },
    {
        "MoveTopLeft", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 0, 1 }, .vertical = { 0, 1 }, .jump = false
        }
    },
    {
        "MoveTopRight", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 1 }, .vertical = { 0, 1 }, .jump = false
        }
    },
    {
        "MoveBottomLeft", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 0, 1 }, .vertical = { 1, 1 }, .jump = false
        }
    },
    {
        "MoveBottomRight", Action_Move, .defaultparam.asMove =
        {
            .horizontal = { 1, 1 }, .vertical = { 1, 1 }, .jump = false
        }
    },
    {
        "JumpNext", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_NEXT
        }
    },
    {
        "JumpPrevious", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_PREV
        }
    },
    {
        "JumpLeft", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_LEFT
        }
    },
    {
        "JumpRight", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_RIGHT
        }
    },
    {
        "JumpUp", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_UP
        }
    },
    {
        "JumpDown", Action_Jump, .defaultparam.asJump =
        {
            .direction = JUMP_DISPLAY_DOWN
        }
    },
    {
        "ProjectDisplay", Action_Project, .defaultparam.asProject =
        {
            .mode = PROJECT_DISPLAY_UNKNOWN
        }
    },
    {
        "ProjectPrimary", Action_Project, .defaultparam.asProject =
        {
            .mode = PROJECT_DISPLAY_PRIMARY
        }
    },
    {
        "ProjectSecondary", Action_Project, .defaultparam.asProject =
        {
            .mode = PROJECT_DISPLAY_SECONDARY
        }
    },
    {
        "ProjectExtend", Action_Project, .defaultparam.asProject =
        {
            .mode = PROJECT_DISPLAY_EXTEND
        }
    },
    {
        "ProjectDuplicate", Action_Project, .defaultparam.asProject =
        {
            .mode = PROJECT_DISPLAY_DUPLICATE
        }
    },
};
#define DEFAULT_ACTION_MAPPING_NAME_MAX_WIDTH (28)
#define DEFAULT_ACTION_MAPPING_LENGTH \
    (sizeof(DefaultActionMapping) / sizeof(DefaultActionMapping_t))
static_assert(DEFAULT_ACTION_MAPPING_LENGTH <= UINT8_MAX);


/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Defaults_GetActionIndexByName
//------------------------------------------------------------------------------
bool Defaults_GetActionIndexByName(const char* p_name, uint8_t* p_index)
{
    assert(p_name);
    assert(p_index);

    const size_t capacity = DEFAULT_ACTION_MAPPING_NAME_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < DEFAULT_ACTION_MAPPING_LENGTH; i++)
    {
        const DefaultActionMapping_t* p_mapping = &(DefaultActionMapping[i]);
        if (Utility_StringEquals(p_mapping->name, p_name, capacity))
        {
            *p_index = i;
            return true;
        }
    }

    return false;
}

//------------------------------------------------------------------------------
// Defaults_GetActionNameByFunction
//------------------------------------------------------------------------------
const char* Defaults_GetActionNameByFunction(p_Action_t action)
{
    if (NULL == action)
    {
        return "No window action";
    }

    // Note that we will return the earliest element which matches the action //
    // This is intended to be the base window action //
    for (uint_fast8_t i = 0; i < DEFAULT_ACTION_MAPPING_LENGTH; i++)
    {
        const DefaultActionMapping_t* p_mapping = &(DefaultActionMapping[i]);
        if (p_mapping->action == action)
        {
            return p_mapping->name;
        }
    }

    if (Action_Sequence == action)
    {
        return "Sequence";
    }

    return "Invalid window action";
}

