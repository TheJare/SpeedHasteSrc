// --------------------------- SERIAL.C ------------------------------
// For use with WATCOM 9.5 + DOS4GW
// (C) Copyright 1993/4 by Jare & JCAB of Iguana.


#include "serial.h"
#include <dpmi.h>
#include <dos.h>
#include <i86.h>
#include <conio.h>
#include <string.h>
#include <stdio.h>

#include <llkey.h>


PUBLIC void SER_InitComInfo(SER_PComPort p, int com, int port, int irq) {
    static struct {
        word port;
        word irq;
    } isacom[4] = {
        { 0x3f8, 4},
        { 0x2f8, 3},
        { 0x3e8, 4},
        { 0x2e8, 3},
    }, mcacom[4] = {
        { 0x3f8, 4},
        { 0x2f8, 3},
        {0x3220, 3},
        {0x3228, 3},
    }, *c;

    assert(p != NULL);
    assert(com >= 0);
    assert(com < 4);

    p->com = com;
    c = isacom;
    if (port < 0 || irq < 0) {      // If forced, no need to call the INT.
        DPMI_rmi.b.AH = 0xC0;       // PS/2 system configuration parameters.
        DPMI_RealModeInt(0x15);     // BIOS system services.
        if (!(DPMI_rmi.w.flags & FLAGS_CARRY)) {
            byte *data;
            data = (byte*)DPMI_MK_PTR(DPMI_rmi.w.ES, DPMI_rmi.w.BX);
            if (data[5] & 0x02)     // Bus type.
                c = mcacom;
        }
    }
    if (port >= 0)
        p->port = port;
    else
//        p->port = c[com].port;
        p->port = ((word*)0x400)[com];      // Get from BIOS.
    if (irq >= 0)
        p->irq  = irq;
    else {
//        p->irq  = c[com].irq;
        int i;
        for (i = 0; i < 4; i++) {           // Look in the table.
            if (c[i].port == p->port)
                p->irq  = c[i].irq;
        }
    }
    p->flags  = 0;
    p->oldint = NULL;
}

PUBLIC bool SER_InitComPort(SER_PComPort p) {
    byte t;
    assert(p != NULL);

        // Reset port parms.
    DPMI_rmi.w.AX = 0xF3;       // 9600 8 n 1
    DPMI_rmi.w.DX = p->com;
    DPMI_RealModeInt(0x14);

    if (DPMI_rmi.b.AH & SERR_LSR_TIMEOUT)
        return FALSE;

        // Check for 16550.
    outp(p->port + SERR_FCR, SERR_FCR_ENABLE + SERR_FCR_TRIGGER04);
    t = inp(p->port + SERR_IIR);
    if ((t & 0xF8) == 0xC0)
        p->flags |= SERPF_IS16550;
    else {
        outp(p->port + SERR_FCR, 0);
    }

    return TRUE;
}

PUBLIC void SER_InitComInterrupts(SER_PComPort p, void __interrupt (__far *isr)(void)) {
    byte t;
    assert(p != NULL);

        // Init interrupts.
    outp(p->port + SERR_IER, 0);
    t = inp(p->port + SERR_MCR);
    t |= SERR_MCR_OUT2;
    t &= ~SERR_MCR_LOOPBACK;
    outp(p->port + SERR_MCR, t);

    if (isr != NULL) {
            // Save previous vector and hook vector.
        p->oldint = _dos_getvect(p->irq + 8);
        _dos_setvect(p->irq + 8, isr);
    }
        // Setup comm interrupts.

    outp(0x21, inp(0x21) & ~(1 << p->irq)); // Mask in port's IRQ.
    _disable();
    outp(p->port + SERR_IER, SERR_IER_RXREADY | SERR_IER_TXREADY);
    outp(0x20, 0xC2);

        // Raise DTR
    outp(p->port + SERR_MCR, inp(p->port + SERR_MCR) | SERR_MCR_DTR);

    _enable();
}

PUBLIC void SER_EndComPort(SER_PComPort p) {
    assert(p != NULL);

        // End port comms.
    outp(p->port + SERR_IER, 0);        
    outp(p->port + SERR_MCR, 0);

        // Mask out IRQ.
    outp(0x21, inp(0x21) | (1 << p->irq));

    if (p->oldint != NULL) {
        _dos_setvect(p->irq + 8, p->oldint);
        p->oldint = NULL;
    }

        // Reset port parms.
    DPMI_rmi.w.AX = 0xF3;       // 9600 8 n 1
    DPMI_rmi.w.DX = p->com;
    DPMI_RealModeInt(0x14);
}

// ------------------------------------------------------
// --------------- Cool stream-based functions.

    // Init structure, hook to interrupts and setup port interrupts.
PUBLIC void SER_InitStream(SER_PStream s, void __interrupt (__far *isr)(void)) {
    assert(s != NULL);

    s->in.head  = s->in.tail  = 0;      // Clean incoming buffer.
    s->out.head = s->out.tail = 0;      // Clean outgoing buffer.
    s->inframe  = FALSE;
    s->blocklen = 0;
    if (isr != NULL)            // Don't init INTs yet.
        SER_InitComInterrupts(&s->port, isr);
}

    // Add byte to stream buffer.
PUBLIC void SER_WriteStreamChar(SER_PStream s, byte c) {
    s->out.buffer[s->out.head % SERS_SIZE] = c;
    s->out.head++;
}

    // Get byte from stream buffer.
PUBLIC int  SER_ReadStreamChar(SER_PStream s) {
    byte c;
    if (s->in.tail >= s->in.head)
        return -1;
    c = s->in.buffer[s->in.tail % SERS_SIZE];
    s->in.tail++;
    return (int)c;
}

    // Unget a read stream char.
PUBLIC void SER_UngetStreamChar(SER_PStream s, int c) {
    s->in.tail--;
    s->in.buffer[s->in.tail % SERS_SIZE] = c;
}

PUBLIC void SER_WriteStream(SER_PStream s, const byte *c, int len) {
    if (len < 0)
        for (;;) {
            SER_WriteStreamChar(s, *c);
            if (*c == '\0')
                break;
            c++;
        }
    else
        while (len > 0) {
            SER_WriteStreamChar(s, *c);
            c++;
            len--;
        }
}

PUBLIC int SER_ReadStream(SER_PStream s, byte *c, int max) {
    int l;

    l = 0;
    while (l < max) {
        int d;
        if ( (d = SER_ReadStreamChar(s)) < 0)
            break;
        *c++ = (byte)d;
        l++;
    }
    return l;
}

// ----------- Block functions.

PUBLIC void SER_InitBlockRead(SER_PStream s) {
    s->inframe  = FALSE;
    s->blocklen = 0;
}

PUBLIC void SER_WriteBlock(SER_PStream s, const byte *c, int len) {
    while (len > 0) {
        if (*c == SER_SEP || *c == SER_ESC)
            SER_WriteStreamChar(s, SER_ESC);
        SER_WriteStreamChar(s, *c);
        c++;
        len--;
    }
    SER_WriteStreamChar(s, SER_SEP);
}
/*
PUBLIC int SER_ReadBlock(SER_PStream s, byte *c, int max) {
    int l, i;
    int h, t;
    bool sepfound;

    l = 0;
    sepfound = FALSE;
    for (h = s->in.head, t = s->in.tail; t != h; t++) {
        if (sepfound) {     // Was prev char a SEP?
            if (s->in.buffer[t % SERS_SIZE] != SER_SEP) {
                break;
            }
            sepfound = FALSE;
        } else {
            l++;
            if (s->in.buffer[t % SERS_SIZE] == SER_SEP)
                sepfound = TRUE;
        }
    }
    if (l <= 0)
        return -1;
        // Smaller block without ending SEP, or with SEP but without anything
        // else in the buffer?
    if ((l-sepfound) < max && h == t)
        return -1;
    if ((l-sepfound) > max) // sepfound will mean l was ++'ed.
        l = max;
    i = 0;
    sepfound = FALSE;
    while (i < l) {
        int d;
        if ( (d = SER_ReadStreamChar(s)) < 0)
            break;
        if (d != SER_SEP || sepfound) { // Assign only if char or second sep.
            *c++ = (byte)d;
            if (!sepfound)  // If was sep, it's already incremented.
                i++;
            sepfound = FALSE;
        } else {
            sepfound = TRUE;
            i++;
        }
    }
        // If sepfound, last i++ didn't mean an assignment.
    return i - (sepfound);
}
*/
/*
PUBLIC int SER_ReadBlock(SER_PStream s, byte *c, int max) {
    int  d;

    if (max > sizeof(s->block))
        max = sizeof(s->block);
    while (s->blocklen <= max) {
        d = SER_ReadStreamChar(s);
        if (d < 0)
            break;
            // Got a SEP followed by a non-SEP? Last one is from the next packet.
        if (s->inframe && d != SER_SEP) {
            SER_UngetStreamChar(s, d);
            break;
        }
        if (s->blocklen >= max)
            break;
        if (s->inframe || d != SER_SEP) {
            s->block[s->blocklen++] = d;
            s->inframe = FALSE;
        } else
            s->inframe = TRUE;
    }
        // Got a smaller packet and there was nothing more? Don't read it
        // yet, cause the last SEP could be the first of a SEP PAIR coming.
    if (s->blocklen < max && d < 0)
        return 0;
    max = s->blocklen;
    memcpy(c, s->block, s->blocklen);
    SER_InitBlockRead(s);
    return max;
}
*/
PUBLIC int SER_ReadBlock(SER_PStream s, byte *c, int max) {
    int  d = SER_SEP;

//    if (s->blocklen < max)
    for (;;) {
        d = SER_ReadStreamChar(s);
        if (d < 0)                          // No more data? Quit loop.
            break;
        if (!s->inframe && d == SER_SEP)    // End of frame? Quit loop.
            break;
        if (s->blocklen >= sizeof(s->block)) // Buffer overflowed? Discard.
            s->blocklen = 0;
        if (s->inframe || d != SER_ESC) {
            s->block[s->blocklen++] = d;
            s->inframe = FALSE;
        } else
            s->inframe = TRUE;
    }
    if (d != SER_SEP)
        return -1;
    if (s->blocklen > max)      // Packet bigger than available? Clip it.
        s->blocklen = max;
    max = s->blocklen;
    memcpy(c, s->block, s->blocklen);
    SER_InitBlockRead(s);
    return max;
}

    // Start transmission of buffer in stream.
PUBLIC void SER_StreamTransmit(SER_PStream s) {
	if (inp(s->port.port + SERR_LSR) & SERR_LSR_TXSHIFTEMPTY)
        if (s->out.head > s->out.tail) {
            byte c = s->out.buffer[s->out.tail % SERS_SIZE];
            s->out.tail++;  // Interrupts will also increment this, so prepare.
            outp(s->port.port + SERR_TRANSMIT, c);
        }
}

    // Most of the ISR is here.
PUBLIC void SER_StreamHandle(SER_PStream s) {
    for (;;) {
        byte c = inp(s->port.port + SERR_IIR) & 7;
        switch (c) {
            case SERR_IIR_MODEMSTATUS:
                inp(s->port.port + SERR_MSR);
                break;

            case SERR_IIR_LINESTATUS :
                inp(s->port.port + SERR_LSR);
                break;

            case SERR_IIR_TXREADY:
                if (s->out.tail < s->out.head) {
                    int count;
                    if (s->port.flags & SERPF_IS16550)
                        count = 16;
                    else
                        count = 1;
                    do {
                       byte c;
                       c = s->out.buffer[s->out.tail % SERS_SIZE];
                       s->out.tail++;
                       outp(s->port.port + SERR_TRANSMIT, c);
                    } while (--count > 0 && s->out.tail < s->out.head);
                }
                break;

            case SERR_IIR_RXREADY:
                do {
                    s->in.buffer[s->in.head % SERS_SIZE]
                            = inp(s->port.port + SERR_RECEIVE);
                    s->in.head++;
                } while ((s->port.flags & SERPF_IS16550)
                      && inp(s->port.port + SERR_LSR) & SERR_LSR_RXREADY);
                break;

            default :
                outp(0x20, 0x20);       // EOI
                return;
        }
    }
}

// -------------------------------------
// Modem stuff. Establish connection thru a phone line.

    // -1 if ESC, else SERMS_MATCH if 'data' received, else SERMS_xxxx
PUBLIC int SER_WaitModem(SER_PStream s, const char *data) {
    int ri;
    int ret = -1;
    LLK_BIOSFlush();
    LLK_LastScan = 0;
    ri = 0;
    while (ret == -1) {
        char resp[200];
        int c;

        if (LLK_LastScan == kESC)
            break;
        if (LLK_BIOSkbhit())
            if (getch() == 27)
                break;

        c = SER_ReadStreamChar(s);
        if (c < 0)
            continue;
        if (c == '\n' || ri >= SIZEARRAY(resp)) {
            resp[ri] = '\0';
            ri = 0;
            if (data != NULL && *data != '\0' && strncmp(resp, data, strlen(data)) == 0)
    	        ret = SERMS_MATCH;
            else {
                static const char *strs[] = {
                    "OK",
                    "RING",
                    "BUSY",
                    "NO CARRIER",
                    "NO DIAL",
                    "CONNECT",
                };
                int i;
                for (i = 0; ret == -1 && i < SIZEARRAY(strs); i++) {
                    if (strncmp(resp, strs[i], strlen(strs[i])) == 0)
                        ret = i + SERMS_OK;
                }
            }
		} else if (c >= ' ')
			resp[ri++] = c;
    }
    LLK_BIOSFlush();
    LLK_LastScan = 0;
    return ret;
}

    // Should return SERMS_OK
PUBLIC int SER_InitModem(SER_PStream s, SER_PModemCfg cfg) {
    int ret;
    SER_WriteStream(s, cfg->initstr, strlen(cfg->initstr));
    SER_WriteStreamChar(s, '\r');
    SER_StreamTransmit(s);
	while ( (ret = SER_WaitModem(s, NULL)) != SERMS_OK && ret >= 0);
    return ret;
}

    // Should return SERMS_CONNECT
PUBLIC int SER_Dial(SER_PStream s, SER_PModemCfg cfg, const char *number) {
    char buf[200];
    int ret;
    sprintf(buf, "%s%s\r", cfg->dialstr, number);
    SER_WriteStream(s, buf, strlen(buf));
    SER_StreamTransmit(s);
	while ( (ret = SER_WaitModem(s, NULL)) != SERMS_CONNECT)
        if (ret < 0 || ret == SERMS_NODIALTONE) {
            SER_Hangup(s, cfg);
            break;
        }
    return ret;
}

    // Should return SERMS_CONNECT
PUBLIC int SER_Answer(SER_PStream s, SER_PModemCfg cfg) {
    int ret;
	while ( (ret = SER_WaitModem(s, NULL)) != SERMS_RING)
        if (ret < 0)
    	    return ret;
	SER_WriteStream(s, "ATA\r", strlen("ATA\r"));
    SER_StreamTransmit(s);
	while ( (ret = SER_WaitModem(s, NULL)) != SERMS_CONNECT)
        if (ret < 0) {
            SER_Hangup(s, cfg);
            break;
        }
    return ret;
}

    // Should return SERMS_OK
PUBLIC int SER_Hangup(SER_PStream s, SER_PModemCfg cfg) {
    SER_WriteStreamChar(s, 27);
    SER_StreamTransmit(s);
    if (cfg->delayfunc == NULL) delay (1250);
    else                        cfg->delayfunc(1250);
    outp(s->port.port + SERR_MCR, inp(s->port.port+SERR_MCR) & ~SERR_MCR_DTR);
    if (cfg->delayfunc == NULL) delay (1250);
    else                        cfg->delayfunc(1250);
    outp(s->port.port + SERR_MCR, inp(s->port.port+SERR_MCR) | SERR_MCR_DTR);
    SER_WriteStream(s, "+++", strlen("+++"));
    SER_StreamTransmit(s);
    if (cfg->delayfunc == NULL) delay (1250);
    else                        cfg->delayfunc(1250);
    SER_WriteStream(s, cfg->hangstr, strlen(cfg->hangstr));
    SER_WriteStreamChar(s, '\r');
    SER_StreamTransmit(s);
    return SERMS_OK;
}

// ------------------------------------------------------
// --------------- Scrap stuff to ease use.

SER_TStream SER_ScrapStream;

PUBLIC void __interrupt __far __loadds SER_ScrapISR(void) {
    SER_StreamHandle(&SER_ScrapStream);
}

// --------------------------- SERIAL.C ------------------------------
