#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#define CMD_BG 0
#define CMD_BGLIST 1
#define CMD_BGKILL 2
#define CMD_BGSTOP 3
#define CMD_BGCONT 4
#define CMD_PSTAT 5

int main(){
	char* cmd;
	char* token;
	int cmd_type;

	int pid = 0;
	char* argv;

	while(1){	
		cmd = readline("PMan: > ");
		
		token = strtok(cmd, ' ');
   
		/* walk through other tokens */
		while( token != NULL ) {
			printf( " %s\n", token );
			
			token = strtok(NULL, ' ');
		}
		
		if (cmd_type == CMD_BG){
			bg_entry(argv);
		}
		else if(cmd_type == CMD_BGLIST){
			bglist_entry();
		}
		else if(cmd_type == CMD_BGKILL || cmd_type == CMD_BGSTOP || cmd_type == CMD_BGCONT){
			bgsig_entry(pid, cmd_type);
		}
		else if(cmd_type == CMD_PSTAT){
			pstat_entry(pid);
		}
		else {
			usage_pman();
		}
		check_zombieProcess();
	}
	return 0;
}

void bg_entry(char **argv){
	
	pid_t pid;
	pid = fork();
	if(pid == 0){
		if(execvp(argv[0], argv) < 0){
			perror("Error on execvp");
		}
		exit(EXIT_SUCCESS);
	}
	else if(pid > 0) {
		// store information of the background child process in your data structures
	}
	else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
}

void bglist_entry(void){}
void bgsig_entry(int pid, int cmd_type){}
void pstat_entry(int pid){}
void usage_pman(void){}

void check_zombieProcess(void){
	int status;
	int retVal = 0;
	
	while(1) {
		usleep(1000);

		char* headPnode = "CHANGEME";
		if(headPnode == NULL){
			return ;
		}
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			//remove the background process from your data structure
		}
		else if(retVal == 0){
			break;
		}
		else{
			perror("waitpid failed");
			exit(EXIT_FAILURE);
		}
	}
	return ;
}