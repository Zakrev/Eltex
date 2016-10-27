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
#include <signal.h>

#define TREE_WIN struct WindowTree
#define MCURS struct mCursor
#define PBAR_WIN struct mProgressBar

#define CURDIR_SIZE 256
#define MAX_PARAM 4 //Максимальное количество параметров передаваемое программе
#define MAX_PARAM_LENGHT 100
#define INOUT "inout.tfm"
/*
        row - y - height
        col - x - width
*/
/*
        MCURS - структура, хранит информацию о положении указателя
*/
MCURS {
        off_t row;
        off_t col;
};
/*
        TREE_WIN - структура, хранит информацию об окне в котором отображается
                дерево файлов
*/
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

/*
        PBAR_WIN - структура, хранит информацию об окне в котором отображается
                прогресс (копирования)
*/
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

/*

*/
#define CLINE struct mComandLine
#define CLINE_LENGHT 512
#define ARG_LENGHT 5+2
#define ARG_U_SIZE 50

CLINE {
        WINDOW *box;
        WINDOW *text;
        char buff[CLINE_LENGHT];
        int buf_pos;
        MCURS cursor;
};

void OnExit();
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
int IsFileExtension(char *file_name);
int InitComandLine(CLINE *cline, int x_col, int y_row);
int MoveCursorToRowCol_CLINE(CLINE *cline, int row, int col);
int RunComandLine(CLINE *cline);
int MoveCursorToRight_CLINE(CLINE *cline);
int MoveCursorToLeft_CLINE(CLINE *cline);
int AddToClineBuffer(CLINE *cline, char c);
int EraseFromClineBuffer(CLINE *cline);
int OnPressEnter_CLINE(CLINE *cline);
int GetArgements(char *arg[ARG_LENGHT], char tmp_arg[ARG_LENGHT][ARG_U_SIZE], char *str, int pos);
int ExecuteLine(char *str);
