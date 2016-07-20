#include "tfm.h"

int main()
{
        TREE_WIN twin_left;
        TREE_WIN twin_right;
        TREE_WIN *now_win;
        int key;
        int b_l_row;
        
        initscr();
        keypad(stdscr, 1);
        noecho();
        refresh();
        b_l_row = InitWindowTree(&twin_left, 0, 0);
        CreateDInfoList(&twin_left);
        PrintDInfoList(&twin_left, 1);
        InitWindowTree(&twin_right, b_l_row, 0);
        CreateDInfoList(&twin_right);
        PrintDInfoList(&twin_right, 1);
        now_win = &twin_left;
        MoveCursorToRowCol(now_win, 0, 0);
        
        while( (key = getch()) > 0 ){
                if(key == KEY_F(10))
                        break;
                if(key == KEY_DOWN)
                        NextLine(now_win);
                if(key == KEY_UP)
                        PrevLine(now_win);
                if(key == 10)//ENTER
                        OnPressEnter(now_win);
                if(key == 9){//TAB
                        now_win = SwitchWin(now_win, &twin_left, &twin_right);
                        MoveCursorToRowCol(now_win, now_win->cursor.x_row, now_win->cursor.y_col);
                }
        }
        endwin();
        return 0;
}

TREE_WIN *SwitchWin(TREE_WIN *now, TREE_WIN *left, TREE_WIN *right)
{
        if(now == left)
                return right;
        else
                return left;
}

void GetWinSize(int *row, int *col)
{
        struct winsize size;
        ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
        *col = size.ws_col;
        *row = size.ws_row;
}

int CheckDname(char *str)
{
        if(str[0] != '.')
                return 0;
        if(str[1] == '\0')
                return -1;
        if(str[1] == '.' && str[2] == '\0')
                return -1;
                
        return 0;
}

int InitWindowTree(TREE_WIN *win, int x_col, int y_row)
{
        int w_col, w_row;
        int b_col, b_row;
        int nl_col, nl_row;
        char *label;
        int i;
        
        GetWinSize(&w_row, &w_col);
        b_col = w_col / 2;
        b_row = w_row / 2;
        nl_col = b_col - b_col / 4;
        nl_row = b_row - 3;
        win->bord = newwin(b_row, b_col, y_row, x_col);
        
        win->label = derwin(win->bord, 1, b_col - 4, 1, 3);
        label = malloc(b_col - 3);
        if(label != NULL){
                i = b_col - 5;
                while( i >= 0){
                        label[i] = ' ';
                        i--;
                }
                label[b_col - 4] = '\0';
                strncpy(label, "Name", 4);
                strncpy(label + nl_col, "Bytes", 4);
                
                wprintw(win->label, label);
                wrefresh(win->label);
                
                free(label);
        }
        
        win->nl_box = derwin(win->bord, nl_row, nl_col, 2, 0);
        box(win->nl_box, ' ', '.');
        wrefresh(win->nl_box);
        win->name_list = derwin(win->nl_box, nl_row - 2, nl_col - 2, 1, 1);
        win->nl_rows = nl_row - 3;
        
        win->il_box = derwin(win->bord, nl_row, b_col - nl_col - 1, 2, nl_col + 1);
        box(win->il_box, ' ', '.');
        wrefresh(win->il_box);
        win->info_list = derwin(win->il_box, nl_row - 2, b_col - nl_col - 3, 1, 1);
        
        box(win->bord, '|', '-');
        wrefresh(win->bord);

        win->name_list_d = NULL;
        win->name_list_d_size = 0;
        win->name_index = 1;

        win->cursor.x_row = win->cursor.y_col = 0;

        return b_col;
}

void ClearDInfoList(TREE_WIN *twin)
{
        int i;
        
        if(twin->name_list_d == NULL)
                return;
        
        for(i = 0; i < twin->name_list_d_size; i++){
                free(twin->name_list_d[i]);
        }
        free(twin->name_list_d);
        twin->name_list_d = NULL;
        twin->name_index = 1;
}

int CreateDInfoList(TREE_WIN *twin)
{
        struct dirent **name_list;
        int size;
        
        ClearDInfoList(twin);
        size = scandir(".", &name_list, 0, alphasort);
        if(size < 0)
                return -1;
        twin->name_list_d_size = size;
        twin->name_list_d = name_list;
        
        return 0;
}

void MakeName(char *str, struct stat sb, char *name)
{
        switch(sb.st_mode & S_IFMT){
                case S_IFDIR:
                        sprintf(str, "/ %s", name);
                break;
                default:
                        if(sb.st_mode & S_IXUSR || sb.st_mode & S_IXGRP || sb.st_mode & S_IXOTH)
                                sprintf(str, "* %s", name);
                        else
                                sprintf(str, "  %s", name);
                break;
        }
}

void PrintDInfoList(TREE_WIN *twin, int start_index)
{
        int i, j;
        struct dirent **name_list = twin->name_list_d;
        struct stat stat_buff;
        char tmp_str[25];
        
        werase(twin->name_list);
        werase(twin->info_list);
        
        if(name_list == NULL)
                return;
                
        for(i = start_index, j = 0; i < twin->name_list_d_size && j <= twin->nl_rows; i++, j++){
                if(stat(name_list[i]->d_name, &stat_buff) > -1){
                        wmove(twin->name_list, j, 0);
                        MakeName(tmp_str, stat_buff, name_list[i]->d_name);
                        waddstr(twin->name_list, tmp_str);
                        sprintf(tmp_str, "%d", (int)stat_buff.st_size);
                        wmove(twin->info_list, j, 0);
                        waddstr(twin->info_list, tmp_str);
                } else {
                        wmove(twin->name_list, j, 0);
                        waddstr(twin->name_list, name_list[i]->d_name);
                }
        }
        wrefresh(twin->name_list);
        wrefresh(twin->info_list);
        MoveCursorToRowCol(twin, twin->cursor.x_row, twin->cursor.y_col);
}

int MoveCursorToRowCol(TREE_WIN *twin, int row, int col)
{
        if( wmove(twin->name_list, col, row) > -1 ){
                twin->cursor.x_row = row;
                twin->cursor.y_col = col;
                wrefresh(twin->name_list);
                return 0;
        }
        
        return -1;
}

int NextLine(TREE_WIN *twin)
{
        int cursor_pos = twin->nl_rows - twin->cursor.y_col;

        if( (twin->name_index + 1) < twin->name_list_d_size){
                twin->name_index++;
                if(cursor_pos == 0)
                        PrintDInfoList(twin, twin->name_index - twin->cursor.y_col);
        } else
                return -1;
        return MoveCursorToRowCol(twin, twin->cursor.x_row, twin->cursor.y_col + 1);
}

int PrevLine(TREE_WIN *twin)
{
        if( (twin->name_index - 1) > 0){
                twin->name_index--;
                if(twin->cursor.y_col == 0)
                        PrintDInfoList(twin, twin->name_index);
        } else
                return -1;
        
        return MoveCursorToRowCol(twin, twin->cursor.x_row, twin->cursor.y_col - 1);
}

int OnPressEnter(TREE_WIN *twin)
{
        struct dirent **name_list = twin->name_list_d;
        struct stat stat_buff;
        int name_index = twin->name_index;
        
        if(name_list == NULL)
                return 0;
                
        if(stat(name_list[name_index]->d_name, &stat_buff) > -1){
                switch(stat_buff.st_mode & S_IFMT){
                        case S_IFDIR:
                                if( chdir(name_list[name_index]->d_name) < 0 )
                                        return -1;
                                CreateDInfoList(twin);
                                PrintDInfoList(twin, 1);
                                MoveCursorToRowCol(twin, 0, 0);
                                
                                return 0;
                        break;
                        default:
                                if(stat_buff.st_mode & S_IXUSR || stat_buff.st_mode & S_IXGRP || stat_buff.st_mode & S_IXOTH){
                                        StartFile(name_list[name_index]->d_name);
                                        initscr();
                                        PrintDInfoList(twin, 1);
                                } else {
                                       EditFile(name_list[name_index]->d_name);
                                       initscr();
                                       PrintDInfoList(twin, 1);
                                }
                        break;
                }
        } else {
                return -1;
        }
        
        return 0;
}

int StartFile(char *fname)
{
        pid_t pid;
        
        pid = fork();
        
        if(pid == 0){
                execl(fname, fname, (char*)NULL);
        } else {
                endwin();
                wait(0);
        }
        
        return 0;
}

int EditFile(char *fname)
{
        pid_t pid;
        
        pid = fork();
        
        if(pid == 0){
                execlp("mc", "mc", "-e", fname, (char*)NULL);
        } else {
                endwin();
                wait(0);
        }
        
        return 0;
}