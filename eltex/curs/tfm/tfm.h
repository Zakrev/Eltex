#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <ncurses.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#define TREE_WIN struct WindowTree
#define MCURS struct mCursor

MCURS {
        off_t x_row;
        off_t y_col;
};

TREE_WIN {
        WINDOW *bord;
        WINDOW *name_list;
        WINDOW *nl_box;
        WINDOW *info_list;
        WINDOW *il_box;
        WINDOW *label;
        MCURS cursor;
        off_t nl_rows;
        int name_index;
        int name_list_d_size;
        struct dirent **name_list_d;
};

void GetWinSize(int *, int *);
int CheckDname(char *);
int InitWindowTree(TREE_WIN *, int, int);
int CreateDInfoList(TREE_WIN *);
void ClearDInfoList(TREE_WIN *);
void PrintDInfoList(TREE_WIN *, int);
int MoveCursorToRowCol(TREE_WIN *, int, int);
int NextLine(TREE_WIN *);
int PrevLine(TREE_WIN *);
int OnPressEnter(TREE_WIN *);
int StartFile(char *);
int EditFile(char *);
TREE_WIN *SwitchWin(TREE_WIN *, TREE_WIN *, TREE_WIN *);