//Name: Aaron Cohen
//FSUID: ajc17d

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/times.h>
#include <time.h>
#include <errno.h>

void myexit(char** cmd);
void mycd(char** cmd, int n);
void mypwd(char** cmd);
void mytree(char** cmd, int n);
void mytime(char** cmd, int n);
void mymtimes(char** cmd, int n);
void mytimeout(char** cmd, int n);
void myexecute(char** cmd);
int myexecutepipe(char** cmd1, char** cmd2);
int myexecutepipe2(char** cmd1, char** cmd2, char** cmd3);
void myexecuteredir(char** cmd, int n);
void printTree(char* path, int tabs);
int fillParams(char* command, char** params);

int main(){
	int numParams = 0;
	int i;
	int charIndex;
	int tool = 0;
	bool hasRedir = false;
	bool hasPipe = false;
	bool builtin = false;
	char command[40];
	char* pipe = "|";
	char* redir1 = ">";
	char* redir2 = "<";
	char* params[20];
	char* cmd1[20];
	char* cmd2[20];
	pid_t queue[10];

		//loop to run commands
	while(true){
		printf("$ ");
		if(fgets(command, sizeof(command), stdin) == NULL){
			break;	
		}
	
		if(command[strlen(command)-1] == '\n'){
			command[strlen(command)-1] = '\0';
		}
		
		//Split input
		int br = fillParams(command, params);

		for(i = 0; i < 20; i++)
			if(params[i] != NULL)
				numParams++;
		numParams = br;

		for(i = 0; i < br; i++){
			if(strcmp(params[i], pipe) == 0){
				hasPipe = true;
				charIndex = i;
				break;
			}
			if(strcmp(params[i], redir1) == 0 || strcmp(params[i], redir2) == 0){
				hasRedir = true;
				charIndex = i;
				break;
			}	
		}
		

		//Calls to functions
		if(strcmp(params[0], "myexit") == 0){
			tool = 1;
			builtin = true;
		}else if(strcmp(params[0], "mycd") == 0){
			tool = 2;
			builtin = true;
		}else if(strcmp(params[0], "mypwd") == 0){
			tool = 3;
			builtin = true;
		}else if(strcmp(params[0], "mytree") == 0){
			tool = 4;
			builtin = true;
		}else if(strcmp(params[0], "mytime") == 0){
			tool = 5;
			builtin = true;
		}else if(strcmp(params[0], "mymtimes") == 0){
			tool = 6;
			builtin = true;
		}else if(strcmp(params[0], "mytimeout") == 0){
			tool = 7;
			builtin = true;
		}else{
			tool = 8;
			builtin = false;
		}

		switch(tool){
			case 1:
				myexit(params);
				break;
			case 2:
				mycd(params, numParams);
				break;
			case 3:
				mypwd(params);
				break;
			case 4:
				mytree(params, numParams);
				break;
			case 5:
				mytime(params, numParams);
				break;
			case 6: 
				mymtimes(params, numParams);
				break;
			case 7:
				mytimeout(params, numParams);
				break;
		}
		
		if(!hasPipe && !builtin){
			myexecute(params);
		}
		
		if(hasRedir){
			myexecuteredir(params, charIndex);
		}
		if(hasPipe){
			for(i = 0; i < charIndex; i++){
				cmd1[i] = params[i];
			}
			int j = 0;
			for(i = charIndex+1; i < br; i++){
				cmd2[j] = params[i];
				j++; 
			}
			if(!builtin){
				if(myexecutepipe(cmd1, cmd2) == 0){
					break;
				}
			}
		}
		
		tool = 0;
		numParams = 0;
		hasPipe = false;
		hasRedir = false;	
	}
	return 0;
}

		//Function to terminate toolkit
void myexit(char** cmd){
		//Fork and if statements 
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		kill(pid, SIGTERM);
		exit(0);
	}else{
		//Wait for child
		int status;
		waitpid(pid, &status, 0);
		exit(0);
	}
}

		//Function to change the directory of the toolkit
void mycd(char** cmd, int n){
		//Current path of the toolkit
	char* currPath = getenv("PWD");
	char* homePath = getenv("HOME");
	char* path;
	char* slash = "/";
	int fd[2];
	pipe(fd);
	pid_t pid = fork();
	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		if(n > 2 || n == 1){
			printf("Incorrect number of parameters for mycd\n");
			exit(1);
		}
		if(cmd[1][0] == '/'){
			setenv("PWD", cmd[1], 1);
			printf("%s\n", cmd[1]);	
		}else{
			strcat(currPath, slash);
			strcat(currPath, cmd[1]);
			setenv("PWD", currPath, 1);
			printf("%s\n", currPath);
		}
		
		//free(path);
		exit(1);

	}else{
		if(cmd[1][0] == '/'){
			setenv("PWD", cmd[1], 1);
			chdir(cmd[1]);
		}else{
			strcat(currPath, slash);
			strcat(currPath, cmd[1]);
			chdir(currPath);
			setenv("PWD", currPath, 1);
		}
		int status;
		waitpid(pid, &status, 0);
	}
}

		//Function to print the current working directory
void mypwd(char** cmd){
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		char * currPath = getenv("PWD");
		printf("%s\n", currPath);
		exit(0);
	}else{
		int status;
		waitpid(pid, &status, 0);
	}
}

		//Function to print files and directories
void mytree(char** cmd, int n){
	char* currPath = getenv("PWD");
	char* homePath = getenv("HOME");
	char* slash = "/";
	//char* path;
	char npath[80];
	char* hidden1 = ".";
	char* hidden2 = "..";
	pid_t pid = fork();
	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		/*if(n == 2){
			strcat(currPath, slash);
			strncpy(npath, currPath, sizeof currPath);
			strcat(npath, cmd[1]);
		}*/
		
		//Call to print tree function
		printTree(currPath, 0);
		exit(0);
	}else{
		int status;
		waitpid(pid, &status, 0);
	}
}

		//Function to time 
void mytime(char** cmd,int n){
	int tool = 0;
	bool builtin = false;
	struct tms t;
	clock_t realT;
	int ticks = sysconf(_SC_CLK_TCK);
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		if(strcmp(cmd[1], "myexit") == 0){
			tool = 1;
			builtin = true;
		}else if(strcmp(cmd[1], "mycd") == 0){
			tool = 2;
			builtin = true;
		}else if(strcmp(cmd[1], "mypwd") == 0){
			tool = 3;
			builtin = true;
		}else if(strcmp(cmd[1], "mytree") == 0){
			tool = 4;
			builtin = true;
		}else if(strcmp(cmd[1], "mytime") == 0){
			tool = 5;
			builtin = true;
		}else if(strcmp(cmd[1], "mymtimes") == 0){
			tool = 6;
			builtin = true;
		}else if(strcmp(cmd[1], "mytimeout") == 0){
			tool = 7;
			builtin = true;
		}else{
			tool = 8;
			builtin = false;
		}

		switch(tool){
			case 1:
				myexit(cmd + 1);
				break;
			case 2:
				mycd(cmd + 1, n);
				break;
			case 3:
				mypwd(cmd + 1);
				break;
			case 4:
				mytree(cmd + 1, n);
				break;
			case 5:
				mytime(cmd + 1, n);
				break;
			case 6: 
				mymtimes(cmd + 1, n);
				break;
			case 7:
				mytimeout(cmd + 1, n);
				break;
		}
		if(tool == 8){
			myexecute(cmd + 1);
		}
		exit(0);
	}else{
		int status;
		if(wait(&status) == -1)
			printf("error\n");
		else{
			printf("        user time        sys time        time\n");
			printf("\t%f\t%f\t%f\n", ((double) t.tms_cutime)/ticks, ((double) t.tms_cstime)/ticks, ((double) realT)/ticks);
		}
	}
}

void mymtimes(char** cmd, int n){
	int i;
	char* currPath = getenv("PWD");
	char* homePath = getenv("HOME");
	char date[80];
	char* slash = "/";
	//char* path;
	struct stat files;
	
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		
		/*if(n > 1){
			strcpy(path, currPath);
			strcat(path, slash);
			strcat(path, cmd[1]);
			printf("test\n");	
		}else{
			strcpy(path, currPath);	
			printf("test\n");
		}*/

		struct dirent* dirPointer;
		DIR* currDir = opendir(currPath);
		time_t now;
		struct tm *s;
		now = time(NULL);
		s = localtime(&now);
		int temp = s->tm_hour;
		
		//Loop to print output
		for(i = 24; i >= 0; i--){
			s->tm_hour = s->tm_hour - i;
			mktime(s);
			strftime(date, sizeof date, "%a %b %d %H:%M:%S %Y : 0", s);
			printf("%s\n", date);
			s->tm_hour = temp;
		}
		
		exit(0);
	}else{
		int status;
		waitpid(pid, &status, 0);
	}
}

		//Program for mytimeout 
void mytimeout(char** cmd, int n){
	int tool = 0;
	bool builtin = false;
	pid_t pid = fork();
	int limit;
	sscanf(cmd[1], "%d", &limit);
	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		//Running each function within timeout
		if(strcmp(cmd[2], "myexit") == 0){
			tool = 1;
			builtin = true;
		}else if(strcmp(cmd[2], "mycd") == 0){
			tool = 2;
			builtin = true;
		}else if(strcmp(cmd[2], "mypwd") == 0){
			tool = 3;
			builtin = true;
		}else if(strcmp(cmd[2], "mytree") == 0){
			tool = 4;
			builtin = true;
		}else if(strcmp(cmd[2], "mytime") == 0){
			tool = 5;
			builtin = true;
		}else if(strcmp(cmd[2], "mymtimes") == 0){
			tool = 6;
			builtin = true;
		}else if(strcmp(cmd[2], "mytimeout") == 0){
			tool = 7;
			builtin = true;
		}else{
			tool = 8;
			builtin = false;
		}

		switch(tool){
			case 1:
				myexit(cmd + 2);
				break;
			case 2:
				mycd(cmd + 2, n);
				break;
			case 3:
				mypwd(cmd + 2);
				break;
			case 4:
				mytree(cmd + 2, n);
				break;
			case 5:
				mytime(cmd + 2, n);
				break;
			case 6: 
				mymtimes(cmd + 2, n);
				break;
			case 7:
				mytimeout(cmd + 2, n);
				break;
		}
		if(tool == 8){
			myexecute(cmd + 2);
		}
		exit(0);
	}else{
		int status;
	
		//Sleeping for the time limit
		sleep(limit);
		kill(pid, SIGKILL);
		//waitpid(pid, &status, 0);
	}
}

		//Function for normal unix execution
void myexecute(char** cmd){
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		execvp(cmd[0], cmd);
		char* err = strerror(errno);
		printf("Unknown command\n");
		exit(0);
	}else{
		int status;
		waitpid(pid, &status, 0);
	}
}

		//Execution for piped commands
int myexecutepipe(char** cmd1, char** cmd2){
	/*int i = 0;
	for(i = 0; i < 2; i++)
			printf("%s ", cmd1[i]);
	printf("\n");
	for(i = 0; i < 1; i++)
			printf("%s ", cmd2[i]);*/

		//File descriptors 
	int fd[2];
	pipe(fd);
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		close(fd[1]);
		dup2(fd[0], 0);
		execvp(cmd2[0], cmd2);

		char* err = strerror(errno);
		printf("Unknown command\n");
		return 0;
	}else{
		close(fd[0]);
		dup2(fd[1], 1);
		execvp(cmd1[0], cmd1);

		char* err = strerror(errno);
		printf("Unknown command\n");
		return 0;
	}
	return 0;
}

		//Execution for 3 piped commands
int myexecutepipe2(char** cmd1, char** cmd2, char** cmd3){
	int fd[2];
	pipe(fd);
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		pid_t pid2 = fork();
		if(pid2 == -1){

		}else if(pid2 == 0){
			close(fd[0]);
			dup2(fd[1], 1);
			execvp(cmd3[0], cmd3);

			char* err = strerror(errno);
			printf("Unknown command\n");
			return 0;
		}else{

		}	
		close(fd[1]);
		dup2(fd[0], 0);
		execvp(cmd2[0], cmd2);

		char* err = strerror(errno);
		printf("Unknown command\n");
		return 0;
	}else{
		close(fd[0]);
		dup2(fd[1], 1);
		execvp(cmd1[0], cmd1);

		char* err = strerror(errno);
		printf("Unknown command\n");
		return 0;
	}
}

		//Execution with file redirection
void myexecuteredir(char** cmd, int n){
	int i, j;
	int fd;
	char* slash = "/";
	char* fPath = getenv("PWD");
	strcat(fPath, slash);
	strcat(fPath, cmd[n+1]);

		//Opening files 
	mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
	fd = open(fPath, O_RDWR | O_CREAT | O_TRUNC, mode);
	char* out = ">";
	char* in = "<";
	char* cmd2[20] = {0};
	bool output = false;
	bool input = false;

		//Checking for output or input
	if(strcmp(cmd[n], out) == 0)
		output = true;
	if(strcmp(cmd[n], in) == 0)
		input = true;
	j = 0;
	for(i = 0; i < n+1; i++)
		if(i != n){
			cmd2[j] = cmd[i];
			j++;
		}
			 
	pid_t pid = fork();

	if(pid == -1){
		printf("Fork error");
		exit(1);
	}else if(pid == 0){
		if(output){
			close(STDOUT_FILENO);
			dup(fd);
			close(fd);
		}else if(input){
			close(STDIN_FILENO);
			dup(fd);
			close(fd);
		}

		execvp(cmd2[0], cmd2);
		char* err = strerror(errno);
		printf("Unknown command\n");
		exit(0);
	}else{
		int status;
		waitpid(pid, &status, 0);
	}
}

		//Recursive function for printing file tree
void printTree(char* path, int tabs){
	
	char* hidden1 = ".";
	char* hidden2 = "..";	
	int i;
	struct dirent* dirPointer;
	DIR* currDir = opendir(path);
	
	if(currDir == NULL){
			printf("Not a directory\n");
			exit(1);
	}
	while((dirPointer = readdir(currDir)) != NULL){
		if(strcmp(dirPointer->d_name, hidden1) != 0 && strcmp(dirPointer->d_name, hidden2) != 0){
			for(i = 0; i < tabs; i++)
				printf("\t");
			printf("%s\n", dirPointer->d_name);
			if(opendir(dirPointer->d_name) != NULL)
				printTree(dirPointer->d_name, tabs+1);
		}
	}
	closedir(currDir);
	return;
}

		//Function to split user input
int fillParams(char* command, char** params){
	int count = -1;
	int i = 0;
	for(i = 0; i < 20; i++){
		params[i] = strsep(&command, " ");

		count++;
		if(params[i] == NULL){
			i = 20;
		}else{
			//count++;
		}
	}
	return count;
}


