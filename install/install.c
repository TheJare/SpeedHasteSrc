/*
    -------------------------------------
    WARNING:

    Watcom 10.5's stat() when applied to the root directory d:\ of a drive
    different from the current drive, changes that drive's current dir
    to root. This forces some weird ways to do things here. And possibly
    changes the current drive to that drive too.

    So, NEVER stat() to a root directory unless there's no chance.

*/

#include <sys/stat.h>
#include <stdio.h>
#include <dos.h>
#include <direct.h>
#include <ctype.h>
#include <conio.h>

#include "textscr.h"

//#define DO_LOG

#ifdef DO_LOG
#define LOG(a) (a)
#else
#define LOG(a) ((void)(0))
#endif

// --------------------
// My state variables.

#define PROGRAM_NAME "CIRCUIT RACER"

PRIVATE char DestDir[300] = "C:\\CR";
PRIVATE char SrcDir[300];

#define MEGS_REQUIRED (10UL*1024*1024)

// ---------------------------------------
// Pathname, directory and file utility functions.

    // Removes multiple backslashes from a pathname if any, and the last
    // backslash if it doesn't denote the root (e.g. "\\" or "F:\\").
    // Overwrites input and also returns it.
PRIVATE char *CleanupPath(char *dest, const char *path) {
    char *d, *p;

    p = path;
    d = dest;
    for (; *p != '\0'; p++) {
        if (*p != '\\' || p[1] != '\\')
            *d++ = *p;
    }
    do {
        d--;
    } while (*d == '\\' && d > dest && d[-1] != ':');
    d[1] = '\0';
    return dest;
}

    // Turns the given path into a full-featured pathname.
PRIVATE char *CompletePath(char *dest, const char *path) {
    char buf[300];
    char cwd[300];
    char *p, *d;
    unsigned total;

    LOG(printf("CompletePath of %s\n", path));
    getcwd(cwd, sizeof(cwd));
    LOG(printf("cwd => %s\n", cwd));
    p = path;
    d = buf;
    if (*p == '\0' || p[1] != ':') {
        *d++ = cwd[0];
        *d++ = cwd[1];
    } else {
        *d++ = *p++;    // Drive
        *d++ = *p++;    // ':'
    }
    LOG(printf("drive => %2.2s\n", buf));
    _dos_setdrive(buf[0] - 'A' + 1, &total);
/*
    sprintf(cwd, "%c:.", buf[0]);
    LOG(printf("chdir() to \"%s\"\n", cwd));
    printf("chdir == %d\n", chdir(cwd));
    cwd[0] = '\0';
*/
    getcwd(cwd, sizeof(cwd));   // Find cur dir of selected drive.
    if (cwd[0] == buf[0]) {      // If drive exists
        LOG(printf("Drive exists, cwd => %s\n", cwd));
        if (*p != '\\') {
            strcpy(d, cwd+2);
            d += strlen(d);
            LOG(printf("path => %s\n", buf));
        }
        getcwd(cwd, sizeof(cwd));   // Find cur dir of selected drive.
        LOG(printf("Cur drive now is => %s\n", cwd));
    } else
        LOG(printf("Drive does not exist, cwd is still \"%s\".\n", cwd));
    sprintf(d, "\\%s", p);
    d += strlen(d);
    CleanupPath(dest, buf);
    return dest;
}

PRIVATE char *GetExistingDrives(char *dest) {
    char buf[40];
    struct stat statbuf;
    int i;
    char *p;

    p = dest;
    for (i = 'C'; i <= 'Z'; i++) {
        sprintf(buf, "%c:.", i);            // Try current directory.
	    if (stat(buf, &statbuf) != 0)
            sprintf(buf, "%c:\\", i);       // Try root directory.
	    if (stat(buf, &statbuf) == 0)
            *p++ = i;
    }
    *p = '\0';
    return dest;
}

    // A == 1, B == 2, etc
PRIVATE unsigned long GetDriveFree(unsigned drive) {
    unsigned long df;
    struct _diskfree_t diskspace;

    if (_dos_getdiskfree(drive, &diskspace) != 0)
        return 0;
    df = (unsigned long)(diskspace.avail_clusters)*
         (unsigned long)(diskspace.sectors_per_cluster)*
         (unsigned long)(diskspace.bytes_per_sector);
    return df;
}

    // TRUE if directory is created.
PRIVATE bool CreateMultiDirectory(char *path) {
    char buf[300];
    char buf2[300];
    char *p;
    struct stat statbuf;

    strcpy(buf2, path);     // Save copy of path in buf2
        // Clean up string if required.
    LOG(printf("Before cleanup: \"%s\"\n", buf2));
    CleanupPath(buf2, buf2);

    LOG(printf("After cleanup: \"%s\"\n", buf2));

    p = buf;
    do {
        strcpy(buf, buf2);
        p = strchr(p, '\\');
        if (p != NULL) {
            LOG(printf("Found p: \"%s\"\n", p));
            if (p > buf && p[-1] != ':')
                *p = '\0';
            else {
//                p[1] = '\0';
                p++;
                continue;
            }
        }
        LOG(printf("Stat()'ing: \"%s\"\n", buf));
	    if (stat(buf, &statbuf) < 0) {
                // Create dir...
            LOG(printf("Mkdir()'ing: \"%s\"\n", buf));
            mkdir(buf);
                // Check again.
        	if (stat(buf, &statbuf) < 0) {
                LOG(printf("Couldn't create: \"%s\"\n", buf));
                return FALSE;
            }
        }
        if (p != NULL)
            p++;
    } while (p != NULL);
    return TRUE;
}

// -------------------------------------------------------------------
// Other functions.

PRIVATE bool CopyFile(const char *file, int bary, int barx, int barw) {
    char destf[300];
    char srcf[300];
    static char buf[8192];
//    static char buf[1];
    FILE *fi = NULL, *fo = NULL;
    long len, b;

    TXS_DrawString(barx, barw, bary-2, file, CDLGTIT, TXS_STCENTER);
    TXS_FillBox(barx, bary, barx+barw-1, bary, '°', 0x07);

    sprintf(destf, "%s\\%s", DestDir, file);
    CleanupPath(destf, destf);
    sprintf(srcf, "%s\\%s", SrcDir, file);
    CleanupPath(srcf, srcf);
    fi = fopen(srcf, "rb");
    if (fi == NULL)
        goto error;
    fo = fopen(destf, "wb");
    if (fo == NULL)
        goto error;
    fseek(fi, 0, SEEK_END);
    len = ftell(fi);
    fseek(fi, 0, SEEK_SET);
    b = len;
    while (b > 0) {
        long n;

        n = b;
        if (n > sizeof(buf))
            n = sizeof(buf);
        if (fread(buf, n, 1, fi) != 1)
            goto error;
        if (fwrite(buf, n, 1, fo) != 1)
            goto error;
        b -= n;
        TXS_FillBox(barx, bary, barx+(len-b)*barw/len-1, bary, 'Û', 0x0F);
        TXS_FillBox(barx+(len-b)*barw/len, bary, barx+barw-1, bary, '°', 0x07);
    }
    fclose(fi);
    fclose(fo);
    return TRUE;

  error:
    if (fi != NULL)
        fclose(fi);
    if (fo != NULL)
        fclose(fo);
    return FALSE;
}

// -------------------------------------------------------------------
// Menu functions themselves.

PRIVATE bool InstalledOK = FALSE;

PRIVATE bool DoInstall(void) {
    static const char *files[] = {
        "DOS4GW.EXE",
        "CR.EXE",
        "SETUP.EXE",
        "CR.JCL"
    };
    int i;

    TXS_DrawShadedBox(3, 7, 75, 11, TXS_DoubleFrame " ", CDLGF, CDLGI);
    for (i = 0; i < SIZEARRAY(files); i++) {
        if (!CopyFile(files[i], 10, 4, 71)) {
            return FALSE;
        }
    }
    InstalledOK = TRUE;
    {
        struct find_t findt;
        unsigned rez;

        {
            char buf[300];
            sprintf(buf, "%s\\%s", SrcDir, "*.DOC");
            CleanupPath(buf, buf);
            rez = _dos_findfirst(buf, _A_NORMAL, &findt);
        }
        while (rez == 0) {
            if (!CopyFile(findt.name, 10, 4, 71)) {
                InstalledOK = FALSE;
                break;
            }
            rez = _dos_findnext(&findt);
        }
        _dos_findclose(&findt);
    }
    return InstalledOK;
}

PRIVATE void ShowConfig(void) {
    int y = 18;
    TXS_DrawShadedBox(1, y-3, 75, y+4, TXS_DoubleFrame " ", CDLGF, CDLGI);
    TXS_DrawBox(1, y-1, 75, y+4, TXS_DownFrame, CDLGF, CDLGI);
    TXS_DrawString(2, 74, y-2, "Current Settings", CDLGTIT, TXS_STCENTER);
    TXS_DrawString(2, 74, y++, " Source directory:", CDLGT, TXS_STLJUST);
    TXS_DrawString(4, 74, y++, SrcDir, CDLGTIT, TXS_STLJUST);
    TXS_DrawString(2, 74, y++, " Destination directory:", CDLGT, TXS_STLJUST);
    TXS_DrawString(4, 74, y++, DestDir, CDLGTIT, TXS_STLJUST);
}

PRIVATE void ChooseDestination(void) {
    char buf[300];
    bool rez;
    int i, y;
    void *back;

    back = TXS_SaveBox(0, 0, 79, 24);

    TXS_DrawBox(50, 3, 78, 12, TXS_DoubleFrame " ", CDLGF, CDLGI);
    TXS_DrawString(51, 77, 4, "Allowed disk drives:", CDLGTIT, TXS_STLJUST);
    GetExistingDrives(buf);
    y = 5;
    for (i = 0; buf[i] != '\0' && y < 12; i++) {
        if (GetDriveFree(buf[i] - 'A' + 1)  >= MEGS_REQUIRED) {
            char d[100];
            sprintf(d, "%c: %4ld Megabytes free", buf[i], GetDriveFree(buf[i] - 'A' + 1)/1024/1024);
            TXS_DrawString(52, 77, y, d, CDLGI, TXS_STLJUST);
            y++;
        }
    }
    for (;;) {
        strcpy(buf, DestDir);
        rez = TXS_InputString(2, 76, 12,
                             "Enter destination directory:",
                             buf, 76-2-2,
                             CDLGI);
        TXS_SetCursor(0, 0);
        if (rez) {
            CleanupPath(buf, buf);
            CompletePath(buf, buf);
            if (strlen(buf) <= 3) {
                TXS_DoDialog(25, 10, NULL, 0,
                    "Please specify a subdirectory|"
                    "Destination directory invalid|"
                    "Press ENTER@Enter directory again#"
                    );
                continue;
            }
            if (stricmp(buf, SrcDir) == 0) {
                TXS_DoDialog(25, 10, NULL, 0,
                    "Source and destination are the same|"
                    "Destination directory invalid|"
                    "Press ENTER@Enter directory again#"
                    );
                continue;
            }
            buf[0] = toupper(buf[0]);
            if (GetDriveFree(buf[0] - 'A' + 1) >= MEGS_REQUIRED) {
                strcpy(DestDir, buf);
                break;
            } else {
                TXS_DoDialog(25, 10, NULL, 0,
                    "Insufficient free space|"
                    "Destination drive invalid|"
                    "Press ENTER@Enter directory again#"
                    );
            }
        } else
            break;
    }
    TXS_RestoreBox(back);
}

PRIVATE void MainMenu(void) {
    static int mmopt = 0;
    do {
        int opt;
//        VGA_VSync();

        TXS_SetCursor(0, 0);
        ShowConfig();
        opt = TXS_DoDialog( 2,  4, &mmopt, TXS_DLGS_NOSAVE,
            "Main Menu|"
            "Select an option|"
            "Install!@Install the game in the selected directory#"
            "Set Destination Directory@Set the directory where the game will be installed#"
            "Abandon@Do not install the game and exit back to MS-DOS#"
            );
        if (opt == -1 || opt == 2) {
            break;
        } else if (opt == 1) {
            ChooseDestination();
        } else if (opt == 0) {
            void *back;

            back = TXS_SaveBox(0, 0, 79, 24);
            if (!CreateMultiDirectory(DestDir))
                TXS_DoDialog(25, 10, NULL, 0,
                    "ERROR|"
                    "Can't create destination directory|"
                    "Press ENTER@Go back to main menu#"
                    );
            InstalledOK = DoInstall();
            if (!InstalledOK)
                TXS_DoDialog(25, 10, NULL, 0,
                    "ERROR|"
                    "Error copying files|"
                    "Press ENTER@Go back to main menu#"
                    );
            TXS_RestoreBox(back);
            if (InstalledOK)
                break;
        }
    } while (TRUE);
}

// -------------------------------------------------------------------
// System functions.

PRIVATE void Usage(void) {
    printf(PROGRAM_NAME " Installation program (C) Copyright 1996 by Javier Ar‚valo.\n"
           "\n"
           "  Type \"INSTALL\", or \"INSTALL [source drive and directory]\",\n"
           "  without the double quotes, and press ENTER.\n"
           "\n"
           "For example, if your CD-ROM drive is D:, type\n"
           "INSTALL D:\\\n"
          );
    exit(1);
}

// -------------------------------------------------------------------

extern void SetMode(int mode);
#pragma aux SetMode parm [AX] = \
    "PUSHA" \
    "INT 0x10" \
    "POPA"

extern void FinishProgram(void) {
    SetMode(3);
}

static int __far HarderrHandler(unsigned __deverr,
                        unsigned __errcode,unsigned __far *__devhdr) {
    _hardresume(_HARDERR_FAIL);
    return 0;
}

main(int argc, char *argv[]) {
    ArgC = argc;
    ArgV = argv;

    _harderr(HarderrHandler);

    SetMode(3);
    if (ArgC > 1 && (ArgV[1][0] == '-' || ArgV[1][0] == '/'))
        Usage();
    {
        char *p;
        int l;

        strcpy(SrcDir, ArgV[0]);
        p = strrchr(SrcDir, '\\');
        if (ArgC > 1 || p == NULL || SrcDir[1] != ':') {
            if (ArgC == 1)
                Usage();
            if (ArgV[1][1] != ':' || ArgV[1][2] != '\\')
                Usage();
            strcpy(SrcDir, ArgV[1]);
        } else {
            *p = '\0';
        }
        l = strlen(SrcDir);
        if (l < 2)
            Usage();
    }

    SrcDir[0] = toupper(SrcDir[0]);
    CompletePath(SrcDir, SrcDir);

    DestDir[0] = toupper(DestDir[0]);
    {
        char buf[100];
        int i;

        GetExistingDrives(buf);
        for (i = 0; buf[i] != '\0'; i++) {
            if (GetDriveFree(buf[i] - 'A' + 1) >= MEGS_REQUIRED) {
                DestDir[0] = buf[0];
                break;
            }
        }
    }

//    printf("Source directory \"%s\", target \"%s\"\n", SrcDir, DestDir);

    TXS_FillBox(0, 0, 80, 25, '±', CBACK);
    TXS_FillBox(0, 0, 79, 1, ' ', 0x11);
    TXS_DrawString(1, 78, 0, "Circuit Racer installation program", CTOP, TXS_STCENTER);
    TXS_DrawString(1, 78, 1, "(C) Copyright 1996/97 by Javier Ar‚valo", CTOP, TXS_STCENTER);
    TXS_FillBox(0, 24, 79, 24, ' ', CDLGTIP);

    MainMenu();

    FinishProgram();
    if (InstalledOK) {
        unsigned total;
        _dos_setdrive(DestDir[0] - 'A' + 1, &total);
        chdir(DestDir);
        printf("Type CR to play Circuit Racer.\n");
    } else {
        unsigned total;
        _dos_setdrive(SrcDir[0] - 'A' + 1, &total);
        chdir(SrcDir);
        printf("Installation aborted. Type INSTALL to install Circuit Racer.\n");
    }
}
