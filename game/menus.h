// ------------------------ MENUS.H ------------------
// All menu screens.

#ifndef _MENUS_H_
#define _MENUS_H_

#include <text.h>

    // Init things neccesary to run the menus.
    // Optionally load a background screen if != NULL, or copy the screen
    // from 'copy'.
PUBLIC bool MENU_Init(const char *fback, const byte *copy);

    // End things neccesary to run the menus.
PUBLIC void MENU_End(void);

//PUBLIC void MENU_MainMenu(void);

// ---------------------

PUBLIC char MENU_SongName[8+1+3+1];

PUBLIC void MENU_InitMusic(const char *fname);

PUBLIC void MENU_DoneMusic(void);

PUBLIC byte MENU_DoControlMenu(const char *menu[], int nopts, int *opt);

PUBLIC bool MENU_DoInput(const char *msg, char *buf, int maxlen, const char *charset);

// ---------------------

PUBLIC byte *MENU_Back;
PUBLIC FONT_TFont MENU_FontRed, MENU_FontYellow, MENU_FontWhite;

enum {
   MENU_MAIN,

   MENU_MAINDEMO,

   MENU_SOUNDCONFIG,
   MENU_DETAILCONFIG,

   MENU_CHOOSECIRC,
   MENU_CHOOSECAR,

   MENU_CONFIG,
   MENU_RACETYPE,

   MENU_CONTROLCONFIG,

   MENU_TIMEORLAPS,

   MENU_JUKEBOX,

   MENU_CARTYPE,

   MENU_SHAREWARE,

   MENU_NETOPT,
   MENU_IPXOPT,
   MENU_SERIALOPT,
   MENU_MODEMOPT,

   MENU_NETGAME,
   MENU_NETTYPE,
   MENU_PHONES,
   MENU_FINDPLAYERS,

};

PUBLIC void MENU_Enter(int nmenu);

PUBLIC void MENU_Switch(int nmenu);

PUBLIC void MENU_Return(void);

PUBLIC int MENU_Run(void);

// -------- Xapuzas varias

PUBLIC int MENU_SelCarNumPlayer;


#endif

// ------------------------ MENUS.H ------------------


