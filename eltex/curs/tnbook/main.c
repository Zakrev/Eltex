#include "tnbook.h"

int main(int argn, char **args)
{
        EDITWIN *ewin;
        MENUWIN mwin;
        int key;
        char ch[3];

        initscr();
        setlocale(0, "utf-8");
        noecho();
        keypad(stdscr, 1);
        refresh();
        ewin = InitEDITWIN();
        InitMENUWIN(&mwin);
        if(argn > 1){
                if(OpenFile(ewin, args[1]) > -1 ){
                        ReadFile(ewin);
                        PrintBuff(ewin);
                } else {
                        perror("Error open file:");
                }
        }
        while( (key = getch()) > -1 ){
                switch(key){
                        case KEY_F(10):
                                CloseFile(ewin);
                                DeleteEDITWIN(ewin);
                                endwin();
                                return 0;
                        break;
                        case KEY_DOWN:
                                ScrollDown(ewin);
                        break;
                        case KEY_UP:
                                ScrollUp(ewin);
                        break;
                        case KEY_LEFT:
                        break;
                        case KEY_RIGHT:
                        break;
                        default:
                                sprintf(ch, "%c", (char)key);
                                printf("%s", ch);
                                //waddstr(ewin->text, ch);
                                wrefresh(ewin->text);
                        break;
                }
        }
        endwin();

        return 0;
}