#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <limits.h>
#include <readline/readline.h>
#include <readline/history.h>

static char *current_dir;
void update_dir();
static int typed_chars = 0;
int memory_alloc_error(){
	fprintf(stderr, "msh: allocation error\n");
	exit(EXIT_FAILURE);
}

/*
	Function declaration
*/
int msh_cd(char **args);
int msh_help(char **args);
int msh_exit(char **args);

char *builtin_str[]={
	"cd",
	"help",
	"exit"
};

/*
	Array of function pointers that take in char** and return int
*/
int (*builtin_func[]) (char **) ={
	&msh_cd,
	&msh_help,
	&msh_exit
};

/*

*/
int msh_num_builtins(){
	return sizeof(builtin_str)/sizeof(char*);
}



/*
	Builtin function implementations.
*/
int msh_cd(char **args){
	if(args[1]==NULL){
		fprintf(stderr, "msh: expected argumet to \"cd\"\n");
	}else{
		if(chdir(args[1])!=0)
			perror("msh");
		else
			update_dir();
	}
	return 1;
}

/*
	Definition of the help function
*/
int msh_help(char **args){
	int i;
	printf("Meluleki Dube Shell\n");
	printf("Type the program names and arguments, then hit enter.\n");
	printf("The following are built in:\n");

	for(i = 0; i< msh_num_builtins(); ++i){
		printf(" %s\n", builtin_str[i]);
	}

	printf("Use the man command for info on other prorgams.\n");
	return 1;
}

/*
	Definition of the exit function
*/
int msh_exit(char **_){
	return 0;
}

//move readline part to new function to tidy code up
#define MSH_RL_BUFFERSIZE 1024
char *msh_read_line(void){

	int bufsize = MSH_RL_BUFFERSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char)*bufsize);
	int c;
	if(!buffer)
		memory_alloc_error();
	while(1){
		//read chars
		c = getchar();
		if(c==EOF || c=='\n'){
			//place the string terminating char at eof or \n
			buffer[position] = '\0';
			return buffer;
		}else if(c==8){
            if(c>0){
                --c;
                buffer[position] = c;
            }
        }else{
            ++typed_chars;
			buffer[position] = c;
		}
		++position;

		//if we exceed the buffer, reallocate.
		if(position>=bufsize){
			bufsize += MSH_RL_BUFFERSIZE;
			buffer = realloc(buffer, bufsize);
			if(!buffer)
				memory_alloc_error();
		}
	}
}

#define MSH_TOK_BUFSIZE 64
#define MSH_TOK_DELM " \t\r\n\a"
char **msh_split_line(char *line){
	int bufsize = MSH_RL_BUFFERSIZE, pos = 0;
	char **tokens = malloc(bufsize* sizeof(char*));
	char *token;
	if(!tokens)
		memory_alloc_error();

	token = strtok(line, MSH_TOK_DELM);//returns the next token
	while(token){
		tokens[pos] = token;
		++pos;

		if(pos >= bufsize){
			bufsize += MSH_RL_BUFFERSIZE;
			tokens = realloc(tokens, bufsize*sizeof(char*));
			if(!tokens)
				memory_alloc_error();
		}

		token = strtok(NULL, MSH_TOK_DELM);
	}

	tokens[pos] = NULL;
	return tokens;

}

int msh_launch(char **args){
	pid_t pid, wpid;
	int status;

	pid = fork();//pid =child
	if(pid==0){
		//child process
		if(execvp(args[0], args)==-1){
			perror("msh");
		}
		exit(EXIT_FAILURE);
	}else if(pid<0){
		perror("msh");
	}else{
		//parent process
		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}


/*
	Definition of the msh_execute function
*/
int msh_execute(char **args){
	int i;
	if(args[0] == NULL){
		//empty command
		return 1;
	}

	for(i=0;i<msh_num_builtins();++i){
		if(strcmp(args[0], builtin_str[i])==0){
			return (*builtin_func[i])(args);
		}
	}
	return msh_launch(args);
}

void msh_loop(){
	char *line;
	char **args;
	int status;


	do{
		printf("%s> ",current_dir);
		line = msh_read_line();
		args = msh_split_line(line);
		status = msh_execute(args);

		free(line);
		free(args);
	}while(status);
}

void update_dir(){
	char cwd[PATH_MAX];
	if(!(current_dir = getcwd(cwd, sizeof(cwd))))
		*cwd = '\0';
}

int main(int argc, char **argv){
	//load config files
	update_dir();
	//Run command loop.
	msh_loop();
	//perform any shutdown or cleanup
	return EXIT_SUCCESS;
}