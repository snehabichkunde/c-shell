#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>

#include<ncurses.h>

#define ctrl(x) ((x)& 0x1f)

int main()
{
    initscr();
    raw();
    noecho();
    int ch;
    char command[1024] = {0};
    size_t command_s = 0;
    bool QUIT = false;
    printw("[c-shell]~");
    while(!QUIT){
        printw(command);
        ch = getch();
        switch(ch){
            case ctrl('q'):
                QUIT = true;
                break;
            case KEY_ENTER:
                break;
            default: 
                command[command_s++] = ch;
                break;
        }
        erase();
    }

    refresh();
    return 0;
}