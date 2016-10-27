#include "tfm.h"

int main()
{
        WINDOW *key_info;
        TREE_WIN twin_left;
        TREE_WIN twin_right;
        TREE_WIN *now_win;
        CLINE cline;
        int key;
        int twin_left_width;
        
        signal(SIGINT, SIG_IGN);
        initscr();
        keypad(stdscr, 1);
        noecho();
        refresh();
        key_info = derwin(stdscr, 0, 0, 0, 0);
        wprintw(key_info, " F10 - Exit    F4 - Comand line    F5 - Copy selected file    Tab - swich window");
        wrefresh(key_info);
        InitComandLine(&cline, 1, 1);
        twin_left_width = InitWindowTree(&twin_left, 0, 5);
        CreateDInfoList(&twin_left);
        PrintDInfoList(&twin_left, 1);
        InitWindowTree(&twin_right, twin_left_width, 5);
        CreateDInfoList(&twin_right);
        PrintDInfoList(&twin_right, 1);
        now_win = &twin_left;
        MoveCursorToRowCol(now_win, 0, 0);
        
        while( (key = getch()) > 0 ){
                if(key == KEY_F(10))
                        OnExit();
                if(key == KEY_F(5)){
                        InitCopyFile(&now_win, now_win == &twin_left ? &twin_right : &twin_left);
                }
                if(key == KEY_F(4)){
                        MoveCursorToRowCol_CLINE(&cline, cline.cursor.row, cline.cursor.col);
                        RunComandLine(&cline);
                        MoveCursorToRowCol(now_win, now_win->cursor.row, now_win->cursor.col);
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

/*
        Функция завершает программу
*/
void OnExit()
{
        endwin();
        exit(0);
}

/*
        Переключает активное окно TREE_WIN.
        Возвращает указатель новое активное окно.
*/
TREE_WIN *SwitchWin(TREE_WIN *now, TREE_WIN *left, TREE_WIN *right)
{
        if(now == left)
                return right;
        else
                return left;
}

/*
        Получение размера терминального окна
*/
void GetWinSize(int *row, int *col)
{
        struct winsize size;
        ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
        *col = size.ws_col;
        *row = size.ws_row;
}

/*
        Функция проверяет строку str. Если строка состоит только из символа "." или пустая, то возвращает ошибку
*/
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

/*
        Функция создает новое окно TREE_WIN и отрисовывает его на экране 
        с верхним левым углом в позиции (x,y).
        Возвращат ширину нового окна.
*/
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

/*
        Удаляет дерево файлов из структуры TREE_WIN
*/
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

/*
        Создает дерево файлов текущего каталога в структуре TREE_WIN
*/
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

/*
        Функция возвращает строку (в переменную name) для колонки Name в окне TREE_WIN
        В строке записываются разрешения и имя файла
*/
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

/*
        Функция очищает окно TREE_WIN, печатает на экран часть дерева файлов относительно позиции курсора (start_index)
        в списке файлов текущего каталога (twin->name_list_d)
*/
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

/*
        Функия передвигает курсор на экране в позицию (row, col)
        И записывает изминения позиции в структуре TREE_WIN
        Если экранный курсор сдвинуть не получилось, возвращает ошибку
*/
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

/*
        Функция делает активной следующую (вниз) строку дерева файлов в окне TREE_WIN
        Возвращает результат выполнения функции MoveCursorToRowCol (результат перемещения курсора),
        либо ошибку, если список файлов закончился
*/
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

/*
        Функция делает активной следующую (вверх) строку дерева файлов в окне TREE_WIN
        Возвращает результат перемещения курсора, либо ошибку, если список файлов закончился
*/
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

/*
        Функция меняет текущий каталог окна TREE_WIN
        Создает и печатает новое дерево файлов
        Возвращает ошибку, если каталог сменить невозможно
*/
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

/*
        Функция меняет текущий каталог окна TREE_WIN
        Не создает и не печатает новое дерево файлов, используется при переключении окон TREE_WIN
        Возвращает ошибку, если каталог сменить невозможно
*/
int ChDir_NotMove(TREE_WIN *twin, char *dname)
{
        if(chdir(dname) < 0)
                return -1;
        getcwd(twin->cur_dir, CURDIR_SIZE);
        
        return 0;
}

/*
        Функция обрабатывает нажатие Enter
        Выполняет следующие действия, в зависимости от типа выбранного файла:
                - смена каталога
                - редактирование файла (если у файла нет прав на запуск)
                - запуск файла
        После запуска на исполнение/редактирования файла вновь инициализирует ncurses и обновляет активное окно
*/
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
                                /*
                                        Смена каталога
                                */
                                return ChDir(twin, name_list[name_index]->d_name);
                        break;
                        default:
                                if( FileIsExe(&stat_buff) == 0 && IsFileExtension(name_list[name_index]->d_name) != 0 ){
                                        /*
                                                Исполнение файла
                                        */
                                        StartFile(name_list[name_index]->d_name);
                                        initscr();
                                        PrintDInfoList(twin, 1);
                                } else {
                                        /*
                                                Редактирование файла
                                        */
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

/*
        Функция отключает ncurses и порождает новый процесс, в котором запускается файл.
        filename - путь и имя исполняемого файла
*/
int StartFile(char *fname)
{
        pid_t pid;
        
        endwin();
        pid = fork();
        if(pid == 0){
                signal(SIGINT, SIG_DFL);
                execl(fname, fname, (char*)NULL);
                /*
                        Процесс останавливается в случае ошибки
                */
                perror("Error on start process");
                exit(1);
        } else {
                wait(0);
        }
        
        return 0;
}

/*
        Функция отключает ncurses и порождает новый процесс, в котором запускается редактор файла.
        filename - путь и имя редактируемого файла
*/
int EditFile(char *fname)
{
        pid_t pid;
        
        endwin();
        pid = fork();        
        if(pid == 0){
                signal(SIGINT, SIG_DFL);
                execlp("mc", "mc", "-e", fname, (char*)NULL);
                /*
                        Процесс останавливается в случае ошибки
                */
                perror("Error on open file");
                exit(1);
        } else {          
                wait(0);
        }
        
        return 0;
}

/*
        Функция создает структуру PBAR_WIN с левым верхним углом в позиции (x, y) и текстом text.
        Заполняет ее и отрисовывает прогрессбар на экране
        Возвращает указатель на новую структуру PBAR_WIN
*/
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

/*
        Функция удаляет прогрессбар с экрана и удаляет структуру PBAR_WIN.
*/
void DeletePBAR(PBAR_WIN *pbar)
{
        wclear(pbar->box);
        wrefresh(pbar->box);
        delwin(pbar->progress);
        delwin(pbar->text);
        delwin(pbar->box);
        free(pbar);
}

/*
        Функция перерисовывает линию прогресса в соответствии с значением val (от 0 до 100)
*/
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

/*
        Функция (поток) пытается открыть файл mCopyInfo.from. Если успешно, то пытается
        создать файл mCopyInfo.to с флагами идентичными флагам mCopyInfo.from.
        Добавляет на экран прогрессбар.
        Копирует данные из файла mCopyInfo.from в файл mCopyInfo.to со скоростью 64 бит/с.
        Также изменяет состояния полосы прогресса в прогрессбаре.
        После копирования закрывает все файлы и убирает с экрана прогрессбар.
*/
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

/*
        Функция инициализирует копирование файла.
        Создает структуру mCopyInfo и заполняет ее информацией о копируемом и новом файлах.
        Если копируемый файл не может быть определен, возвращается ошибка
        Если копируемый файл является директорией, возвращается ошибка
        Создает новый поток CopyFile, которому передает структуру mCopyInfo
*/
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
        if(stat(mci->from, &from_stat) < -1){
                /*
                        Неопределенный файл
                */
                free(mci);
                return -1;
        }
        if( (from_stat.st_mode & S_IFMT) == S_IFDIR ){
                /*
                        Дирректория
                */
                free(mci);
                return -1;
        }
        mci->from_flags = from_stat.st_mode;
        pthread_create(&pth, NULL, CopyFile, mci);
        
        return 0;
}

/*
        Функция проверяет является-ли файл читаемым
*/
int FileIsRead(struct stat *sf)
{
        if( sf->st_mode & S_IRUSR || sf->st_mode & S_IRGRP || sf->st_mode & S_IROTH )
                return 0;
        
        return -1;
}

/*
        Функция проверяет является-ли файл изменяемым
*/
int FileIsWrite(struct stat *sf)
{
        if( sf->st_mode & S_IWUSR || sf->st_mode & S_IWGRP || sf->st_mode & S_IWOTH )
                return 0;
        
        return -1;
}

/*
        Функция проверяет является-ли файл исполняемым
*/
int FileIsExe(struct stat *sf)
{
        if( sf->st_mode & S_IXUSR || sf->st_mode & S_IXGRP || sf->st_mode & S_IXOTH )
                return 0;
        
        return -1;
}

/*
        Функция проверяет наличия расширения у файла file_name
        Возвращает ошибку, если расширения нет
*/
int IsFileExtension(char *file_name)
{
        int i;
        
        for(i = 0; file_name[i] != '\0'; i++){
                if(file_name[i] == '.')
                        return 0;
        }
        
        return -1;
}

/*
        Функция создает новое окно CLINE и отрисовывает его на экране 
        с верхним левым углом в позиции (x,y).
*/
int InitComandLine(CLINE *cline, int x_col, int y_row)
{
        int b_col, b_row;
        
        GetWinSize(&b_row, &b_col);
        b_row = 3;
        b_col -= 4;
        
        cline->box = newwin(b_row, b_col, y_row, x_col);
        box(cline->box, '|', '-');
        wrefresh(cline->box);
        
        cline->text = derwin(cline->box, 1, b_col-4, 1, 2);
        wrefresh(cline->text);
        
        cline->cursor.row = 0;
        cline->cursor.col = 0;
        cline->buf_pos = 0;
        cline->buff[0] = '\0';

        return 0;
}

/*
        Функция обрабатывает клавиши в режиме командной строки
*/
int RunComandLine(CLINE *cline)
{
        int key;
        
        while( (key = getch()) > 0 ){
                switch(key){
                        case KEY_F(10):
                                /*
                                        Нажатие F10, выход из программы
                                */
                                OnExit();
                        break;
                        case KEY_F(4):
                                /*
                                        Нажатие F4, выход из режима командной строки
                                */
                                return 0;
                        break;
                        case 263:
                                /*
                                        Нажатие Backspace, удаление последнего символа из буфера
                                */
                                EraseFromClineBuffer(cline);
                        break;
                        case 10:
                                /*
                                        Нажатие Enter, выполнение введенных команд
                                */
                                if(OnPressEnter_CLINE(cline) == 0){
                                        initscr();
                                        cline->buff[0] = '\0';
                                        cline->buf_pos = 0;
                                        MoveCursorToRowCol_CLINE(cline, cline->cursor.row, 0);
                                        wclear(cline->text);
                                        wrefresh(cline->text);
                                        
                                        return 0;
                                }
                                initscr();
                                wrefresh(cline->text);
                        break;
                        default:
                                /*
                                        Нажатие любой другой клавиши, запись символа в буфер
                                */
                                AddToClineBuffer(cline, (char)key);
                        break;
                }
        }
        
        return 0;
}

/*
        Функия передвигает курсор на экране в позицию (row, col) в пределах CLINE
        Если экранный курсор сдвинуть не получилось, возвращает ошибку
*/
int MoveCursorToRowCol_CLINE(CLINE *cline, int row, int col)
{
        if( wmove(cline->text, row, col) > -1 ){
                cline->cursor.row = row;
                cline->cursor.col = col;
                wrefresh(cline->text);
                return 0;
        }
        
        return -1;
}

/*
        Функция заносит новый символ в буфер CLINE->buff
        Проверяет символ, после чего заносит символ в буфер в позицию CLINE->buf_pos
        Увеличивает значение позиции курсора по x и позиции указателя CLINE->buf_pos
        Печатает новый символ на экране
        
*/
int AddToClineBuffer(CLINE *cline, char c)
{       
        if(!( (c >= 'A' && c <= 'Z') || 
        (c >= 'a' && c <= 'z') || 
        (c >= '0' && c <= '9') || 
        c == '/' || c == ' ' || c == '_' || c == '-' || c == '|' || c == '.' || c == '\'' || c == '"'))
                return -1;
        
        if(cline->buf_pos + 2 >= CLINE_LENGHT)
                return -1;
        
        cline->buff[cline->buf_pos] = c;
        cline->buff[cline->buf_pos + 1] = '\0';
        cline->buf_pos += 1;
        cline->cursor.col += 1;
        
        wprintw(cline->text, cline->buff + cline->buf_pos - 1);
        wrefresh(cline->text);
        
        return 0;
}

/*
        Функция удаляет последний символ из буфера CLINE->buff
        Уменьшает значение позиции курсора по x и позиции указателя CLINE->buf_pos
        Удаляет последний символ с экрана
*/
int EraseFromClineBuffer(CLINE *cline)
{              
        if(cline->buf_pos - 1 < 0)
                return -1;
        
        if( wmove(cline->text, cline->cursor.row, cline->cursor.col - 1) > -1 ){
                cline->buff[cline->buf_pos - 1] = '\0';
                cline->buf_pos -= 1;
                cline->cursor.col -= 1;
                
                wdelch(cline->text);
                wrefresh(cline->text);
        }
        
        return 0;
}

/*
        Обрабатывает нажатие Enter в окне CLINE
        Создает процесс, который разбирает и выполняет команды из буфера
        Возвращает результат работы потомка
*/
int OnPressEnter_CLINE(CLINE *cline)
{        
        pid_t pid;
        int rez;
        
        endwin();
        pid = fork();
        
        if(pid == 0){
                ExecuteLine(cline->buff);
        } else {
                waitpid(pid, &rez, 0);
                if(rez != 0)
                        return -1;               
        }
        
        return 0;
}

/*
        Функция разбирает входную строку str с позиции pos на команды и параметры,
        которые записывает в массив строк arg
        Возвращает позицию символа следующей команды после '|', в строке str.
*/
int GetArgements(char *arg[ARG_LENGHT], char tmp_arg[ARG_LENGHT][ARG_U_SIZE], char *str, int pos)
{
        int i = 0, j;

        for( i = 0; str[pos] != '|' && str[pos] != '\0' && i < ARG_LENGHT; pos++){                
                for(j = 0; j < ARG_U_SIZE && str[pos] != ' ' && str[pos] != '|' && str[pos] != '\0'; j++, pos++){
                        tmp_arg[i][j] = str[pos];
                }
                tmp_arg[i][j] = '\0';
                arg[i] = tmp_arg[i];
                i++;
                if(str[pos] == '\0')
                        break;
        }
        arg[i] = (char*)NULL;
        
        return pos+2;
}

/*
        Функция получает на вход строку str с командами.
        Разбирает ее и поочередно запускает полученные команды.
        
        Подсчитывает количество команд в строке.
        Создает канал 1 для вывода в него данных первой команды.
        Получает первую команду и ее аргументы, затем создает процесс-потомок.
        Назначает потомку канал 1 вместо стандартного stdout, и заменяет потомка новой командой.
        Закрывает канал 1 на запись и ждет завершения потомка.
        Если команд было больше одной, то входит в цикл, либо читает канал 1, выводит полученную информацию и завершается.
        
        В цикле создает канал 2, получает следующую команду и ее аргументы, создает процесс-потомок.
        Заменяет потомку stdout на канал 2, а stdin на канал 1, после чего заменяет процесс исполняемой командой.
        Ждет завершений потомка, полностью закрывает канал 1, также закрывает канал 2 на ввод.
        Если команд на исполнение больше не осталось, читает канал 2 и выводит полученную информацию. Завершается.
        Иначе, создает канал 1, получает следующую команду и ее аргументы, создает процесс-потомок.
        Заменяет потомку stdout на канал 1, а stdin на канал 2, после чего заменяет процесс исполняемой командой.
        Ждет завершений потомка, полностью закрывает канал 2, также закрывает канал 1 на ввод.
        Если команд на исполнение больше не осталось, читает канал 1 и выводит полученную информацию. Завершается.
        Иначе, повторяет цикл.
        
        Возвращает результат запуска последней команды
*/
int ExecuteLine(char *str)
{
        char out[CLINE_LENGHT] = "";
        
        char *arg[ARG_LENGHT];
        char tmp_arg[ARG_LENGHT][ARG_U_SIZE];
        int pipefd_left[2], pipefd_right[2];
        int i, count, pos, rez = 0;
        pid_t pid;
        
        /*
                Подсчет команд
        */
        for(i = 0, count = 0; str[i] != '\0'; i++){
                if(str[i] == '|')
                        count++;
        }
        count += 1;
        
        if(pipe(pipefd_left) < 0){
                perror("Error on create pipe");
                exit(1);
        }
        pos = GetArgements(arg, tmp_arg, str, 0);
        count--;
        pid = fork();
        if(pid == 0){
                close(pipefd_left[0]);
                dup2(pipefd_left[1], 1);
                execvp(arg[0], arg);
                perror(arg[0]);
                exit(1);
        } else {
                close(pipefd_left[1]);
                waitpid(pid, &rez, 0);
                while(count > 0){                        
                        if(count <= 0)
                                break;                        
                        if(pipe(pipefd_right) < 0){
                                perror("Error on create pipe");
                                exit(1);
                        }
                        pos = GetArgements(arg, tmp_arg, str, pos);
                        count--;
                        pid = fork();
                        if(pid == 0){
                                dup2(pipefd_right[1], 1);
                                dup2(pipefd_left[0], 0);
                                execvp(arg[0], arg);
                                perror(arg[0]);
                                exit(1);
                        } else {
                                waitpid(pid, &rez, 0);
                                close(pipefd_left[0]);
                                close(pipefd_right[1]);                                        
                                if(count <= 0){
                                        goto read_right;      
                                }
                                if(pipe(pipefd_left) < 0){
                                        perror("Error on create pipe");
                                        exit(1);
                                }
                                pos = GetArgements(arg, tmp_arg, str, pos);
                                count--;
                                pid = fork();
                                if(pid == 0){
                                        dup2(pipefd_right[0], 0);
                                        dup2(pipefd_left[1], 1);
                                        execvp(arg[0], arg);
                                        perror(arg[0]);
                                        exit(1);
                                }else{
                                        waitpid(pid, &rez, 0);
                                        close(pipefd_right[0]);
                                        close(pipefd_left[1]);
                                }
                        }
                }
                goto read_left;
        
                read_right:
                        dup2(pipefd_right[0], 0);
                        close(pipefd_right[0]);
                read_left:
                        dup2(pipefd_left[0], 0);
                        close(pipefd_left[0]);
                
                if(rez != 0)
                        exit(1);
                
                read(0, out, 512);
                if(out[0] == '\0')
                        printf("Output is empty.\n");
                else
                        printf("%s", out);             
                
                exit(0);
        }
        
        return 0;
}
