/*==============================================================================
File name   : Mapping.h
Author      : Alexander Krysl (axkrysl47)
Date        : 2026-09-28
Version     : 1.0.0
Description : This module defines Keymap string name mappings.

Copyright (c) 2026
==============================================================================*/
#ifndef MAPPING_H
#define MAPPING_H

/* Includes ==================================================================*/
#include <stdint.h>
#include <assert.h>

#include "GridLock.h"

#include "Utility.h"
#include "Window.h"
#include "Display.h"
#include "Action.h"


/* Local Structures/Macros ===================================================*/
//------------------------------------------------------------------------------
#define DepthFromKeymap(keymap) \
    ((int16_t)(sizeof(keymap) / sizeof((keymap)[0])))

#define SEQUENCE_KEYMAP_OFFSET (2)

//------------------------------------------------------------------------------
typedef struct
{
    const char* name;
    BorderVisibility_t borderVisibility;
}
BorderVisibilityMapping_t;

//------------------------------------------------------------------------------
typedef struct
{
    const char* name;
    unsigned int keycode;
}
KeybindMapping_t;

//------------------------------------------------------------------------------
typedef struct
{
    const char* name;
    InputType_t type;
}
InputTypeMapping_t;

//------------------------------------------------------------------------------
typedef struct
{
    const char* name;
    JumpDisplay_t direction;
}
JumpDisplayMapping_t;

//------------------------------------------------------------------------------
typedef struct
{
    const char* name;
    ProjectDisplay_t mode;
}
ProjectDisplayMapping_t;


/* Public Function Prototypes ================================================*/
//------------------------------------------------------------------------------
static inline const BorderVisibilityMapping_t* Mapping_BorderVisibility(
    const char* string
);
static inline const KeybindMapping_t* Mapping_Keybind(
    const char* string
);
static inline const InputTypeMapping_t* Mapping_InputType(
    const char* string
);
static inline const JumpDisplayMapping_t* Mapping_JumpDisplay(
    const char* string
);
static inline const ProjectDisplayMapping_t* Mapping_ProjectDisplay(
    const char* string
);


/* Public Global Variables ===================================================*/
//----------------------------------------------------------------------------------------------------------------------
static const char*  VersionKeymap[]               = { "version" };
static const int8_t VersionKeymapDepth            = DepthFromKeymap(VersionKeymap);
static const char*  BorderKeymap[]                = { "borders" };
static const int8_t BorderKeymapDepth             = DepthFromKeymap(BorderKeymap);
static const char*  MinimumKeymap[]               = { "minimum" };
static const int8_t MinimumKeymapDepth            = DepthFromKeymap(MinimumKeymap);
//----------------------------------------------------------------------------------------------------------------------
static const char*  HotkeyKeymap[]                = { "hotkeys", "-" };
static const int8_t HotkeyKeymapDepth             = DepthFromKeymap(HotkeyKeymap);
static const char*  ModifiersKeymap[]             = { "hotkeys", "-", "modifiers", "-" };
static const int8_t ModifiersKeymapDepth          = DepthFromKeymap(ModifiersKeymap);
static const char*  KeybindKeymap[]               = { "hotkeys", "-", "key" };
static const int8_t KeybindKeymapDepth            = DepthFromKeymap(KeybindKeymap);
static const char*  ActionKeymap[]                = { "hotkeys", "-", "action" };
static const int8_t ActionKeymapDepth             = DepthFromKeymap(ActionKeymap);
static const char*  EnableKeymap[]                = { "hotkeys", "-", "enable" };
static const int8_t EnableKeymapDepth             = DepthFromKeymap(EnableKeymap);
static const char*  DisableKeymap[]               = { "hotkeys", "-", "disable" };
static const int8_t DisableKeymapDepth            = DepthFromKeymap(DisableKeymap);
//----------------------------------------------------------------------------------------------------------------------
static const char*  ParametersKeymap[]            = { "hotkeys", "-", "parameters" };
static const int8_t ParametersKeymapDepth         = DepthFromKeymap(ParametersKeymap);
static const char*  RatioKeymap[]                 = { "hotkeys", "-", "parameters", "ratio" };
static const int8_t RatioKeymapDepth              = DepthFromKeymap(RatioKeymap);
static const char*  InlayKeymap[]                 = { "hotkeys", "-", "parameters", "inlay" };
static const int8_t InlayKeymapDepth              = DepthFromKeymap(InlayKeymap);
static const char*  InlayRatioKeymap[]            = { "hotkeys", "-", "parameters", "inlay", "ratio" };
static const int8_t InlayRatioKeymapDepth         = DepthFromKeymap(InlayRatioKeymap);
static const char*  GridKeymap[]                  = { "hotkeys", "-", "parameters", "grid", "-" };
static const int8_t GridKeymapDepth               = DepthFromKeymap(GridKeymap);
static const char*  SizeKeymap[]                  = { "hotkeys", "-", "parameters", "size", "-" };
static const int8_t SizeKeymapDepth               = DepthFromKeymap(SizeKeymap);
static const char*  ScalarKeymap[]                = { "hotkeys", "-", "parameters", "scalar", "-" };
static const int8_t ScalarKeymapDepth             = DepthFromKeymap(ScalarKeymap);
//----------------------------------------------------------------------------------------------------------------------
static const char*  SequenceKeymap[]              = { "hotkeys", "-", "sequence", "-" };
static const int8_t SequenceKeymapDepth           = DepthFromKeymap(SequenceKeymap);
static const char*  SequenceActionKeymap[]        = { "hotkeys", "-", "sequence", "-", "action" };
static const int8_t SequenceActionKeymapDepth     = DepthFromKeymap(SequenceActionKeymap);
//----------------------------------------------------------------------------------------------------------------------
static const char*  SequenceParametersKeymap[]    = { "hotkeys", "-", "sequence", "-", "parameters" };
static const int8_t SequenceParametersKeymapDepth = DepthFromKeymap(SequenceParametersKeymap);
static const char*  SequenceRatioKeymap[]         = { "hotkeys", "-", "sequence", "-", "parameters", "ratio" };
static const int8_t SequenceRatioKeymapDepth      = DepthFromKeymap(SequenceRatioKeymap);
static const char*  SequenceInlayKeymap[]         = { "hotkeys", "-", "sequence", "-", "parameters", "inlay" };
static const int8_t SequenceInlayKeymapDepth      = DepthFromKeymap(SequenceInlayKeymap);
static const char*  SequenceInlayRatioKeymap[]    = { "hotkeys", "-", "sequence", "-", "parameters", "inlay", "ratio" };
static const int8_t SequenceInlayRatioKeymapDepth = DepthFromKeymap(SequenceInlayRatioKeymap);
static const char*  SequenceGridKeymap[]          = { "hotkeys", "-", "sequence", "-", "parameters", "grid", "-" };
static const int8_t SequenceGridKeymapDepth       = DepthFromKeymap(SequenceGridKeymap);
static const char*  SequenceSizeKeymap[]          = { "hotkeys", "-", "sequence", "-", "parameters", "size", "-" };
static const int8_t SequenceSizeKeymapDepth       = DepthFromKeymap(SequenceSizeKeymap);
static const char*  SequenceScalarKeymap[]        = { "hotkeys", "-", "sequence", "-", "parameters", "scalar", "-" };
static const int8_t SequenceScalarKeymapDepth     = DepthFromKeymap(SequenceScalarKeymap);
//----------------------------------------------------------------------------------------------------------------------
static_assert(
    DepthFromKeymap(SequenceParametersKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(ParametersKeymap))
);
static_assert(
    DepthFromKeymap(SequenceRatioKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(RatioKeymap))
);
static_assert(
    DepthFromKeymap(SequenceInlayKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(InlayKeymap))
);
static_assert(
    DepthFromKeymap(SequenceInlayRatioKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(InlayRatioKeymap))
);
static_assert(
    DepthFromKeymap(SequenceGridKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(GridKeymap))
);
static_assert(
    DepthFromKeymap(SequenceSizeKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(SizeKeymap))
);
static_assert(
    DepthFromKeymap(SequenceScalarKeymap)
        == (SEQUENCE_KEYMAP_OFFSET + DepthFromKeymap(ScalarKeymap))
);

//------------------------------------------------------------------------------
static const BorderVisibilityMapping_t BorderVisibilityMapping[] =
{
    { "invisible" , BORDER_INVISIBLE }, // 0
    { "visible"   , BORDER_VISIBLE   }, // 1
    { "full"      , BORDER_FULL      }, // 2
};
#define BORDER_VISIBILITY_MAPPING_MAX_WIDTH (10)
#define BORDER_VISIBILITY_LENGTH \
    (sizeof(BorderVisibilityMapping) / sizeof(BorderVisibilityMapping_t))

//------------------------------------------------------------------------------
static const KeybindMapping_t KeybindMapping[] =
{
    { "Backspace"     , VK_BACK       }, // 0x08
    { "Tab"           , VK_TAB        }, // 0x09
    { "Enter"         , VK_RETURN     }, // 0x0D
    { "Return"        , VK_RETURN     }, // 0x0D
    { "Escape"        , VK_ESCAPE     }, // 0x1B
    { "Esc"           , VK_ESCAPE     }, // 0x1B
    { "Space"         , VK_SPACE      }, // 0x20
    { "PageUp"        , VK_PRIOR      }, // 0x21
    { "PageDown"      , VK_NEXT       }, // 0x22
    { "End"           , VK_END        }, // 0x23
    { "Home"          , VK_HOME       }, // 0x24
    { "Left"          , VK_LEFT       }, // 0x25
    { "LeftArrow"     , VK_LEFT       }, // 0x25
    { "Up"            , VK_UP         }, // 0x26
    { "UpArrow"       , VK_UP         }, // 0x26
    { "RightArrow"    , VK_RIGHT      }, // 0x27
    { "DownArrow"     , VK_DOWN       }, // 0x28
    { "Insert"        , VK_INSERT     }, // 0x2D
    { "Delete"        , VK_DELETE     }, // 0x2E
    { "Help"          , VK_HELP       }, // 0x2F
    { "F1"            , VK_F1         }, // 0x70
    { "F2"            , VK_F2         }, // 0x71
    { "F3"            , VK_F3         }, // 0x72
    { "F4"            , VK_F4         }, // 0x73
    { "F5"            , VK_F5         }, // 0x74
    { "F6"            , VK_F6         }, // 0x75
    { "F7"            , VK_F7         }, // 0x76
    { "F8"            , VK_F8         }, // 0x77
    { "F9"            , VK_F9         }, // 0x78
    { "F10"           , VK_F10        }, // 0x79
    { "F11"           , VK_F11        }, // 0x7A
    { "F12"           , VK_F12        }, // 0x7B
    { "F13"           , VK_F13        }, // 0x7C
    { "F14"           , VK_F14        }, // 0x7D
    { "F15"           , VK_F15        }, // 0x7E
    { "F16"           , VK_F16        }, // 0x7F
    { "F17"           , VK_F17        }, // 0x80
    { "F18"           , VK_F18        }, // 0x81
    { "F19"           , VK_F19        }, // 0x82
    { "F20"           , VK_F20        }, // 0x83
    { "F21"           , VK_F21        }, // 0x84
    { "F22"           , VK_F22        }, // 0x85
    { "F23"           , VK_F23        }, // 0x86
    { "F24"           , VK_F24        }, // 0x87
    { "Numpad0"       , VK_NUMPAD0    }, // 0x60
    { "Numpad 0"      , VK_NUMPAD0    }, // 0x60
    { "Numpad_0"      , VK_NUMPAD0    }, // 0x60
    { "Numpad1"       , VK_NUMPAD1    }, // 0x61
    { "Numpad 1"      , VK_NUMPAD1    }, // 0x61
    { "Numpad_1"      , VK_NUMPAD1    }, // 0x61
    { "Numpad2"       , VK_NUMPAD2    }, // 0x62
    { "Numpad 2"      , VK_NUMPAD2    }, // 0x62
    { "Numpad_2"      , VK_NUMPAD2    }, // 0x62
    { "Numpad3"       , VK_NUMPAD3    }, // 0x63
    { "Numpad 3"      , VK_NUMPAD3    }, // 0x63
    { "Numpad_3"      , VK_NUMPAD3    }, // 0x63
    { "Numpad4"       , VK_NUMPAD4    }, // 0x64
    { "Numpad 4"      , VK_NUMPAD4    }, // 0x64
    { "Numpad_4"      , VK_NUMPAD4    }, // 0x64
    { "Numpad5"       , VK_NUMPAD5    }, // 0x65
    { "Numpad 5"      , VK_NUMPAD5    }, // 0x65
    { "Numpad_5"      , VK_NUMPAD5    }, // 0x65
    { "Numpad6"       , VK_NUMPAD6    }, // 0x66
    { "Numpad 6"      , VK_NUMPAD6    }, // 0x66
    { "Numpad_6"      , VK_NUMPAD6    }, // 0x66
    { "Numpad7"       , VK_NUMPAD7    }, // 0x67
    { "Numpad 7"      , VK_NUMPAD7    }, // 0x67
    { "Numpad_7"      , VK_NUMPAD7    }, // 0x67
    { "Numpad8"       , VK_NUMPAD8    }, // 0x68
    { "Numpad 8"      , VK_NUMPAD8    }, // 0x68
    { "Numpad_8"      , VK_NUMPAD8    }, // 0x68
    { "Numpad9"       , VK_NUMPAD9    }, // 0x69
    { "Numpad 9"      , VK_NUMPAD9    }, // 0x69
    { "Numpad_9"      , VK_NUMPAD9    }, // 0x69
    { "Backquote"     , VK_OEM_1      }, // 0xBA  `
    { "Equals"        , VK_OEM_PLUS   }, // 0xBB  =
    { "Plus"          , VK_OEM_PLUS   }, // 0xBB  -
    { "Comma"         , VK_OEM_COMMA  }, // 0xBC  ,
    { "Minus"         , VK_OEM_MINUS  }, // 0xBD  -
    { "Period"        , VK_OEM_PERIOD }, // 0xBE  .
    { "Slash"         , VK_OEM_2      }, // 0xBF  /
    { "Quote"         , VK_OEM_3      }, // 0xC0  '
    { "LeftBracket"   , VK_OEM_4      }, // 0xDB  [
    { "Left Bracket"  , VK_OEM_4      }, // 0xDB  [
    { "Left_Bracket"  , VK_OEM_4      }, // 0xDB  [
    { "Backslash"     , VK_OEM_5      }, // 0xDC  '\'
    { "RightBracket"  , VK_OEM_6      }, // 0xDD  ]
    { "Right Bracket" , VK_OEM_6      }, // 0xDD  ]
    { "Right_Bracket" , VK_OEM_6      }, // 0xDD  ]
    { "PrintScreen"   , VK_SNAPSHOT   }, // 0x2C  (Print Screen)
    { "Print Screen"  , VK_SNAPSHOT   }, // 0x2C  (Print Screen)
    { "Print_Screen"  , VK_SNAPSHOT   }, // 0x2C  (Print Screen)
};
#define KEYBIND_MAPPING_NAME_MAX_WIDTH (14)
#define KEYBIND_MAPPING_LENGTH \
    (sizeof(KeybindMapping) / sizeof(KeybindMapping_t))

//------------------------------------------------------------------------------
static const InputTypeMapping_t InputTypeMapping[] =
{
    { "Unknown"        , INPUTTYPE_UNKNOWN   },
    { "None"           , INPUTTYPE_NONE      },
    { "Scalar"         , INPUTTYPE_SCALAR    },
    { "DisplayWidth"   , INPUTTYPE_DISPLAY_W },
    { "Display Width"  , INPUTTYPE_DISPLAY_W },
    { "Display_Width"  , INPUTTYPE_DISPLAY_W },
    { "DisplayHeight"  , INPUTTYPE_DISPLAY_H },
    { "Display Height" , INPUTTYPE_DISPLAY_H },
    { "Display_Height" , INPUTTYPE_DISPLAY_H },
    { "Display"        , INPUTTYPE_DISPLAY   },
    { "WindowWidth"    , INPUTTYPE_WINDOW_W  },
    { "Window Width"   , INPUTTYPE_WINDOW_W  },
    { "Window_Width"   , INPUTTYPE_WINDOW_W  },
    { "WindowHeight"   , INPUTTYPE_WINDOW_H  },
    { "Window Height"  , INPUTTYPE_WINDOW_H  },
    { "Window_Height"  , INPUTTYPE_WINDOW_H  },
    { "Window"         , INPUTTYPE_WINDOW    }
};
#define INPUTTYPE_MAPPING_NAME_MAX_WIDTH (15)
#define INPUTTYPE_MAPPING_LENGTH \
    (sizeof(InputTypeMapping) / sizeof(InputTypeMapping_t))

//------------------------------------------------------------------------------
static const JumpDisplayMapping_t JumpDisplayMapping[] =
{
    { "Unknown"  , JUMP_DISPLAY_UNKNOWN },
    { "None"     , JUMP_DISPLAY_NONE    },
    { "Next"     , JUMP_DISPLAY_NEXT    },
    { "Previous" , JUMP_DISPLAY_PREV    },
    { "Prev"     , JUMP_DISPLAY_PREV    },
    { "Left"     , JUMP_DISPLAY_LEFT    },
    { "Right"    , JUMP_DISPLAY_RIGHT   },
    { "Up"       , JUMP_DISPLAY_UP      },
    { "Down"     , JUMP_DISPLAY_DOWN    },
};
#define JUMP_DISPLAY_MAPPING_NAME_MAX_WIDTH (9)
#define JUMP_DISPLAY_MAPPING_LENGTH \
    (sizeof(JumpDisplayMapping) \
    / sizeof(JumpDisplayMapping_t))

//------------------------------------------------------------------------------
static const ProjectDisplayMapping_t ProjectDisplayModeMapping[] =
{
    { "Primary"   , PROJECT_DISPLAY_PRIMARY   },
    { "Main"      , PROJECT_DISPLAY_PRIMARY   },
    { "Duplicate" , PROJECT_DISPLAY_DUPLICATE },
    { "Extend"    , PROJECT_DISPLAY_EXTEND    },
    { "Secondary" , PROJECT_DISPLAY_SECONDARY },
    { "External"  , PROJECT_DISPLAY_SECONDARY },
};
#define PROJECT_DISPLAY_MAPPING_NAME_MAX_WIDTH (10)
#define PROJECT_DISPLAY_MAPPING_LENGTH \
    (sizeof(ProjectDisplayModeMapping) \
    / sizeof(ProjectDisplayMapping_t))



/* Public Function Implementations ===========================================*/
//------------------------------------------------------------------------------
// Mapping_BorderVisibility
//------------------------------------------------------------------------------
static inline const BorderVisibilityMapping_t* Mapping_BorderVisibility(
    const char* string)
{
    assert(string);

    const size_t capacity = BORDER_VISIBILITY_MAPPING_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < BORDER_VISIBILITY_LENGTH; i++)
    {
        const BorderVisibilityMapping_t* p_mapping =
            &(BorderVisibilityMapping[i]);
        if (Utility_StringEquals(p_mapping->name, string, capacity))
        {
            return p_mapping;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
// Mapping_Keybind
//------------------------------------------------------------------------------
static inline const KeybindMapping_t* Mapping_Keybind(
    const char* string)
{
    assert(string);
    static_assert(KEYBIND_MAPPING_LENGTH <= UINT8_MAX);

    const size_t capacity = KEYBIND_MAPPING_NAME_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < KEYBIND_MAPPING_LENGTH; i++)
    {
        const KeybindMapping_t* p_match = &(KeybindMapping[i]);
        if (Utility_StringEquals(p_match->name, string, capacity))
        {
            return p_match;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
// Mapping_InputType
//------------------------------------------------------------------------------
static inline const InputTypeMapping_t* Mapping_InputType(
    const char* string)
{
    assert(string);

    const size_t capacity = INPUTTYPE_MAPPING_NAME_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < INPUTTYPE_MAPPING_LENGTH; i++)
    {
        const InputTypeMapping_t* p_match = &(InputTypeMapping[i]);
        if (Utility_StringEquals(p_match->name, string, capacity))
        {
            return p_match;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
// Mapping_JumpDisplay
//------------------------------------------------------------------------------
static inline const JumpDisplayMapping_t* Mapping_JumpDisplay(
    const char* string)
{
    assert(string);

    const size_t capacity = JUMP_DISPLAY_MAPPING_NAME_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < JUMP_DISPLAY_MAPPING_LENGTH; i++)
    {
        const JumpDisplayMapping_t* p_mapping =
            &(JumpDisplayMapping[i]);
        if (Utility_StringEquals(p_mapping->name, string, capacity))
        {
            return p_mapping;
        }
    }

    return NULL;
}

//------------------------------------------------------------------------------
// Mapping_ProjectDisplay
//------------------------------------------------------------------------------
static inline const ProjectDisplayMapping_t* Mapping_ProjectDisplay(
    const char* string)
{
    assert(string);

    const size_t width = PROJECT_DISPLAY_MAPPING_NAME_MAX_WIDTH;
    for (uint_fast8_t i = 0; i < PROJECT_DISPLAY_MAPPING_LENGTH; i++)
    {
        const ProjectDisplayMapping_t* p_mapping =
            &(ProjectDisplayModeMapping[i]);
        if (Utility_StringEquals(p_mapping->name, string, width))
        {
            return p_mapping;
        }
    }

    return NULL;
}


#endif /* MAPPING_H */
