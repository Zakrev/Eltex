#include "tfm.h"

int main()
{
        WINDOW *key_info;
        TREE_WIN twin_left;
        TREE_WIN twin_right;
        TREE_WIN *now_win;
        int key;
        int twin_left_width;
        
        initscr();
        keypad(stdscr, 1);
        noecho();
        refresh();
        key_info = derwin(stdscr, 0, 0, 0, 0);
        wprintw(key_info, " F10 - Exit    F5 - Copy selected file");
        wrefresh(key_info);
        twin_left_width = InitWindowTree(&twin_left, 0, 1);
        CreateDInfoList(&twin_left);
        PrintDInfoList(&twin_left, 1);
        InitWindowTree(&twin_right, twin_left_width, 1);
        CreateDInfoList(&twin_right);
        PrintDInfoList(&twin_right, 1);
        now_win = &twin_left;
        MoveCursorToRowCol(now_win, 0, 0);
        
        while( (key = getch()) > 0 ){
                if(key == KEY_F(10))
                        break;
                if(key == KEY_F(5)){
                        InitCopyFile(&now_win, now_win == &twin_left ? &twin_right : &twin_left);
                }
                if(key == KEY_DOWN)
                        NextLine(now_win);
                if(key == KEY_UP)
                        PrevLine(now_win);
                if(key == 10)//ENTER
                        OnPressEnter(now_win);
                if(key == 9){//TAB
                        now_win = SwitchWin(now_win, &twin_left, &twin_right);
                        ChDir_NotMove(now_win, now_win->cur_dir);
                        MoveCursorToRowCol(now_win, now_win->cursor.row, now_win->cursor.col);
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

        win->cursor.row = win->cursor.col = 0;

        getcwd(win->cur_dir, CURDIR_SIZE);

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

void MakeName(char *str, struct stat *sb, char *name)
{
        char r = ' ';
        char w = ' ';
        char x = ' ';
        
        if(sb == NULL){
                sprintf(str, "??? %s", name);//Неизвестный
                return;
        }
        
        if( (sb->st_mode & S_IFMT) == S_IFDIR ){//Директория
                sprintf(str, " /  %s", name);
                return;
        }
        //Разрешения 
        if(FileIsExe(sb) == 0){//Запуск
                x = 'x';
        }
        if(FileIsRead(sb) == 0){//Чтение
                r = 'r';
        }
        if(FileIsWrite(sb) == 0){//Запись
                w = 'w';
        }
        
        sprintf(str, "%c%c%c %s", r, w, x, name);
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
                        MakeName(tmp_str, &stat_buff, name_list[i]->d_name);
                        waddstr(twin->name_list, tmp_str);
                        sprintf(tmp_str, "%d", (int)stat_buff.st_size);
                        wmove(twin->info_list, j, 0);
                        waddstr(twin->info_list, tmp_str);
                } else {
                        wmove(twin->name_list, j, 0);
                        MakeName(tmp_str, NULL, name_list[i]->d_name);
                        waddstr(twin->name_list, tmp_str);
                }
        }
        wrefresh(twin->name_list);
        wrefresh(twin->info_list);
        MoveCursorToRowCol(twin, twin->cursor.row, twin->cursor.col);
}

int MoveCursorToRowCol(TREE_WIN *twin, int row, int col)
{
        if( wmove(twin->name_list, col, row) > -1 ){
                twin->cursor.row = row;
                twin->cursor.col = col;
                wrefresh(twin->name_list);
                return 0;
        }
        
        return -1;
}

int NextLine(TREE_WIN *twin)
{
        int cursor_pos = twin->nl_rows - twin->cursor.col;

        if( (twin->name_index + 1) < twin->name_list_d_size){
                twin->name_index++;
                if(cursor_pos == 0)
                        PrintDInfoList(twin, twin->name_index - twin->cursor.col);
        } else
                return -1;
        return MoveCursorToRowCol(twin, twin->cursor.row, twin->cursor.col + 1);
}

int PrevLine(TREE_WIN *twin)
{
        if( (twin->name_index - 1) > 0){
                twin->name_index--;
                if(twin->cursor.col == 0)
                        PrintDInfoList(twin, twin->name_index);
        } else
                return -1;
        
        return MoveCursorToRowCol(twin, twin->cursor.row, twin->cursor.col - 1);
}

int ChDir(TREE_WIN *twin, char *dname)
{
        if(chdir(dname) < 0)
                return -1;
        CreateDInfoList(twin);
        PrintDInfoList(twin, 1);
        MoveCursorToRowCol(twin, 0, 0);
        getcwd(twin->cur_dir, CURDIR_SIZE);
        
        return 0;
}

int ChDir_NotMove(TREE_WIN *twin, char *dname)
{
        if(chdir(dname) < 0)
                return -1;
        getcwd(twin->cur_dir, CURDIR_SIZE);
        
        return 0;
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
                                return ChDir(twin, name_list[name_index]->d_name);
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

PBAR_WIN *CreatePBAR(TREE_WIN *twin, int x, int y, char *text)
{
        PBAR_WIN *pbar;
        int height, win_h;
        int width, win_w;
        
        GetWinSize(&win_h, &win_w);
        pbar = malloc(sizeof(PBAR_WIN));
        if(pbar == NULL)
                return NULL;
        height = win_h / 4;
        width = win_w - 2;
        if(x < 0 || y < 0){
                x = 1;
                y = win_h - height;
        }
        pbar->box = newwin(height, width, y, x);
        box(pbar->box, ' ', '-');
        wrefresh(pbar->box);
        pbar->text = derwin(pbar->box, height - 3, width - 2, 1, 1);
        if(text != NULL)
                waddstr(pbar->text, text);
        wrefresh(pbar->text);
        pbar->progress = derwin(pbar->box, 1, width - 2, height - 2, 1);
        wrefresh(pbar->progress);
        pbar->progress_size = width - 2;
        MoveCursorToRowCol(twin, twin->cursor.row, twin->cursor.col);

        return pbar;
}

void DeletePBAR(PBAR_WIN *pbar)
{
        wclear(pbar->box);
        wrefresh(pbar->box);
        delwin(pbar->progress);
        delwin(pbar->text);
        delwin(pbar->box);
        free(pbar);
}

void SetProgressPercent(TREE_WIN *twin, PBAR_WIN *pbar, int val)
{
        int step = (val * pbar->progress_size) / 100;
        
        werase(pbar->progress);
        while(step-- > 0){
                waddch(pbar->progress, '#');
        }
        wrefresh(pbar->progress);
        MoveCursorToRowCol(twin, twin->cursor.row, twin->cursor.col);
}

void *CopyFile(void *arg)
{
        struct mCopyInfo *mci = (struct mCopyInfo *)arg;
        char pb_message[CURDIR_SIZE * 2];
        int to_d;
        int from_d;
        char copy_buff[64];
        off_t size;
        int copyed = 0;
        int copyed_byte;
        
        PBAR_WIN *pbar;
        
        from_d = open(mci->from, O_RDONLY);
        if(from_d < 0){
                /*
                        PRINT ERROR
                */
                pthread_exit(0);
        }
        to_d = open(mci->to, O_CREAT | O_RDWR, mci->from_flags);
        if(to_d < 0){
                /*
                        PRINT ERROR
                */
                close(from_d);
                pthread_exit(0);
        }
        size = lseek(from_d, 0, SEEK_END);
        lseek(from_d, 0, SEEK_SET);
        sprintf(pb_message, "%s\ncopy to\n%s", mci->from, mci->to);
        pbar = CreatePBAR(*mci->twin, -1, -1, pb_message);
        while( (copyed_byte = read(from_d, copy_buff, 64)) > 0){
                write(to_d, copy_buff, copyed_byte);
                copyed += copyed_byte;
                SetProgressPercent(*mci->twin, pbar, (copyed * 100) / size);
                sleep(1);//Ожидание для наглядности
        }
        close(from_d);
        close(to_d);
        DeletePBAR(pbar);
        free(mci);
        pthread_exit(0);
}

int CopyFlags(char *fname)
{
        struct stat fstat;

        if( stat(fname, &fstat) > -1 ){
                return fstat.st_mode;
        }

        return S_IRWXU;
}

int InitCopyFile(TREE_WIN **from_win, TREE_WIN *to_win)
{
        pthread_t pth;
        struct mCopyInfo *mci;
        struct dirent *name_list = (*from_win)->name_list_d[(*from_win)->name_index];
        struct stat from_stat;
        
        mci = malloc(sizeof(struct mCopyInfo));
        if(mci == NULL)
                return -1;
        mci->twin = from_win;
        sprintf(mci->from, "%s/%s", (*from_win)->cur_dir, name_list->d_name);
        sprintf(mci->to, "%s/%s", to_win->cur_dir, name_list->d_name);
        if(stat(mci->from, &from_stat) < -1){//Непонятный файл
                free(mci);
                return -1;
        }
        if( (from_stat.st_mode & S_IFMT) == S_IFDIR ){//Директория
                free(mci);
                return -1;
        }
        mci->from_flags = from_stat.st_mode;
        pthread_create(&pth, NULL, CopyFile, mci);
        
        return 0;
}

int FileIsRead(struct stat *sf)
{
        if( sf->st_mode & S_IRUSR || sf->st_mode & S_IRGRP || sf->st_mode & S_IROTH )
                return 0;
        
        return -1;
}

int FileIsWrite(struct stat *sf)
{
        if( sf->st_mode & S_IWUSR || sf->st_mode & S_IWGRP || sf->st_mode & S_IWOTH )
                return 0;
        
        return -1;
}

int FileIsExe(struct stat *sf)
{
        if( sf->st_mode & S_IXUSR || sf->st_mode & S_IXGRP || sf->st_mode & S_IXOTH )
                return 0;
        
        return -1;
}