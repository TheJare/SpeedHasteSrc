// ----------------------------- LLKEY.H -------------------------------
// For use with Watcom C 9.5 and DOS4GW
// (C) Copyright 1993-4 by Jare & JCAB of Iguana-VangeliSTeam.
// Based on code bye Patch/Avalanche.

/*
 * Initialize with LLK_Init, end with LLK_End().
 *  Use the kXXXXX constants to index the LLK_Keys array and check for
 * the pressed keys. LLK_NumKeys will give the # of keys pressed right
 * now, and LLK_SpacePressed is set to either kSPACE or kESC when any
 * of these have been pressed, or else 0; kinda "has he ever pressed
 * ESC ot SPACE?". You may set it to 0 manually to "forget" previous
 * states. A more general variable is LLK_LastScan, which is the same
 * thing but will store any scancode, not just ESC and SPACE.

 * Also provided are pointers to BIOS' key buffer for fast keypress
 * checking: BIOS_KeyXxxx.
 */

#ifndef _LLKEY_H_
#define _LLKEY_H_

#ifndef _BASE_H_
#include <base.h>
#endif

#define BIOS_KeyStart (((word *)0x41A)[0])
#define BIOS_KeyEnd   (((word *)0x41A)[1])

#define LLK_BIOSkbhit()   (BIOS_KeyStart != BIOS_KeyEnd)
#define LLK_BIOSWaitKey() {for (;!LLK_BIOSkbhit(););}
#define LLK_BIOSFlush()   (BIOS_KeyStart = BIOS_KeyEnd)

    // Index this array based on the kXXXXX macros defined below.
    // When an element is TRUE, that key is being held down.
PUBLIC volatile byte LLK_Keys[256];

    // Number of keys currently down.
PUBLIC volatile byte LLK_NumKeys;

    // This one should disappear right now! Stores either kESC or kSPACE.
PUBLIC volatile byte LLK_SpacePressed;

    // Here is stored the last scancode of the last keydown (i.e. no
    // autorepeat or the like). When the application detects a scancode
    // here and processes it, it should also clear it.
PUBLIC volatile byte LLK_LastScan;

#define LLK_kbhit() (LLK_NumKeys != 0)
#define LLK_PressAnyKey() {while(!LLK_kbhit());  while(LLK_kbhit());}

    // --- The handler can be directed to chain back to BIOS or not.

    // ChainChange == TRUE (default) ==> User can switch chaining to BIOS
    // kbd handler, by pressing ScrLock. If FALSE, the user won't be able
    // to change that behaviour.
PUBLIC bool  LLK_ChainChange;
    // Actual chaining flag. In the beginning, there is no chaining but
    // changes are allowed.
PUBLIC bool  LLK_DoChain;
    // Allow keyoboard controller's autorepeat.
PUBLIC bool  LLK_Autorepeat;

PUBLIC void  LLK_Init(void);
PUBLIC void  LLK_End(void);

#define         kSYSREQ              0x54
#define         kCAPSLOCK            0x3A
#define         kNUMLOCK             0x45
#define         kSCROLLLOCK          0x46
#define         kLEFTCTRL            0x1D
#define         kLEFTALT             0x38
#define         kLEFTSHIFT           0x2A
#define         kRIGHTCTRL           0x9D
#define         kRIGHTALT            0xB8
#define         kRIGHTSHIFT          0x36
#define         kESC                 0x01
#define         kBACKSPACE           0x0E
#define         kENTER               0x1C
#define         kSPACE               0x39
#define         kTAB                 0x0F
#define         kF1                  0x3B
#define         kF2                  0x3C
#define         kF3                  0x3D
#define         kF4                  0x3E
#define         kF5                  0x3F
#define         kF6                  0x40
#define         kF7                  0x41
#define         kF8                  0x42
#define         kF9                  0x43
#define         kF10                 0x44
#define         kF11                 0x57
#define         kF12                 0x58
#define         kA                   0x1E
#define         kB                   0x30
#define         kC                   0x2E
#define         kD                   0x20
#define         kE                   0x12
#define         kF                   0x21
#define         kG                   0x22
#define         kH                   0x23
#define         kI                   0x17
#define         kJ                   0x24
#define         kK                   0x25
#define         kL                   0x26
#define         kM                   0x32
#define         kN                   0x31
#define         kO                   0x18
#define         kP                   0x19
#define         kQ                   0x10
#define         kR                   0x13
#define         kS                   0x1F
#define         kT                   0x14
#define         kU                   0x16
#define         kV                   0x2F
#define         kW                   0x11
#define         kX                   0x2D
#define         kY                   0x15
#define         kZ                   0x2C
#define         k1                   0x02
#define         k2                   0x03
#define         k3                   0x04
#define         k4                   0x05
#define         k5                   0x06
#define         k6                   0x07
#define         k7                   0x08
#define         k8                   0x09
#define         k9                   0x0A
#define         k0                   0x0B
#define         kMINUS               0x0C
#define         kEQUAL               0x0D
#define         kLBRACKET            0x1A
#define         kRBRACKET            0x1B
#define         kSEMICOLON           0x27
#define         kTICK                0x28
#define         kAPOSTROPHE          0x29
#define         kBACKSLASH           0x2B
#define         kCOMMA               0x33
#define         kPERIOD              0x34
#define         kSLASH               0x35
#define         kINS                 0xD2
#define         kDEL                 0xD3
#define         kHOME                0xC7
#define         kEND                 0xCF
#define         kPGUP                0xC9
#define         kPGDN                0xD1
#define         kLARROW              0xCB
#define         kRARROW              0xCD
#define         kUARROW              0xC8
#define         kDARROW              0xD0
#define         kKEYPAD0             0x52
#define         kKEYPAD1             0x4F
#define         kKEYPAD2             0x50
#define         kKEYPAD3             0x51
#define         kKEYPAD4             0x4B
#define         kKEYPAD5             0x4C
#define         kKEYPAD6             0x4D
#define         kKEYPAD7             0x47
#define         kKEYPAD8             0x48
#define         kKEYPAD9             0x49
#define         kKEYPADDEL           0x53
#define         kKEYPADSTAR          0x37
#define         kKEYPADMINUS         0x4A
#define         kKEYPADPLUS          0x4E
#define         kKEYPADENTER         0x9C
#define         kPRTSC               0xB7
#define         kCTRLPRTSC           0xB7
#define         kSHIFTPRTSC          0xB7
#define         kKEYPADSLASH         0xB5
#define         kCTRLBREAK           0xC6
#define         kPAUSE               0xC5

#endif

/* -------------------------- End of LLKEY.H --------------------------- */
