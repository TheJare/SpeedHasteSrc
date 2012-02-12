cc
for %i in (*.exe) wstrip -q %i
for %i in (util\*.exe) wstrip -q %i
for %i in (paths\*.exe) wstrip -q %i
move fl2.arj speedh.arj i:\back
arj a fl2 makefile *.c *.h *.inc *.asm util\*.c util\*.asm util\*.h util\*.inc *.bat crypt\*.c paths\*.c
arj a fl2 maps\*.c maps\*.bat maps\*.pro
arj a speedh util\*.exe util\*.c *.exe grafs.* *.dat *.clr *.pth paths\*.exe
arj a speedh maps\*.exe maps\*.bat maps\*.pro
rc
