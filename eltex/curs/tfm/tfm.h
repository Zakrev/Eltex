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
#include <pthread.h>
#include <fcntl.h>

#define TREE_WIN struct WindowTree
#define MCURS struct mCursor
#define PBAR_WIN struct mProgressBar

#define CURDIR_SIZE 256
/*
        row - y - height
        col - x - width
*/

MCURS {
        off_t row;
        off_t col;
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
        char cur_dir[CURDIR_SIZE];
        struct dirent **name_list_d;
};

PBAR_WIN {
        WINDOW *box;
        WINDOW *text;
        WINDOW *progress;
        int progress_size;
};

struct mCopyInfo {
        TREE_WIN **twin;
        char from[CURDIR_SIZE];
        char to[CURDIR_SIZE];
        int from_flags;
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
PBAR_WIN *CreatePBAR(TREE_WIN *, int, int, char *);
void SetProgressPercent(TREE_WIN *, PBAR_WIN *, int);
int ChDir(TREE_WIN *, char *);
int ChDir_NotMove(TREE_WIN *, char *);
void *CopyFile(void *);
int InitCopyFile(TREE_WIN **, TREE_WIN *);
int FileIsRead(struct stat *);
int FileIsWrite(struct stat *);
int FileIsExe(struct stat *);