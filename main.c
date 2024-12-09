#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>

#include<ncurses.h>

#define ctrl(x) ((x)& 0x1f)
#define SHELL "[c-shell]~ "
#define ENTER 10

#define DATA_START_CAPACITY 128 // Initial capacity for dynamic arrays


#define ASSERT(cond, ...) \
    do { \
        if (!(cond)) { \
            endwin(); \
            fprintf(stderr, "%s:%d: ASSERTION FAILED: ", __FILE__, __LINE__); \
            fprintf(stderr, __VA_ARGS__); \
            fprintf(stderr, "\n"); \
            exit(1); \
        } \
    } while (0)

#define DA_APPEND(da, item) do {                                                       \
    if ((da)->count >= (da)->capacity) {                                               \
        (da)->capacity = (da)->capacity == 0 ? DATA_START_CAPACITY : (da)->capacity*2; \
        void *new = calloc(((da)->capacity+1), sizeof(*(da)->data));                   \
        ASSERT(new,"outta ram");                                                       \
        if ((da)->data != NULL)                                                        \
            memcpy(new, (da)->data, (da)->count);                                      \
        free((da)->data);                                                              \
        (da)->data = new;                                                              \
    }                                                                                  \
    (da)->data[(da)->count++] = (item);                                                \
} while (0)





typedef struct 
{
    char *data;     // pointer to the character array 
    size_t count;   // current number of characters in the array 
    size_t capacity;    // max capacity before reallocation is needed 
}String;

typedef struct 
{
    String *data;     
    size_t count;   
    size_t capacity;    
}Strings;

int main()
{
    initscr();  // Initializes the ncurses environment for screen manipulation
    raw();      // Disables line buffering, allowing real-time input capture.
    noecho();   // Prevents typed characters from being automatically displayed.
    int ch;
    String command = {0};
    Strings command_his = {0};
    

    bool QUIT = false; 
    size_t line = 0;
    while(!QUIT){
        mvprintw(line, 0, SHELL);
        mvprintw(line , 0+ sizeof(SHELL)-1,"%.*s",(int)command.count, command.data);
        ch = getch();
        switch(ch){
            case ctrl('q'):  // quit 
                QUIT = true;
                break;
            case ENTER: // enter 
                line ++;             
                mvprintw(line, 0, "`command` is not recognized as internal or external");
                line++;  
                DA_APPEND(&command_his, command);
                free(command.data);
                command = (String){0};
                break;

            default: 
                DA_APPEND(&command, ch);
                break;
        }
    }

    refresh();
    endwin();

    return 0;
}