#include "tnbook.h"

void OpenAndPrintFile(MEDITW *editw)
{
        editw->filed = open("ej.txt", O_RDONLY);
        if(editw->filed == -1){
                werase(editw->text);
                return;
        }
        read(editw->filed, editw->buff, editw->row * editw->col);
        wprintw(editw->text, editw->buff);
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
                                MoveCursorTo(&editw, MC_TO_DOWN);
                        break;
                        case KEY_UP:
                                MoveCursorTo(&editw, MC_TO_UP);
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