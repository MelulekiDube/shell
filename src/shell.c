#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "shell.h"

Array_(string, builtin_str) = {
        "cd",
        "help",
        "exit"
};

/*-------------------------- Array of function pointers that take in char** and return int --------------------------*/
FUNCTION_PTR_A(int, builtin_func, command) = {
        &msh_cd,
        &msh_help,
        &msh_exit
};


/*-------------------------- Impl of the help function --------------------------*/
int msh_cd(command cmd) {
    if (!cmd.argc) {
        fprintf(stderr, "msh: expected argument to \"cd\"\n");
    } else if (cmd.argc > 2) {
        fprintf(stderr, "msh: expected 1 argument to \"cd\"\n");
    } else{
        if(cmd.argc == 1)
            cmd.argv[1] = "";
        if (chdir(cmd.argv[1]) != 0)
            perror("msh");
        else
            update_dir();
    }
    return 1;
}

int msh_help(command _) {
    int i;
    printf("Meluleki Dube Shell\n");
    printf("Type the program names and arguments, then hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < MSH_NUM_BUILTINS; ++i) {
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for info on other programs.\n");
    return 1;
}

int msh_exit(command _) {
    return 0;
}
/*-------------------------- End of Built in functions  --------------------------*/


/*-------------------------- Getting User input and manipulating it --------------------------*/
string get_command_line() {
    int c;
    int buffer_size = 0;
    int position = 0;
    string buffer = NULL;
    while (1) {
        //if we exceed the buffer, reallocate.
        if (!buffer || position >= buffer_size) {
            buffer_size += MSH_RL_BUFFER_SIZE;
            buffer = MORE_SPACE(char, buffer, buffer_size);
            if (!buffer) {
                EXIT_REPORT_ERROR(1, "Buffer too small to hold the required size");
            }
#ifdef DEBUG
            printf("Buffer allocated and pointer points to: %p\n", buffer);
#endif
        }
        //read chars
        c = getchar();
        if (c == EOF || c == '\n') {
            //place the string terminating char at eof or \n
            buffer[position] = '\0';
            break;
        } else {
            buffer[position] = (char) c;
        }
        ++position;
    }
#ifdef DEBUG
    printf("The command read was:\n%s\n", buffer);
#endif
    return buffer;
}

int parse_command(string input, command *cmd) {
    if (!input || !cmd) return 0;
    int size = 0;
    if (!cmd->command_name) {
        OUT_OF_MEMORY_ERROR("command.command_name");
    }
    string token = strtok(input, WHITESPACE);//returns the next token
    while (token && size < MAX_ARGS) {
        string temp = create(char, MAX_ARGS_LEN);
        if (!cmd->command_name) {
            OUT_OF_MEMORY_ERROR("temp");
        }
        strcpy(temp, token);
        if (size == 0) {
            cmd->command_name = temp;
        }
        cmd->argv[size++] = temp;
        token = strtok(NULL, WHITESPACE);
    }
    if (size == 0) return 0;
    cmd->argv[size] = NULL;
    cmd->argc = size;
    return 1;
}

/*-------------------------- End of Getting User input and manipulating it --------------------------*/

int msh_launch(command cmd) {
    int status;
    pid_t pid = 10;

    pid = fork();//pid =child
    if (pid == 0) {
        //child process
        if (execvp(cmd.command_name, cmd.argv) == -1) {
            perror("msh");
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    } else if (pid < 0) {
        perror("msh");
    } else {
        //parent process
        do {
            waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
    return 1;
}


/*
	Definition of the msh_execute function
*/
int msh_execute(command cmd) {
    int i;
    if (cmd.argc == 0) {
        //empty command
        return 1;
    }

    for (i = 0; i < MSH_NUM_BUILTINS; ++i) {
        if (strcmp(cmd.command_name, builtin_str[i]) == 0) {
            return (*builtin_func[i])(cmd);
        }
    }
    return msh_launch(cmd);
}

void print_prompt() {
    update_dir();
    printf("%s> ", current_dir);
}

void update_dir() {
    getwd(current_dir);
}

#ifndef TEST
int main(int argc, char **argv) {
    int status;
    string line;
    command cmd;
    status  =1;
    do {
        print_prompt();
        line = get_command_line();
        if (line && line[0]) {
            parse_command(line, &cmd);
            status = msh_execute(cmd);
            if (cmd.argv[0]) {
                CLEAN_ARRAY(cmd.argv, cmd.argc);
            }
            free(line);
        }
    } while (status);
    return EXIT_SUCCESS;
}
#endif