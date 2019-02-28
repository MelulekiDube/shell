#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int memory_alloc_error(){
	fprintf(stderr, "lsh: allocation error\n");
	exit(EXIT_FAILURE);
}

/*
	Function declaration
*/
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

char *builtin_str[]={
	"cd",
	"help",
	"exit"
};

/*
	Array of function pointers that take in char** and return int
*/
int (*builtin_func[]) (char **) ={
	&lsh_cd,
	&lsh_help,
	&lsh_exit
};

/*

*/
int lsh_num_builtins(){
	return sizeof(builtin_str)/sizeof(char*);
}



/* 
	Builtin function implementations.
*/
int lsh_cd(char **args){
	if(args[1]==NULL){
		fprintf(stderr, "lsh: expected argumet to \"cd\"\n");
	}else{
		if(chdir(args[1])!=0)
			perror("lsh");
	}
	return 1;
}

/*
	Definition of the help function
*/
int lsh_help(char **args){
	int i;
	printf("Meluleki Dube Shell\n");
	printf("Type the program names and arguments, then hit enter.\n");
	printf("The following are built in:\n");
	
	for(i = 0; i< lsh_num_builtins(); ++i){
		printf(" %s\n", builtin_str[i]);
	}
	
	printf("Use the man command for info on other prorgams.\n");
	return 1;
}

/*
	Definition of the exit function
*/
int lsh_exit(char **_){
	return 0;
}

//move readline part to new function to tidy code up
#define LSH_RL_BUFFERSIZE 1024
char *lsh_read_line(void){
	int bufsize = LSH_RL_BUFFERSIZE;
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
		}else{
			buffer[position] = c;
		}
		++position;
		
		//if we exceed the buffer, reallocate.
		if(position>=bufsize){
			bufsize += LSH_RL_BUFFERSIZE;
			buffer = realloc(buffer, bufsize);
			if(!buffer)
				memory_alloc_error();
		}
	}
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELM " \t\r\n\a"
char **lsh_split_line(char *line){
	int bufsize = LSH_RL_BUFFERSIZE, pos = 0;
	char **tokens = malloc(bufsize* sizeof(char*));
	char *token;
	if(!tokens)
		memory_alloc_error();
	
	token = strtok(line, LSH_TOK_DELM);//returns the next token
	while(token){
		tokens[pos] = token;
		++pos;
		
		if(pos >= bufsize){
			bufsize += LSH_RL_BUFFERSIZE;
			tokens = realloc(tokens, bufsize*sizeof(char*));
			if(!tokens)
				memory_alloc_error();
		}
		
		token = strtok(NULL, LSH_TOK_DELM);
	}
	
	tokens[pos] = NULL;
	return tokens;
	
}

int lsh_launch(char **args){
	pid_t pid, wpid;
	int status;
	
	pid = fork();//pid =child
	if(pid==0){
		//child process
		if(execvp(args[0], args)==-1){
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	}else if(pid<0){
		perror("lsh");
	}else{
		//parent process
		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}


/*
	Definition of the lsh_execute function
*/
int lsh_execute(char **args){
	int i;
	if(args[0] == NULL){
		//empty command
		return 1;
	}
	
	for(i=0;i<lsh_num_builtins();++i){
		if(strcmp(args[0], builtin_str[i])==0){
			return (*builtin_func[i])(args);
		}
	}	
	return lsh_launch(args);
}

void lsh_loop(){
	char *line;
	char **args;
	int status;
	
	
	do{
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);
		
		free(line);
		free(args);
	}while(status);
}

int main(int argc, char **argv){
	//load config files
	
	//Run command loop.
	lsh_loop();
	
	//perform any shutdown or cleanup
	
	return EXIT_SUCCESS;
}