#include<stdio.h>
#include<stdbool.h>
#include<stdlib.h>
#include<string.h>
#include <errno.h>   //represent error codes.
#include<ncurses.h>
#include<unistd.h>
#include<sys/wait.h>
#include <curses.h>

#define ctrl(x) ((x)& 0x1f)
#define SHELL "[c-shell]~ "
#define ENTER 10
#define UP_ARROW 259
#define DOWN_ARROW 258
#define LEFT_ARROW 260
#define RIGHT_ARROW 261
#ifndef KEY_BACKSPACE
#define KEY_BACKSPACE 127
#endif

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

// Function declarations
void handle_command(char **args, size_t *line);
char *str_to_cstr(String str);
char **parse_command(char *command);


void clear_line(size_t line, size_t width) {
    for (size_t i = 0; i < width; i++) {
        mvprintw(line, i, " ");  // Clear the entire line by printing spaces
    }
}


void handle_command(char **args, size_t *line) {
    char buf[4096] = {0};

    int filedes[2];
    if (pipe(filedes) < 0) {
        mvprintw(*line, 0, "error %s", strerror(errno));
        return;
    }

    int status;
    int pid = fork();

    if (pid < 0) { // error
        mvprintw(*line, 0, "error %s", strerror(errno));
        return;
    } else if (!pid) { // child process
        close(filedes[0]);
        if (dup2(filedes[1], STDOUT_FILENO) < 0) {
            printf("%s\n", strerror(errno));
        }
        close(filedes[1]);

        if (execvp(args[0], args) < 0) {
            printf("%s\n", strerror(errno));
        }
        exit(1);
    } else { // parent process
        close(filedes[1]);

        int nbytes;
        while ((nbytes = read(filedes[0], buf, sizeof(buf) - 1)) != 0) {
            mvprintw(*line, 0, "%s", buf);
            for (size_t i = 0; buf[i] != '\0'; i++) {
                if (buf[i] == '\n') {
                    *line += 1;
                }
            }
            refresh();
            memset(buf, 0, sizeof(buf));
        }

        int wpid = waitpid(pid, &status, 0);
        while (!WIFEXITED(status) && !WIFSIGNALED(status)) {
            wpid = waitpid(pid, &status, 0);
        }
        close(filedes[0]);
        refresh();
    }
}




char *str_to_cstr(String str)
{
    char * cstr = malloc(sizeof(char)*str.count +1);
    memcpy(cstr, str.data, sizeof(char)*str.count);
    cstr[str.count] = '\0';
    return cstr;
}

char **parse_command(char *command) {
    char *cur = strtok(command, " ");
    if (cur == NULL) {
        return NULL;
    }

    size_t args_s = 8;
    char **args = malloc(sizeof(char*) * args_s);
    size_t args_cur = 0;

    while (cur != NULL) {
        if (args_cur >= args_s) {
            args_s *= 2;
            args = realloc(args, sizeof(char*) * args_s);
        }

        args[args_cur++] = cur;
        cur = strtok(NULL, " ");
    }

    args[args_cur] = NULL;

    return args;
}

int main()
{
    initscr();  // Initializes the ncurses environment for screen manipulation
    raw();      // Disables line buffering, allowing real-time input capture.
    noecho();   // Prevents typed characters from being automatically displayed.
    keypad(stdscr, TRUE); // Enable arrow key processing
    int ch;
    int history_index = -1;
    String command = {0};
    Strings command_his = {0};
    DA_APPEND(&command_his, command);
    
    int command_pos = 0;  

    bool QUIT = false; 
    size_t line = 0;
    size_t height = 0;
    size_t width = 0;
    while(!QUIT){
        getmaxyx(stdscr, height, width);
        clear_line(line, width);
        mvprintw(line, 0, SHELL);
        mvprintw(line , 0 + sizeof(SHELL) - 1, "%.*s", (int)command.count, command.data);

        move(line, sizeof(SHELL)-1+command_pos);

        ch = getch();
        switch(ch){
            case ctrl('q'):  // quit 
                QUIT = true;
                break;

            case ENTER: // enter 
                line++;  
                clear_line(line, width); 
                command_pos = 0;  
                char **args = NULL;  
                if(command.count> 0){
                    args = parse_command(str_to_cstr(command));
                }
                mvprintw(line, command.count, "\n\r");
                if(args!=NULL){
                    handle_command(args, &line);        
                    DA_APPEND(&command_his, command);
                    free(args);
                }
                command = (String){0};  // Reset command
                history_index = -1;  // Reset the history index
                break;
            case ctrl('c'):
                line++;
                command = (String){0}; 
                break;
            case KEY_BACKSPACE:
            case 8:
                if(command.count > 0) command.data[--command.count] = '\0';
                break;
            case LEFT_ARROW:
                if(command_pos > 0) {
                command_pos--;
                }
                break;
            case RIGHT_ARROW:
                if(command_pos < command.count) {
                    command_pos++;
                }
                break;
            case UP_ARROW:
                if (command_his.count > 0) {
                    if (history_index == -1) {
                        // Start at the last history command
                        history_index = 0;
                    } else if (history_index < (int)command_his.count - 1) {
                        // Move one step back in history
                        history_index++;
                    }
                    // Fetch the command from history
                    clear_line(line, width);  // Clear the current line
                    command = command_his.data[command_his.count - 1 - history_index];
                    command_pos = command.count;
                }
                break;

            case DOWN_ARROW:
                if (history_index > -1) {
                    history_index--;
                    clear_line(line, width); 
                    command = command_his.data[command_his.count - 1 - history_index];
                } else {
                    clear_line(line, width); 
                    command = (String){0};  // Clear the current command when reaching the newest one
                }
                command_pos = command.count;
                break;
            

            default: 
                DA_APPEND(&command, ch);
                command_pos++;
                break;
        }
    }

    if (command.data != NULL) {
        free(command.data);  // Free the last command data if allocated
    }
    if (command_his.data != NULL) {
        free(command_his.data);  // Free history data
    }

    refresh();
    endwin();

    return 0;
}