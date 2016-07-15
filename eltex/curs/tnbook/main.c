#include "tnbook.h"

void ClearBuffer(char* buff, int size)
{
        int i;

        for(i = 0; i < size; i++)
                buff[i] = ' ';
        buff[size - 1] = '\0';
}

void ScrollDown(MEDITW *editw)
{
        MFILE *file = &editw->file;
        off_t old_first_line = file->first_line;
        off_t new_ch_count;//Прочитанные новые символы из оригинального файла
        int tmp_curs_row = editw->cursor.row;
        int tmp_curs_col = editw->cursor.col;

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

        MoveCursorToRowCol(editw, 0, 0);
        werase(editw->text);
        wrefresh(editw->text);
        wprintw(editw->text, editw->buff);
        MoveCursorToRowCol(editw, tmp_curs_row, tmp_curs_col);
}

void ScrollUp(MEDITW *editw)
{
        MFILE *file = &editw->file;
        off_t old_first_line = file->first_line;
        int tmp_curs_row = editw->cursor.row;
        int tmp_curs_col = editw->cursor.col;

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
        
        MoveCursorToRowCol(editw, 0, 0);
        werase(editw->text);
        wrefresh(editw->text);
        wprintw(editw->text, editw->buff);
        MoveCursorToRowCol(editw, tmp_curs_row, tmp_curs_col);
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
        file->last_line = lseek(file->filed, 0, SEEK_CUR);
        lseek(file->filed, 0, SEEK_SET);
        
        lseek(file->tmpd, 0, SEEK_SET);
        write(file->tmpd, editw->buff, file->last_line - file->copy_pos);
        file->copy_pos = lseek(file->tmpd, 0, SEEK_CUR);
        
        wprintw(editw->text, editw->buff);
        MoveCursorToRowCol(editw, 0, 0);
}

int main()
{
        WINDOW *m_box;
        WINDOW *m_text;
        MEDITW editw;
        char *key;

        initscr();
        noecho();
        keypad(stdscr, 1);
        refresh();
        CreateDataBox(&editw);
        CreateDataText(&editw);
        m_box = DrawMMenuBox();
        m_text = DrawMMenuText(m_box, "Use Shift + .. : F5 - New File\tF6 - Open file\tF7 - Save file\tF8 - Find text\tF9 - Exit");
        MoveCursorToRowCol(&editw, 0, 0);
        wrefresh(editw.text);
        OpenAndPrintFile(&editw);
        wrefresh(editw.text);
        while( key = getch() ){
                //PrintCh(&editw, (char)key);
                switch((int)key){
                        case KEY_F(21):
                                endwin();
                                return 0;
                        break;
                        case KEY_DOWN:
                                //MoveCursorTo(&editw, MC_TO_DOWN);
                                ScrollDown(&editw);
                        break;
                        case KEY_UP:
                                //MoveCursorTo(&editw, MC_TO_UP);
                                ScrollUp(&editw);
                        break;
                        case KEY_LEFT:
                                MoveCursorTo(&editw, MC_TO_LEFT);
                        break;
                        case KEY_RIGHT:
                                MoveCursorTo(&editw, MC_TO_RIGHT);
                        break;
                }
                wrefresh(editw.text);
        }
        DeleteDataText(&editw);
        endwin();

        return 0;
}