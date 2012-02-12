// --------------------------- SERIAL.H ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.

#ifndef _SERIAL_H_
#define _SERIAL_H_

#ifndef _BASE_H_
#include <base.h>
#endif

typedef struct {
    word    com;
    word    port;
    word    irq;
    word    flags;
    void __interrupt (__far *oldint)(void);
} SER_TComPort, *SER_PComPort;

enum {
    SERPF_IS16550 = 0x0001,

    SERR_TRANSMIT   = 0,
    SERR_RECEIVE    = 0,
    SERR_IER        = 1,
    SERR_IER_RXREADY        = 0x01,
    SERR_IER_TXREADY        = 0x02,
    SERR_IER_LINESTATUS     = 0x04,
    SERR_IER_MODEMSTATUS    = 0x08,
    SERR_IIR        = 2,
    SERR_IIR_NOINT          = 0x01,
    SERR_IIR_MODEMSTATUS    = 0x00,
    SERR_IIR_TXREADY        = 0x02,
    SERR_IIR_RXREADY        = 0x04,
    SERR_IIR_LINESTATUS     = 0x06,
    SERR_FCR        = 2,
    SERR_FCR_ENABLE         = 0x01,
    SERR_FCR_RXRESET        = 0x02,
    SERR_FCR_TXRESET        = 0x04,
    SERR_FCR_RXLSB          = 0x40,
    SERR_FCR_RXMSB          = 0x80,
    SERR_FCR_TRIGGER01      = 0x00,
    SERR_FCR_TRIGGER04      = 0x40,
    SERR_FCR_TRIGGER08      = 0x80,
    SERR_FCR_TRIGGER14      = 0xC0,
    SERR_LCR        = 3,
    SERR_LCR_NBITMASK       = 0x03,
    SERR_LCR_NBITSELECT0    = 0x01,     //..
    SERR_LCR_NBITSELECT1    = 0x02,
    SERR_LCR_STOPBITS       = 0x04,
    SERR_LCR_PARITYMASK     = 0x38,
    SERR_LCR_PARITYODD      = 0x08,
    SERR_LCR_PARITYEVEN     = 0x18,
    SERR_LCR_PARITYSTICK    = 0x28,
    SERR_LCR_SETBREAK       = 0x40,
    SERR_LCR_DLAB           = 0x80,
    SERR_MCR        = 4,
    SERR_MCR_DTR            = 0x01,
    SERR_MCR_RTS            = 0x02,
    SERR_MCR_OUT1           = 0x04,
    SERR_MCR_OUT2           = 0x08,
    SERR_MCR_LOOPBACK       = 0x10,
    SERR_LSR        = 5,
    SERR_LSR_RXREADY        = 0x01,
    SERR_LSR_OVERRUNERROR   = 0x02,
    SERR_LSR_PARITYERROR    = 0x04,
    SERR_LSR_FRAMINGERROR   = 0x08,
    SERR_LSR_BREAK          = 0x10,
    SERR_LSR_TXREADY        = 0x20,
    SERR_LSR_TXSHIFTEMPTY   = 0x40,
    SERR_LSR_TIMEOUT        = 0x80,
    SERR_MSR        = 6,
    SERR_MSR_DELTACTS       = 0x01,
    SERR_MSR_DELTADSR       = 0x02,
    SERR_MSR_TERI           = 0x04,
    SERR_MSR_DELTACD        = 0x08,
    SERR_MSR_CTS            = 0x10,
    SERR_MSR_DSR            = 0x20,
    SERR_MSR_RI             = 0x40,
    SERR_MSR_CD             = 0x80,
};

PUBLIC void SER_InitComInfo(SER_PComPort p, int com, int port, int irq);

PUBLIC bool SER_InitComPort(SER_PComPort p);

PUBLIC void SER_InitComInterrupts(SER_PComPort p, void __interrupt (__far *isr)(void));

PUBLIC void SER_EndComPort(SER_PComPort p);

// ------------------------------------------------------
// --------------- Cool stream-based functions.

enum {
    SERS_SIZE = 2048,

    SER_SEP = 0x55,         // Binary 01010101
    SER_ESC = 0x66,         // Binary 01100110
};

typedef struct {
    struct {
        dword   head, tail;             // Write on head, read from tail.
        byte    buffer[SERS_SIZE];
    } in, out;
    SER_TComPort port;
    byte  block[200];
    dword blocklen;
    bool  inframe;
} SER_TStream, *SER_PStream;

    // Init structure, hook to interrupts and setup port interrupts.
PUBLIC void SER_InitStream(SER_PStream s, void __interrupt (__far *isr)(void));

    // Add byte to stream buffer.
PUBLIC void SER_WriteStreamChar(SER_PStream s, byte c);

    // Get byte from stream buffer.
PUBLIC int  SER_ReadStreamChar(SER_PStream s);

    // Unget a read stream char.
PUBLIC void SER_UngetStreamChar(SER_PStream s, int c);

    // Write a buffer. If len < 0, write up to and INCLUDING a '\0'.
PUBLIC void SER_WriteStream(SER_PStream s, const byte *c, int len);

    // Read up to 'max' bytes. Returns number of bytes read.
PUBLIC int SER_ReadStream(SER_PStream s, byte *c, int max);

PUBLIC void SER_InitBlockRead(SER_PStream s);

    // Write len bytes. Will duplicate SER_SEP characters and
    // add another one at the end.
PUBLIC void SER_WriteBlock(SER_PStream s, byte *c, int len);

    // Read next bytes up to max or the SER_SEP character is found.
    // Will unify duplicate SER_SERP chars. Returns number of bytes read.
PUBLIC int SER_ReadBlock(SER_PStream s, byte *c, int max);

    // Start transmission of buffer in stream.
PUBLIC void SER_StreamTransmit(SER_PStream s);

    // Most of the ISR is here.
PUBLIC void SER_StreamHandle(SER_PStream s);

// -------------------------------------
// Modem stuff. Establish connection thru a phone line.

typedef struct {
    const char *initstr;
    const char *dialstr;
    const char *hangstr;
    void (*delayfunc)(int millisec);
} SER_TModemCfg, *SER_PModemCfg;

enum {
    SERMS_MATCH,        // When waiting for a string, exit with <= 0
    SERMS_OK,
    SERMS_RING,
    SERMS_BUSY,
    SERMS_NOCARRIER,
    SERMS_NODIALTONE,
    SERMS_CONNECT,
    SERMS_OTHER,
};

    // -1 if ESC, else SERMS_MATCH if 'data' received, else SERMS_xxxx
PUBLIC int SER_WaitModem(SER_PStream s, const char *data);

    // Should return SERMS_OK
PUBLIC int SER_InitModem(SER_PStream s, SER_PModemCfg cfg);

    // Should return SERMS_CONNECT
PUBLIC int SER_Dial(SER_PStream s, SER_PModemCfg cfg, const char *number);

    // Should return SERMS_CONNECT
PUBLIC int SER_Answer(SER_PStream s, SER_PModemCfg cfg);

    // Should return SERMS_OK
PUBLIC int SER_Hangup(SER_PStream s, SER_PModemCfg cfg);

// ------------------------------------------------------
// --------------- Scrap stuff to ease use.

PUBLIC SER_TStream SER_ScrapStream;

PUBLIC void __interrupt __far __loadds SER_ScrapISR(void);


#endif

// --------------------------- SERIAL.H ------------------------------

