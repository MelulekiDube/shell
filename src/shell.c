#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <readline/readline.h>

#define     LINE_LEN                                        80
#define     MAX_ARGS                                        64
#define     MAX_ARGS_LEN                                    16
#define     MAX_COMMAND_LEN                                 100
#define     MAX_PATHS                                       64
#define     MAX_PATH_LEN                                    96
#define     MSH_RL_BUFFER_SIZE                              1024
#define     MSH_NUM_BUILTINS                                3
#define     WHITESPACE                                      " .,\t\n"
#define     create(type, size)                              ((type*)malloc( sizeof(type)*size))
#define     Array(type, name, size)                         type name[size]
#define     Array_(type, name)                              type name[]
#define     Function_ptr(return_type, function_name, ...)   return_type (*function_name)(__VA_ARGS__)
#define     FUNCTION_PTR_A(return_type, arr_name, ...)      return_type (*arr_name[])(__VA_ARGS__)
#define     CLEAN_ARRAY(array, size)                        for(int i=0; i<size; ++i)  free(array[i]); \
                free(array)
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

static string current_dir;


void    print_prompt();
void    update_dir();
int     parsePath(Array_(string, dirs)); /*builds the paths dirs from the path variable*/
string  look_up_path(string *argv, string *dir); /*searches the dir for the directories to check if they have file with arv[0]*/
string  get_command_line();
int     parse_command(string, command*);

/*
	Built in Function declaration
*/
int msh_cd(command cmd);

int msh_help(command);

int msh_exit(command);

Array_(string, builtin_str) = {
        "cd",
        "help",
        "exit"
};

/*
	Array of function pointers that take in char** and return int
*/
FUNCTION_PTR_A(int, builtin_func, command) = {
        &msh_cd,
        &msh_help,
        &msh_exit
};

/*-------------------------- Definition of the help function --------------------------*/
int msh_cd(command cmd){
    if(!cmd.argc){
        fprintf(stderr, "msh: expected argument to \"cd\"\n");
    }else if(cmd.argc > 1) {
        fprintf(stderr, "msh: expected 1 argument to \"cd\"\n");
    }else {
        if (chdir(cmd.argv[1]) != 0)
            perror("msh");
        else
            update_dir();
    }
    return 1;
}

int msh_help(command _){
    int i;
    printf("Meluleki Dube Shell\n");
    printf("Type the program names and arguments, then hit enter.\n");
    printf("The following are built in:\n");

    for(i = 0; i < MSH_NUM_BUILTINS; ++i){
        printf(" %s\n", builtin_str[i]);
    }

    printf("Use the man command for info on other programs.\n");
    return 1;
}

int msh_exit(command _){
    return 0;
}
/*-------------------------- End of Built in functions * --------------------------*/

string get_command_line() {
    int c;
    int buffer_size = MSH_RL_BUFFER_SIZE;
    int position = 0;
    string buffer = create(char, buffer_size);
    if (!buffer) {
        OUT_OF_MEMORY_ERROR("buffer");
    }
    while (1) {
        //read chars
        c = getchar();
        if (c == EOF || c == '\n') {
            //place the string terminating char at eof or \n
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        ++position;

        //if we exceed the buffer, reallocate.
        if (position >= buffer_size) {
            buffer_size += MSH_RL_BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if (!buffer){
                EXIT_REPORT_ERROR(1, "Buffer too small to hold the required size");
            }
        }
    }
}

#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELM " \t\r\n\a"

char **msh_split_line(char *line) {
    int bufsize = MSH_RL_BUFFER_SIZE, pos = 0;
    char **tokens = malloc(bufsize * sizeof(char *));
    char *token;
    if (!tokens) {
        OUT_OF_MEMORY_ERROR("token");
    }
    token = strtok(line, MSH_TOK_DELM);//returns the next token
    while (token) {
        tokens[pos] = token;
        ++pos;

        if (pos >= bufsize) {
            bufsize += MSH_RL_BUFFER_SIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                OUT_OF_MEMORY_ERROR("token");
            }
        }

        token = strtok(NULL, MSH_TOK_DELM);
    }

    tokens[pos] = NULL;
    return tokens;
}

int parse_command(string input, command *cmd){
    int size = 0;

    string token = strtok(input, MSH_TOK_DELM) ;//returns the next token
    if(!token) return 0;
    cmd->command_name = create(char, MAX_COMMAND_LEN);
    if(!cmd->command_name){
        OUT_OF_MEMORY_ERROR("command.command_name");
    }
    strcpy(cmd->command_name, token);
    token = strtok(NULL, MSH_TOK_DELM);
    while (token && size< MAX_ARGS){
        string temp = create(char, MAX_ARGS_LEN);
        if(!cmd->command_name){
            OUT_OF_MEMORY_ERROR("temp");
        }
        strcpy(temp, token);
        cmd->argv[size++]  = temp;
        token = strtok(NULL, MSH_TOK_DELM);
    }
    cmd->argc=size;
    return 1;
}


int msh_launch(command cmd) {
    pid_t pid, wpid;
    int status;

    pid = fork();//pid =child
    if (pid == 0) {
        //child process
        if (execvp(cmd.command_name, cmd.argv) == -1) {
            perror("msh");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        perror("msh");
    } else {
        //parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
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
    printf("%s> ", current_dir);
}

void msh_loop() {
    string  line;
    int     status;
    command cmd;

    do {
        print_prompt();
        line = readline("");
        parse_command(line, &cmd);
        status = msh_execute(cmd);

        CLEAN_ARRAY(cmd.argv, cmd.argc);
        free(line);
    } while (status);
}

void update_dir() {
    char cwd[PATH_MAX];
    if (!(current_dir = getcwd(cwd, sizeof(cwd))))
        *cwd = '\0';
}
#define TEST
#ifdef  TEST
// int     parse_command(string, command*);
void test_parse_command(){
    command cmd;
    string input = create(char, LINE_MAX);
    strcpy(input, "cd temp");

    int expect_perfect = parse_command(input, &cmd);
    int match_name = strcmp(cmd.command_name, "cd");
    int match_argv = strcmp(cmd.argv[0], "temp");

    printf("Perfect command.name: %s\ncommand.argv[0]: %s\ncommand.argc: %d\n", cmd.command_name, cmd.argv[0], cmd.argc);
    assert(expect_perfect==1);
    assert(match_name==0);
    assert(match_argv==0);
    assert(cmd.argc==1);

    strcpy(input, "find ./ -name \\*.html -printf '%CD\\t%p\\n' | grep \"03/10/08\" | awk '{print $2}' | xargs -t -i mv {} temp/");
    expect_perfect = parse_command(input, &cmd);

    printf("Perfect command.name: %s\ncommand.argv[0]: %s\ncommand.argc: %d\n", cmd.command_name, cmd.argv[0], cmd.argc);
    match_name = strcmp(cmd.command_name, "find");
    assert(expect_perfect==1);
    assert(match_name==0);
    assert(cmd.argc==19);


    memset(input, 0, LINE_MAX);
    int expect_fail = parse_command(input, &cmd);
    assert(expect_fail == 0);

}
#endif

int main(int argc, char **argv) {
    test_parse_command();
#undef TEST
    /*
    //load config files
    update_dir();
    //Run command loop.
    msh_loop();
    //perform any shutdown or cleanup
    */
    return EXIT_SUCCESS;
}