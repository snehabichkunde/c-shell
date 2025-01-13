# c_shell

`my_shell` is a POSIX-compliant shell written in C, designed to provide a functional and user-friendly terminal experience. It supports basic shell operations, built-in commands, and keyboard shortcuts for an enhanced workflow. The shell is built using `ncurses` for an interactive REPL (Read-Eval-Print Loop) interface.

---

## Quick Start

1. **Build the Project**
   ```bash
   make


## Keyboard Shortcuts

The following shortcuts are supported for efficient shell usage:

| Shortcut | Action                              |
|----------|-------------------------------------|
| `CTRL+A` | Move to the beginning of the line   |
| `CTRL+B` | Move the cursor backward            |
| `CTRL+D` | Quit the shell                      |
| `CTRL+E` | Move to the end of the line         |
| `CTRL+F` | Move the cursor forward             |
| `CTRL+K` | Cut everything after the cursor     |
| `CTRL+L` | Clear the screen, keeping the command |
| `CTRL+U` | Cut everything before the cursor    |
| `CTRL+Y` | Paste cut content                   |
| `CTRL+R` | Reverse search through command history |
| `CTRL+L` | Clear, keeping current command      |
| `CTRL+U` | Cut everything before cursor        |


## Builtins
 exit
 cd
 history