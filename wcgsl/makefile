
WOPTIONS=-d1 -fp3 -mf -5r -onteaxl -s -w9 -zp4 -zq -dNDEBUG
#WOPTIONS=-d2 -fp3 -mf -5r -onteaxl -s -w9 -zp4 -zq -dNDEBUG
#WOPTIONS=-d2 -fp3 -mf -3r -ox -s -w9 -zp4 -zq -dNDEBUG
#WOPTIONS=-d2 -fp3 -mf -5r -s -w9 -zp4 -zq
LIB=wlib -c -q
CC=wcc386 $(WOPTIONS)
AS=tasm -ml -t -w2 -z -zi -m9 -jJUMPS -iH
LN=wcl386 $(WOPTIONS) -x -fm -l=dos4g

OBJS = \
     animatec.obj \
     atan.obj     \
     atantbl.obj  \
     base.obj     \
     bitmap.obj   \
     comms.obj    \
     controls.obj \
     cspline.obj  \
     cursor.obj   \
     desktop.obj  \
     dpmi.obj     \
     dpmia.obj    \
     gamevtal.obj \
     graphics.obj \
     hard.obj     \
     hordrawa.obj \
     is2code.obj  \
     jclib.obj    \
     joystick.obj \
     keynames.obj \
     lbm.obj      \
     llipx.obj    \
     llkeya.obj   \
     llkeyc.obj   \
     llmouse.obj  \
     llscra.obj   \
     llscrc.obj   \
     llvesa.obj   \
     m12utila.obj \
     m12utilc.obj \
     namelist.obj \
     object3d.obj \
     pix.obj      \
     polygona.obj \
     polygonc.obj \
     rot3da.obj   \
     rot3dc.obj   \
     serial.obj   \
     sincos.obj   \
     sincos_i.obj \
     sqrta.obj    \
     strlist.obj  \
     strparse.obj \
     text.obj     \
     textscr.obj  \
     timer.obj    \
     vbl.obj      \
     vblc.obj     \
     vgaa.obj     \
     vgac.obj     \
     vertdrwa.obj \
     vertdrwc.obj \
     vtal.obj

$(WATCOM)\lib386\local.lib: $(OBJS) local.wln
    $(LIB) $(WATCOM)\lib386\local.lib @local.wln

.c.obj:
    $(CC) $<
    $(LIB) $(WATCOM)\lib386\local.lib -+$@

.asm.obj:
    $(AS) $<
    $(LIB) $(WATCOM)\lib386\local.lib -+$@

controls.c: h\controls.h

h\controls.h: h\desktop.h h\animate.h h\bitmap.h

