#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <assert.h>

#define     LINE_LEN                                        80
#define     MAX_ARGS                                        64
#define     MAX_ARGS_LEN                                    16
#define     MAX_COMMAND_LEN                                 100
#define     MAX_PATHS                                       64
#define     MAX_PATH_LEN                                    96
#define     MSH_RL_BUFFER_SIZE                              1024
#define     MSH_NUM_BUILTINS                                3
#define     WHITESPACE                                      " \t\r\n\a"
#define     create(type, size)                              malloc( sizeof(type)*size)
#define     Array(type, name, size)                         type name[size]
#define     Array_(type, name)                              type name[]
#define     Function_ptr(return_type, function_name, ...)   return_type (*function_name)(__VA_ARGS__)
#define     MORE_SPACE(type, name, size)                    name ? realloc(name, size) : create(type, size)
#define     FUNCTION_PTR_A(return_type, arr_name, ...)      return_type (*arr_name[])(__VA_ARGS__)
#define     CLEAN_ARRAY(array, size)                        for(int i=0; i<size; ++i)  free(array[i])
#define     EXIT_REPORT_ERROR(error_code, error_str)        fprintf(stderr, "md_shell error code: %d\n error string: %s", error_code, error_str); \
                exit(EXIT_FAILURE)
#define     OUT_OF_MEMORY_ERROR(variable_name)              fprintf(stderr, "%s.%d Malloc did not return adequate space for %s", __FILE__, __LINE__, variable_name); \
                 exit(EXIT_FAILURE)

typedef char *string;

#ifndef NULL
#define NULL ( (void *) 0)
#endif

typedef struct command_t {
    string command_name;
    int argc;
    Array(string, argv, MAX_ARGS);
} command;

static Array(char, current_dir, PATH_MAX);

/*-------------------------- Def Functions to keep print the current dir to the console --------------------------*/
void print_prompt();

void update_dir();
/*-------------------------- End of Def for fun to keep print the current dir to the console--------------------------*/

/*-------------------------- Definition Functions deal with the environment variables --------------------------*/
int parsePath(Array_(string, dirs)); /*builds the paths dirs from the path variable*/
string
look_up_path(string *argv, string *dir); /*searches the dir for the directories to check if they have file with arv[0]*/
/*-------------------------- End of Definition for functions to process the environment vars --------------------------*/

/*-------------------------- Definition for functions to get user input and parse it  --------------------------*/
string get_command_line();

int parse_command(string, command *);
/*-------------------------- End of Def for functions to get and minipulate user input  --------------------------*/

/*-------------------------- Definition for execution functions --------------------------*/
int msh_launch(command cmd);

int msh_execute(command cmd);

int scan_dir(command cmd); /* will scan the current dir to see if we have an executable program we can run*/
/*-------------------------- End of definition for execution functions --------------------------*/

/*-------------------------- Definition of the help function --------------------------*/
int msh_cd(command cmd);

int msh_help(command);

int msh_exit(command);
/*-------------------------- End of Definition of the help function --------------------------*/

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

#ifdef  TEST
// int     parse_command(string, command*);
void test_parse_command(){
    command cmd;
    string input = create(char, LINE_MAX);
    strcpy(input, "cd temp");

    int expect_perfect = parse_command(input, &cmd);
    int match_name = strcmp(cmd.command_name, "cd");
    int match_name_2 = strcmp(cmd.argv[0], "cd");
    int match_argv = strcmp(cmd.argv[1], "temp");

    printf("command.name: %s\ncommand.argv[0]: %s\ncommand.argc: %d\n", cmd.command_name, cmd.argv[0], cmd.argc);
    assert(expect_perfect==1);
    assert(match_name==0);
    assert(match_argv==0);
    assert(match_name_2==0);
    assert(cmd.argc==2);

    strcpy(input, "find ./ -name \\*.html -printf '%CD\\t%p\\n' | grep \"03/10/08\" | awk '{print $2}' | xargs -t -i mv {} temp/");
    expect_perfect = parse_command(input, &cmd);

    printf("command.name: %s\ncommand.argv[0]: %s\ncommand.argc: %d\n", cmd.command_name, cmd.argv[0], cmd.argc);
    match_name = strcmp(cmd.command_name, "find");
    assert(expect_perfect==1);
    assert(match_name==0);
    assert(cmd.argc==20);


    memset(input, 0, LINE_MAX);
    int expect_fail = parse_command(input, &cmd);
    assert(expect_fail == 0);

}

int main(){
    test_parse_command();
}
#else

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