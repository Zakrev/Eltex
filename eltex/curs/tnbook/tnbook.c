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

int MoveCursorToRowCol(MEDITW *editw, int row, int col)
{
        if( wmove(editw->text, row, col) != -1 ){
                editw->cursor.row = row;
                editw->cursor.col = col;
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