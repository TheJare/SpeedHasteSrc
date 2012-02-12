

WOPTIONS=-d1 -fp3 -mf -5r -oneatxl -w9 -zp4 -zq -DNDEBUG
#WOPTIONS=-d2 -fp3 -mf -5r -s -w9 -zp4 -zq
LIB=wlib -c -q
CC=wcc386 $(WOPTIONS)
AS=tasm -l -m9 -ml -t -w2 -z -zi -i$(WATCOM)\LOCAL\H  -jJUMPS
#LN=wcl386 -d2 -l=dos4g -fm -k64000 -fe=speed
LN=wcl386 -l=dos4g -fm -k64000 -fe=CR
#LN=wcl386 -d2 -l=pmodew  -k64000 -fe=speed

LIBS=$(WATCOM)\lib386\local.lib speedh.lib

speed.exe: fl2.obj $(LIBS)
        $(LN) fl2.obj speedh.lib

speedh.lib: 3dfloor.obj  \
            cars.obj     \
            credits.obj  \
            config.obj   \
            fire.obj     \
            fla.obj      \
            flsprs.obj   \
            globals.obj  \
            hud.obj      \
            intro.obj    \
            menus.obj    \
            models.obj   \
            net.obj      \
            race.obj     \
            racemap.obj  \
            racers.obj   \
            records.obj  \
            sectors.obj  \
            things.obj   \
            userctl.obj

fl2.obj: fl2.c 3dfloor.h racemap.h cars.h flsprs.h
    $(CC) fl2.c

.c.obj:
    $(CC) $<
    $(LIB) speedh.lib -+$@

.asm.obj:
    $(AS) $<
    $(LIB) speedh.lib -+$@


cars.obj:    cars.c    cars.h globals.h

fl2.obj:     fl2.c     3dfloor.h globals.h userctl.h blueprin.c

things.obj:  things.c  things.h globals.h cars.h userctl.h

flsprs.obj:  flsprs.c  flsprs.h globals.h

globals.obj: globals.c globals.h

3dfloor.obj: 3dfloor.c 3dfloor.h

userctl.obj: userctl.c userctl.h globals.h

racemap.obj: racemap.c racemap.h

cars.h: things.h

things.h: flsprs.h

flsprs.h: 3dfloor.h

globals.h: cars.h racemap.h

racemap.h: things.h

userctl.h: things.h

