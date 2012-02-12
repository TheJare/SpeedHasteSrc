
NAME=install
EXE=install

WOPTIONS=-d1 -fpi -ms -r -2 -oneatxl -w9 -zp2 -zq -DNDEBUG
#WOPTIONS=-d2 -fpi -ml -r -2 -w9 -zp4 -zq
LIB=wlib -c -q
CC=wcc $(WOPTIONS)
AS=tasm -l -m9 -ml -t -w2 -z -zi -i$(WATCOM)\LOCAL\H  -jJUMPS
#AS=tasm -m9 -ml -t -w2 -z -zi -i$(WATCOM)\LOCAL\H  -jJUMPS
LN=wcl -d2 -l=dos -fe=$(EXE) /fm
#LN=wcl -l=dos -fe=$(EXE)
#LN=wcl -l=pmodew -fe=3daview

LIBS=
OBJS=$(NAME).obj base.obj textscr.obj

$(EXE).exe: $(OBJS) $(LIBS)
        $(LN) $(OBJS)
        #$(OBJS)

.c.obj:
    $(CC) $<

.asm.obj:
    $(AS) $<

install.obj: textscr.h

textscr.obj: textscr.h

textscr.h: base.h

base.obj: base.h

