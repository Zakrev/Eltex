#include "tnbook.h"

void GetWinSize(int *col, int *row)
{
        struct winsize size;
        ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
        *col = size.ws_col;
        *row = size.ws_row;
}

void CreateDataBox(MEDITW *editw)
{
        int w_col, w_row;
        WINDOW *wbox;
        
        GetWinSize(&w_col, &w_row);
        wbox = newwin(w_row - (w_row / SIZE_CONST), w_col, 0, 0);
        box(wbox, '|', '-');
        wrefresh(wbox);

        editw->mbox = wbox;
}

void CreateDataText(MEDITW *editw)
{
        WINDOW *tbox;
        int w_col, w_row;
        
        GetWinSize(&w_col, &w_row);
        tbox = derwin(editw->mbox, w_row - (w_row / SIZE_CONST) - 2, w_col - 2, 1, 1);
        wrefresh(tbox);
        editw->row = w_row - (w_row / SIZE_CONST) - 2;
        editw->col = w_col - 2;
        editw->text = tbox;
        editw->buff = malloc(editw->row * editw->col);
        editw->cursor.row = 0;
        editw->cursor.col = 0;
}

void DeleteDataText(MEDITW *editw)
{
        free(editw->buff);
}

WINDOW *DrawMMenuBox()
{
        int w_col, w_row;
        WINDOW *wbox;
        
        GetWinSize(&w_col, &w_row);
        wbox = newwin(w_row / SIZE_CONST, w_col, w_row - (w_row / SIZE_CONST), 0);
        box(wbox, '|', '-');
        wrefresh(wbox);

        return wbox;
}

WINDOW *DrawMMenuText(WINDOW *mmenu_box, char *text)
{
        WINDOW *tbox;
        int w_col, w_row;
        
        GetWinSize(&w_col, &w_row);
        tbox = derwin(mmenu_box, 1, w_col - 2, 1, 1);
        wprintw(tbox, text);
        wrefresh(tbox);

        return tbox;
}

int FindEndLine(char *buff, int line_size)
{
        int i;
        
        for(i = 0; i < line_size - 1 && buff[i] != '\n' && buff[i] != '\0'; i++);

        if(buff[i] == '\0')
                return i - 1;
        
        return i;
}

int MoveCursorToRowCol(MEDITW *editw, int row, int col)
{
        int new_line_size;
        int end_line = editw->end_line;
        off_t curs_pos = editw->file.curs_pos;

        if(col > editw->col)
                return -1;
        if(col < 0)
                return -1;
        if(row + 1 > editw->row){
                ScrollDown(editw);
                return -1;
        }
        if(row < 0){
                ScrollUp(editw);
                return -1;
        }
        /*if(editw->cursor.row < row){
                new_line_size = FindEndLine(editw->buff + end_line + 1, editw->col);
                end_line += new_line_size + 1;
                if(col > new_line_size){
                        col = new_line_size;
                }
        } */
        if( wmove(editw->text, row, col) != -1 ){
                editw->cursor.row = row;
                editw->cursor.col = col;
                editw->end_line = end_line;
        } else 
                return -1;

        return 0;
}

int MoveCursorTo(MEDITW *editw, int dir)
{
        int n_row, n_col;

        n_row = editw->cursor.row;
        n_col = editw->cursor.col;
        
        switch(dir){
                case MC_TO_LEFT:
                        n_col -= 1;
                break;
                case MC_TO_RIGHT:
                        n_col += 1;
                break;
                case MC_TO_UP:
                        n_row -= 1;
                break;
                case MC_TO_DOWN:
                        n_row += 1;
                break;
        }
        
        return MoveCursorToRowCol(editw, n_row, n_col);
}

int PrintNewLine(MEDITW *editw)
{
        winsch(editw->text, '\n');
        editw->cursor.col = 0;
        return MoveCursorTo(editw, MC_TO_DOWN);
}

int PrintCh(MEDITW *editw, char ch)
{
        waddch(editw->text, ch);
        if(MoveCursorTo(editw, MC_TO_RIGHT) == -1){
                editw->cursor.col = 0;
                return MoveCursorTo(editw, MC_TO_DOWN);
        }
        
        return 0;
}

void ClearBuffer(char* buff, int size)
{
        int i;

        for(i = 0; i < size; i++)
                buff[i] = '\0';
}

void ScrollDown(MEDITW *editw)
{
        MFILE *file = &editw->file;
        off_t old_first_line = file->first_line;
        off_t new_ch_count;//Прочитанные новые символы из оригинального файла

        if( (lseek(file->filed, 0, SEEK_END) - file->first_line) <= editw->col * editw->row)//Если файл закончился
                return;

        file->first_line = lseek(file->filed, file->first_line + editw->col, SEEK_SET);
        ClearBuffer(editw->buff, editw->col * editw->row);
        read(file->filed, editw->buff, editw->row * editw->col);
        file->last_line = lseek(file->filed, 0, SEEK_CUR);
        file->curs_pos += file->first_line - old_first_line;
        lseek(file->filed, file->curs_pos, SEEK_SET);

        new_ch_count = file->last_line - file->copy_pos;
        if( new_ch_count > 0 ){
                lseek(file->tmpd, file->copy_pos, SEEK_SET);
                write(file->tmpd, editw->buff + ((file->last_line - file->first_line) - new_ch_count), new_ch_count);
                file->copy_pos = lseek(file->tmpd, 0, SEEK_CUR);
        }

        werase(editw->text);
        wrefresh(editw->text);
        waddstr(editw->text, editw->buff);
        wmove(editw->text, editw->cursor.row, editw->cursor.col);
}

void ScrollUp(MEDITW *editw)
{
        MFILE *file = &editw->file;
        off_t old_first_line = file->first_line;

        if(file->first_line == 0)
                return;

        if( (file->first_line -= editw->col) < 0 )
                file->first_line = 0;
        file->first_line = lseek(file->filed, file->first_line, SEEK_SET);
        ClearBuffer(editw->buff, editw->col * editw->row);
        read(file->filed, editw->buff, editw->row * editw->col);
        file->last_line = lseek(file->filed, 0, SEEK_CUR);
        file->curs_pos += file->first_line - old_first_line;
        lseek(file->filed, file->curs_pos, SEEK_SET);
        
        werase(editw->text);
        wrefresh(editw->text);
        waddstr(editw->text, editw->buff);
        wmove(editw->text, editw->cursor.row, editw->cursor.col);
}

void OpenAndPrintFile(MEDITW *editw)
{
        MFILE *file = &editw->file;
        
        file->filed = open("ej.txt", O_RDONLY);
        if(file->filed == -1){
                werase(editw->text);
                /*
                        ERROR
                */
                return;
        }
        file->tmpd = open("ej.txt.tmp", O_CREAT | O_RDWR, S_IRWXO | S_IRWXU | S_IRWXG);
        if(file->tmpd == -1){
                close(file->filed);
                werase(editw->text);
                /*
                        ERROR
                */
                return;
        }
        file->first_line = file->curs_pos = file->copy_pos = lseek(file->filed, 0, SEEK_SET);
        ClearBuffer(editw->buff, editw->col * editw->row);
        read(file->filed, editw->buff, editw->row * editw->col);
        editw->end_line = FindEndLine(editw->buff, editw->col);
        file->last_line = lseek(file->filed, 0, SEEK_CUR);
        lseek(file->filed, 0, SEEK_SET);
        
        lseek(file->tmpd, 0, SEEK_SET);
        write(file->tmpd, editw->buff, file->last_line - file->copy_pos);
        file->copy_pos = lseek(file->tmpd, 0, SEEK_CUR);
        
        waddstr(editw->text, editw->buff);
        MoveCursorToRowCol(editw, 0, 0);
}