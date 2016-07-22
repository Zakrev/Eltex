#include "tnbook.h"

void GetWinSize(int *col, int *row)
{
        struct winsize size;
        ioctl(fileno(stdout), TIOCGWINSZ, (char *) &size);
        *col = size.ws_col;
        *row = size.ws_row;
}

EDITWIN *InitEDITWIN()
{
        int win_w, win_h;
        EDITWIN *ewin;
        
        ewin = malloc(sizeof(EDITWIN));
        if(ewin == NULL)
                return NULL;
        GetWinSize(&win_w, &win_h);
        ewin->box = newwin(win_h - win_h / SIZEN, win_w, 0, 0);
        box(ewin->box, '|', '-');
        wrefresh(ewin->box);
        
        ewin->text = derwin(ewin->box, win_h - win_h / SIZEN - 2, win_w - 2, 1, 1);
        ewin->text_h = win_h - win_h / SIZEN - 2;
        ewin->text_w = win_w - 2;
        
        ewin->buff_index = 0;
        ewin->buff_text = malloc(ewin->text_h * ewin->text_w);
        
        ewin->cursor.x = 0;
        ewin->cursor.y = 0;
        
        ewin->file.filed = -1;
        ewin->file.tmpd = -1;
        ewin->file.file_pos = 0;
        ewin->file.tmp_pos = 0;
        ewin->file.first_ch = 0;
        ewin->file.last_ch = 0;
        
        return ewin;
}

int InitMENUWIN(MENUWIN *mwin)
{
        int win_w, win_h;
        
        GetWinSize(&win_w, &win_h);
        mwin->box = newwin(win_h / SIZEN, win_w, win_h - win_h / SIZEN, 0);
        box(mwin->box, '|', '-');
        wrefresh(mwin->box);
        
        mwin->text = derwin(mwin->box, win_h / SIZEN - 2, win_w - 2, 1, 1);
        wprintw(mwin->text, "F10 - Exit");
        wrefresh(mwin->text);
        
        return 0;
}

int OpenFile(EDITWIN * ewin, char *fname)
{
        EFILE *file = &ewin->file;
        
        file->filed = open(fname, O_RDONLY);
        if(file->filed == -1){
                return -1;
        }
        sprintf(file->tmp_name, "%s.tmp", fname);
        file->tmpd = open(file->tmp_name, O_CREAT | O_RDWR, S_IRWXO | S_IRWXU | S_IRWXG);
        if(file->tmpd == -1){
                CloseFile(ewin);
                return -1;
        }
        
        return 0;
}

void CopyFile(EDITWIN *ewin)
{
        EFILE *file = &ewin->file;
        off_t pos;
        off_t tmp_npos;
        int read_count = -1;
        
        pos = lseek(file->filed, 0, SEEK_END);
        if(pos == file->tmp_pos)
                return;
                
        tmp_npos = file->first_ch + ewin->text_h * ewin->text_w;
        if(file->tmp_pos < tmp_npos){
                lseek(file->filed, file->tmp_pos, SEEK_SET);
                read_count = read(file->filed, ewin->buff_text, ewin->text_w * ewin->text_h);
                lseek(file->tmpd, 0, SEEK_END);
                write(file->tmpd, ewin->buff_text, read_count);
                file->tmp_pos = lseek(file->filed, 0, SEEK_CUR);
        }
}

int ReadFile(EDITWIN *ewin)
{
        EFILE *file = &ewin->file;
        int rcount;
        
        if(file->filed == -1)
                return -1;
        CopyFile(ewin);
        lseek(file->tmpd, file->first_ch, SEEK_SET);
        rcount = read(file->tmpd, ewin->buff_text, ewin->text_h * ewin->text_w);
        if(rcount < ewin->text_h * ewin->text_w)
                ewin->buff_text[rcount] = '\0';
        file->last_ch = lseek(file->tmpd, 0, SEEK_CUR);
        
        return 0;
}

void EraseCh(char *str, int lenght, char ch, char nch)
{
        int i;
        
        for(i = 0; i < lenght; i++){
                if(str[i] == ch)
                        str[i] = nch;
        }
}

int PrintBuff(EDITWIN *ewin)
{
        werase(ewin->text);
        EraseCh(ewin->buff_text, ewin->text_w * ewin->text_h, '\n', ' ');
        wprintw(ewin->text, ewin->buff_text);
        wmove(ewin->text, ewin->cursor.y, ewin->cursor.x);
        wrefresh(ewin->text);
        
        return 0;
}

int ScrollDown(EDITWIN *ewin)
{
        EFILE *file = &ewin->file;
        off_t end_ch;
        off_t first_ch;
        
        first_ch = file->first_ch + ewin->text_w;
        end_ch = lseek(file->filed, 0, SEEK_END);
        if(end_ch < (file->first_ch + ewin->text_w * ewin->text_h)){
                return -1;
        }
        file->first_ch = first_ch;
        ReadFile(ewin);
        PrintBuff(ewin);
        
        return 0;
}

int ScrollUp(EDITWIN *ewin)
{
        EFILE *file = &ewin->file;
        off_t first_ch;
        
        first_ch = file->first_ch - ewin->text_w;
        if(first_ch < 0){
                return -1;
        }
        file->first_ch = first_ch;
        ReadFile(ewin);
        PrintBuff(ewin);
        
        return 0;
}

int CloseFile(EDITWIN *ewin)
{
        EFILE *file = &ewin->file;
        
        if(file->filed != -1)
                close(file->filed);
        if(file->tmpd != -1){
                remove(file->tmp_name);
                close(file->tmpd);
        }
        file->filed = -1;
        file->tmpd = -1;
        file->last_ch = file->first_ch = file->file_pos = file->tmp_pos = 0;
        ewin->buff_index = 0;
        ewin->cursor.x = 0;
        ewin->cursor.y = 0;
        werase(ewin->text);
        wrefresh(ewin->text);
        
        return 0;
}

void DeleteEDITWIN(EDITWIN *ewin)
{
        delwin(ewin->text);
        delwin(ewin->box);
        free(ewin->buff_text);
        free(ewin);
}