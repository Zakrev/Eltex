#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <ncurses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <locale.h>

#define MCURSOR struct mCursor
#define EDITWIN struct mEditWindow
#define EFILE struct mEditFile
#define MENUWIN struct mMenuWindow
#define SIZEN 8

/*
        row - y - height
        col - x - width
*/

MCURSOR {
        int x;
        int y;
};

EFILE {
        int filed;
        int tmpd;
        char tmp_name[256];
        off_t file_pos;
        off_t tmp_pos;
        off_t first_ch;
        off_t last_ch;
};

EDITWIN {
        WINDOW *box;
        WINDOW *text;
        int text_h;
        int text_w;
        int buff_index;
        char *buff_text;
        MCURSOR cursor;
        EFILE file;
};

MENUWIN {
        WINDOW *box;
        WINDOW *text;
};

void GetWinSize(int *, int *);
EDITWIN *InitEDITWIN();
void DeleteEDITWIN(EDITWIN *);
int InitMENUWIN(MENUWIN *);
int OpenFile(EDITWIN *, char *);
int ReadFile(EDITWIN *);
int PrintBuff(EDITWIN *);
int ScrollDown(EDITWIN *);
int ScrollUp(EDITWIN *);
int CloseFile(EDITWIN *);