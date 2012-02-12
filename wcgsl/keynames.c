// ----------------------------- KEYNAMES.C -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Based on constants bye Patch/Avalanche.

#include <keynames.h>

#include <string.h>

PUBLIC KEYN_TKey KEYN_Keys[] =  {
    {kSYSREQ              , "SysReq."           },
    {kCAPSLOCK            , "Caps Lock"         },
    {kNUMLOCK             , "Num.Lock"          },
    {kSCROLLLOCK          , "Scr.Lock"          },
    {kLEFTCTRL            , "Left Ctrl"         },
    {kLEFTALT             , "Left Alt"          },
    {kLEFTSHIFT           , "Left Shift"        },
    {kRIGHTCTRL           , "Right Ctrl"        },
    {kRIGHTALT            , "Right Alt"         },
    {kRIGHTSHIFT          , "Right Shift"       },
    {kESC                 , "ESC"               },
    {kBACKSPACE           , "Backspace"         },
    {kENTER               , "Return"            },
    {kSPACE               , "Space"             },
    {kTAB                 , "TAB"               },
    {kF1                  , "F1"                },
    {kF2                  , "F2"                },
    {kF3                  , "F3"                },
    {kF4                  , "F4"                },
    {kF5                  , "F5"                },
    {kF6                  , "F6"                },
    {kF7                  , "F7"                },
    {kF8                  , "F8"                },
    {kF9                  , "F9"                },
    {kF10                 , "F10"               },
    {kF11                 , "F11"               },
    {kF12                 , "F12"               },
    {kA                   , "A"                 },
    {kB                   , "B"                 },
    {kC                   , "C"                 },
    {kD                   , "D"                 },
    {kE                   , "E"                 },
    {kF                   , "F"                 },
    {kG                   , "G"                 },
    {kH                   , "H"                 },
    {kI                   , "I"                 },
    {kJ                   , "J"                 },
    {kK                   , "K"                 },
    {kL                   , "L"                 },
    {kM                   , "M"                 },
    {kN                   , "N"                 },
    {kO                   , "O"                 },
    {kP                   , "P"                 },
    {kQ                   , "Q"                 },
    {kR                   , "R"                 },
    {kS                   , "S"                 },
    {kT                   , "T"                 },
    {kU                   , "U"                 },
    {kV                   , "V"                 },
    {kW                   , "W"                 },
    {kX                   , "X"                 },
    {kY                   , "Y"                 },
    {kZ                   , "Z"                 },
    {k1                   , "1"                 },
    {k2                   , "2"                 },
    {k3                   , "3"                 },
    {k4                   , "4"                 },
    {k5                   , "5"                 },
    {k6                   , "6"                 },
    {k7                   , "7"                 },
    {k8                   , "8"                 },
    {k9                   , "9"                 },
    {k0                   , "0"                 },
    {kMINUS               , "-"                 },
    {kEQUAL               , "="                 },
    {kLBRACKET            , "["                 },
    {kRBRACKET            , "]"                 },
    {kSEMICOLON           , ";"                 },
    {kTICK                , "^"                 },
    {kAPOSTROPHE          , "'"                 },
    {kBACKSLASH           , "\\"                },
    {kCOMMA               , ","                 },
    {kPERIOD              , "."                 },
    {kSLASH               , "/"                 },
    {kINS                 , "Insert"            },
    {kDEL                 , "Delete"            },
    {kHOME                , "Home"              },
    {kEND                 , "End"               },
    {kPGUP                , "PageUp"            },
    {kPGDN                , "PageDown"          },
    {kLARROW              , "Left"              },
    {kRARROW              , "Right"             },
    {kUARROW              , "Up"                },
    {kDARROW              , "Down"              },
    {kKEYPAD0             , "Keypad 0"          },
    {kKEYPAD1             , "Keypad 1"          },
    {kKEYPAD2             , "Keypad 2"          },
    {kKEYPAD3             , "Keypad 3"          },
    {kKEYPAD4             , "Keypad 4"          },
    {kKEYPAD5             , "Keypad 5"          },
    {kKEYPAD6             , "Keypad 6"          },
    {kKEYPAD7             , "Keypad 7"          },
    {kKEYPAD8             , "Keypad 8"          },
    {kKEYPAD9             , "Keypad 9"          },
    {kKEYPADDEL           , "Keypad Del"        },
    {kKEYPADSTAR          , "Keypad *"          },
    {kKEYPADMINUS         , "Keypad -"          },
    {kKEYPADPLUS          , "Keypad +"          },
    {kKEYPADENTER         , "Keypad Enter"      },
    {kPRTSC               , "Print Scr"         },
    {kCTRLPRTSC           , "CtrlPrtScr"        },
    {kSHIFTPRTSC          , "ShiftPrtScr"       },
    {kKEYPADSLASH         , "Keypad /"          },
    {kCTRLBREAK           , "CtrlBreak"         },
    {kPAUSE               , "Pause"             },
};

// ----------------

PUBLIC const char*KEYN_FindKey(byte key) {
    int i;
    for (i = 0; i < SIZEARRAY(KEYN_Keys); i++)
        if (key == KEYN_Keys[i].key)
            return KEYN_Keys[i].name;
    return NULL;
}

PUBLIC byte KEYN_FindKeyCode(const char *name) {
    int i;
    for (i = 0; i < SIZEARRAY(KEYN_Keys); i++)
        if (stricmp(name, KEYN_Keys[i].name) == 0)
            return KEYN_Keys[i].key;
    return 0;
}

// ----------------------------- KEYNAMES.C -------------------------------
