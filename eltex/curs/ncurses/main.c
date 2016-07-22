#include <termios.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <stdlib.h>
#include <curses.h>

#define D_RIGHT 1
#define D_LEFT 2
#define D_UP 3
#define D_DOWN 4

void SigWinch(int signo)
{
        struct winsize size;

        ioctl(fileno(stdout), TIOCGWINSZ, (char*) &size);
        resizeterm(size.ws_row, size.ws_col);
}

struct Snake {
        WINDOW* body;
        WINDOW* nobody;
        int dir;
        int x;
        int y;
        int len;
        int l_x;
        int l_y;
};

void init_snake(struct Snake* snake, int s_x, ints_y, int s_dir)
{
        snake.x = s_x;
        snake.y = s_y;
        snake.body = newwin(1, 1, c_x, c_y);
        snake.nobody = newwin(1,1,c_x,c_y);
}

void move_snake(WINDOW* snake, int dir)
{
        
}

int main()
{
        struct Snake snake;
        int i = 0;
        char* key;

        initscr();
        signal(SIGWINCH, SigWinch);
        cbreak();
        noecho();
        curs_set(0);
        box(stdscr, '|', '-');
        keypad(stdscr, 1);
        move(5, 5);
        refresh();
        snake.body = newwin(1, 1, c_x, c_y);
        snake.nobody = newwin(1,1,c_x,c_y);
        wprintw(snake, "0");
        wrefresh(snake);
        timeout(1000);
        while(1){
                key = wgetch(stdscr);

                c_x++;
                mvwin(snake, c_x, c_y);
                wrefresh(snake);
                wrefresh(stdscr);

                if(key == KEY_END)
                        break;
        }
        timeout(-1);
        refresh();
        getch();
        endwin();

        return 0;
}