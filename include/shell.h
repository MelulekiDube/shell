//
// Created by Meluleki Dube on 14/02/2021.
//

#pragma once

#include <limits.h>
#include <assert.h>

#define TEST
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

