#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

#include "shell.h"
#define ctrl(x) ((x) & 0x1f)

static const char SHELL_PROMPT[] = "[my_shell]$ ";
static const Repl REPL_INIT = { .is_running = true };



#define export __attribute__((visibility("default")))

bool export shell_repl_initialize(Repl *repl) {
    *repl = REPL_INIT;
	initscr();
	raw();
	noecho();

	// TODO: change pad size on resize
	size_t width;
	size_t height;
	getmaxyx(stdscr, height, width);
	repl->buffer = newpad(height*4, width);

	keypad(repl->buffer, TRUE);
	scrollok(repl->buffer, TRUE);
    return true;
}


void export shell_cleanup(Repl *repl)
{
	delwin(repl->buffer);
	noraw();
	endwin();
	echo();
}


static
void clear_line(WINDOW* window, size_t line, size_t width) {
	for (size_t i = SSTR_LEN(SHELL_PROMPT); i < width - SSTR_LEN(SHELL_PROMPT); i++)
		mvwprintw(window, line, i, " ");
}


bool export shell_readline(Repl *repl)
{
	String command = repl->input;
	size_t buf_height;
	size_t buf_width;
	size_t height;
	size_t width;
	size_t line = repl->line;
	size_t position = 0;
	size_t command_max = repl->command_his.count;
	int ch;
	size_t top_row = 0;
	
	command.count = 0;
	while (true) {

		getmaxyx(repl->buffer, buf_height, buf_width);
		if(line >= buf_height/2) {
			wresize(repl->buffer, buf_height*2, buf_width);
		}

		getmaxyx(stdscr, height, width);
		clear_line(repl->buffer, line, width);
	
		if(line >= height) top_row = line - height+1;

		mvwprintw(repl->buffer, line, 0, "%s%.*s", SHELL_PROMPT, (int)command.count, command.data);

		if (position > command.count)
			position = command.count;

		wmove(repl->buffer, line, SSTR_LEN(SHELL_PROMPT) + position);
		prefresh(repl->buffer, top_row, 0, 0, 0, height-1, width-1);

		ch = wgetch(repl->buffer);
		switch (ch) {
			case KEY_RESIZE:
				getmaxyx(stdscr, height, width);
				wresize(repl->buffer, buf_height, width);
				break;
			case KEY_ENTER:
			case '\n': {
				line += 1 + (command.count+SSTR_LEN(SHELL_PROMPT))/width;
				if (command.count == 0)
					continue;
				repl->line = line;
				repl->input = command;
			} return true;
			case ctrl('l'):
				wclear(repl->buffer);
				wrefresh(repl->buffer);
				line = 0;
				break;
			case ctrl('d'):
			case ctrl('q'):
				repl->is_running = false;
				return true;
			case ctrl('a'):
				position = 0;
				break;
			case KEY_LEFT:
			case ctrl('b'):
				position -= 1;
				break;
			case ctrl('e'):
				position = command.count;
				break;
			case ctrl('k'):
				DA_CHECK_BOUNDS(&repl->clipboard, command.count-position, command.count*2);
				strncpy(repl->clipboard.data, &command.data[position], sizeof(char)*(command.count-position));
				repl->clipboard.count = command.count-position;
				command.count = position;
				break;
			case KEY_RIGHT:
			case ctrl('f'):
				position += 1;
				break;
			case ctrl('u'):
				DA_CHECK_BOUNDS(&repl->clipboard, position, command.count*2);
				memcpy(repl->clipboard.data, command.data, position * sizeof(char));
				memmove(command.data, &command.data[position], (command.count - position) * sizeof(char));
				command.count -= position;
				repl->clipboard.count = position;
				position = 0;
				break;
			case ctrl('y'): 
				DA_CHECK_BOUNDS(&command, command.count+repl->clipboard.count, command.capacity*2);
				memmove(&command.data[position+repl->clipboard.count], &command.data[position], 
						command.count - position);
				strncpy(&command.data[position], repl->clipboard.data, sizeof(char)*repl->clipboard.count);
				command.count += repl->clipboard.count;
				position = command.count;
				command.data[command.count+1] = '\0';
				break;
			case ctrl('c'):
				line++;
				command.count = 0;
				break;
			case KEY_BACKSPACE:
				if (command.count > 0)
					command.data[--command.count] = '\0';
				break;
			case KEY_UP:
				if (repl->command_his.count > 0) {
					repl->command_his.count--;
					command = repl->command_his.data[repl->command_his.count];
					position = command.count;
				}
				break;
			case KEY_DOWN:
				if (repl->command_his.count < command_max) {
					repl->command_his.count++;
					command = repl->command_his.data[repl->command_his.count];
					position = command.count;
				}
				break;
			default:
				DA_APPEND(&command, ch);
				memmove(&command.data[position+1], &command.data[position], command.count - 1 - position);
				command.data[position++] = ch;
				break;
		}
	}
}

bool export shell_evaluate(Repl *repl)
{
	char **args = parse_command(str_to_cstr(repl->input));

	if (args != NULL) {
		handle_command(repl, args, &repl->line);
		DA_APPEND(&repl->command_his, repl->input);
	}
	repl->input = (String){ 0 };
	return true;
}

int shell_repl_run(void)
{
    Repl repl = { 0 };

    if (!shell_repl_initialize(&repl))
        return EXIT_FAILURE;
    while (repl.is_running) {
        if (!shell_readline(&repl))
            break;
        if (!shell_evaluate(&repl))
            break;
    }
	shell_cleanup(&repl);   
	return EXIT_SUCCESS;
}