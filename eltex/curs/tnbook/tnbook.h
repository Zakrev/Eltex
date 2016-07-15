#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <error.h>
#include <ncurses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>

#define MCURSOR struct mCursor
#define MC_TO_LEFT 1
#define MC_TO_RIGHT 2
#define MC_TO_UP 3
#define MC_TO_DOWN 4

#define MEDITW struct mEditWin
#define SIZE_CONST 8
MCURSOR {
        int row;
        int col;
};

MEDITW {
        WINDOW *mbox;
        WINDOW *text;
        MCURSOR cursor;
        int row;
        int col;
        int filed;
        int fpos;
        char *buff;
};

void GetWinSize(int *, int *);
void CreateDataBox(MEDITW *);
void CreateDataText(MEDITW *);
void DeleteDataText(MEDITW *);
WINDOW *DrawMMenuBox();
WINDOW *DrawMMenuText(WINDOW *, char *);
int MoveCursorToRowCol(MEDITW *, int , int);
int MoveCursorTo(MEDITW *, int);
int PrintNewLine(MEDITW *);
int PrintCh(MEDITW *, char);