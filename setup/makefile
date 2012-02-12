
NAME=setup

WOPTIONS=-d1 -fp3 -mf -3r -oneatxl -w9 -zp4 -zq -DNDEBUG
#WOPTIONS=-d2 -fp3 -mf -3r -w9 -zp4 -zq
LIB=wlib -c -q
CC=wcc386 $(WOPTIONS)
AS=tasm -l -m9 -ml -t -w2 -z -zi -i$(WATCOM)\LOCAL\H  -jJUMPS
LN=wcl386 -l=dos4g /"option stub=wstubq"
#LN=wcl386 -d2 -l=dos4g /"option stub=wstubq"
#LN=wcl386 -d2 -l=pmodew

LIBS=$(WATCOM)\lib386\local.lib
OBJS=config.obj

$(NAME).exe: $(NAME).obj $(OBJS) $(LIBS)
        $(LN) $(NAME).obj $(OBJS)

.c.obj:
    $(CC) $<

.asm.obj:
    $(AS) $<

