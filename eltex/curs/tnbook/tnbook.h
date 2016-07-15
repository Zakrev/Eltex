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
#define MFILE struct mOpenFile
#define MEDITW struct mEditWin
#define SIZE_CONST 8
MCURSOR {
        int row;
        int col;
};

MFILE {
        int filed;//оригинальный файл
        int tmpd;//рабочий файл
        off_t first_line;//Начало первой линии входящей в редактор
        off_t last_line;//конец последней линии также
        off_t curs_pos;//текущая позиция курсора (в обоих файлах)
        off_t copy_pos;//Последний скопированный символ (чтоб не переписывать рабочий файл)
};

MEDITW {
        WINDOW *mbox;
        WINDOW *text;
        MCURSOR cursor;
        MFILE file;
        int row;//размер в строках
        int col;//размер в столбцах
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