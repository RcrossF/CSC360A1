#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "helpers.h"
#include <sys/times.h>

#define NUM_ARGS 20
#define MAX_PROCS 50

#define CMD_BG 0
#define CMD_BGLIST 1
#define CMD_BGKILL 2
#define CMD_BGSTART 3
#define CMD_BGSTOP 4
#define CMD_PSTAT 5

struct process running_procs[MAX_PROCS];

int main(){
	char* argv[NUM_ARGS]; // Support 20 arguments of 40 characters each
	char* cmd;
	char* token;
	int cmd_type;
	int pid = 0;

	for(int i = 0; i < MAX_PROCS-1;i++){
		running_procs[i].pid = 0;
	}
	

	while(1){	
		cmd = readline("PMan: > ");

		// Parse command
		argv[0] = strtok(cmd, " ");
   
		// Check if we got absolutely no input
		if(cmd[0] == '\0')	cmd_type = -1;
		else				cmd_type = parse_cmd_type(argv[0]);

		if(cmd_type == -1){
			printf("\nCommand not found\n");
			continue;
		}

		// Walk through all passed params
		int i = 0;
		while(argv[i] != NULL ) {
			argv[++i] = strtok(NULL, " ");
		}
		
		switch (cmd_type){
			case CMD_BG:
				bg_entry(argv);
				break;
			
			case CMD_BGLIST:
				bglist_entry();
				break;

			case CMD_BGKILL:
			case CMD_BGSTOP:
				bgsig_entry(pid, cmd_type);
				break;

			case CMD_PSTAT:
				pid = atoi(argv[1]);
				pstat_entry(pid);
				break;

			default:
				break;
		}

		//check_zombieProcess();

		// Empty argv for the next loop
		int j = 0;
		while(j < NUM_ARGS - 1){
			argv[j] = "";
			j++;
		}
	}

	return 0;
}

int parse_cmd_type(char* cmd_type){
	if(cmd_type[0] == '\0') return -1;

	strToUpper(cmd_type);
	if(strcmp(cmd_type, "BG") == 0)				return CMD_BG;
	else if(strcmp(cmd_type, "BGLIST") == 0)	return CMD_BGLIST;
	else if(strcmp(cmd_type, "BGKILL") == 0)	return CMD_BGKILL;
	else if(strcmp(cmd_type, "BGSTART") == 0)	return CMD_BGSTART;
	else if(strcmp(cmd_type, "BGSTOP") == 0)	return CMD_BGSTOP;
	else if(strcmp(cmd_type, "PSTAT") == 0)	return CMD_PSTAT;
	else return -1;
}

void append_new_proc(int pid){
	struct process proc;
	for(int i=0;i<MAX_PROCS-1;i++){
		if(running_procs[i].pid == 0){
			running_procs[i].pid = pid;
		}
	}
	
}

struct process query_proc(int pid){
	char temp[350];
	char comm[350];
	char state;
	long int rss;
	int vol_ctx;
	int nonvol_ctx;
	long unsigned int utime;
	long unsigned int stime;
	char cmd[50];
	

	// Comm
	sprintf(cmd, "ps -p %d -o comm=", pid);
	singleLineCmd(cmd, comm);
	// Remove newline from end of string
	for(int i = 0; i<349;i++){
		if(comm[i] == '\n'){
			comm[i] = '\0';
			break;
		}
	}
	// voluntary_ctxt_switches
	sprintf(cmd, "cat /proc/%d/status | grep voluntary_ctxt | grep -Eo '[0-9]{1,10}'", pid);
	singleLineCmd(cmd, temp);
	vol_ctx = atoi(temp);

	// nonvoluntary_ctxt_switches
	sprintf(cmd, "cat /proc/%d/status | grep nonvoluntary_ctxt | grep -Eo '[0-9]{1,10}'", pid);
	singleLineCmd(cmd, temp);
	nonvol_ctx = atoi(temp);

	sprintf(cmd, "cat /proc/%d/stat", pid);
	singleLineCmd(cmd, temp);

	// Parse output of /proc/{pid}/stat
	int d = 0; // For unused sscanf elems
	char s[100];
	int n = sscanf(temp, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld", &d, s, &state, &d, &d, &d, &d, &d, &d, &d, &d, &d, &d, &utime, &stime, &d, &d, &d, &d, &d, &d, &d, &d, &rss);
	if(n < 24){
		printf("Error parsing command: ");
		printf(cmd);
		sprintf(temp, "\n%d/24 values parsed sucessfully", n);
		printf(temp);
	}

	// Build a struct with all this parsed info
	struct process proc;
	proc.pid = pid;
	strcpy(proc.comm, comm);
	proc.rss = rss;
	proc.state = state;
	proc.utime = utime;
	proc.stime = stime;
	proc.vol_ctxt_switches = vol_ctx;
	proc.nonvol_ctxt_switches = nonvol_ctx;

	return proc;
}


void bg_entry(char *argv){
	pid_t pid;
	pid = fork();
	if(pid == 0){
		if(execvp(argv[0], argv) < 0){
			perror("Error on execvp");
		}
		exit(EXIT_SUCCESS);
	}
	else if(pid > 0) {
		// TODO: get executable path then pass to append-new-proc
		append_new_proc(pid);
	}
	else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
}

void bglist_entry(void){}
void bgsig_entry(int pid, int cmd_type){}
void pstat_entry(int pid){
	char tempstr[150];

	if(pid_exists(pid) != 0){
		sprintf(tempstr, "\nError:	Process %d does not exist\n", pid);
		printf(tempstr);
		return;
	}
	struct process proc_info = query_proc(pid);

	sprintf(tempstr, "\nDetails for PID %d:\n"
					"	comm: %s\n"
					"	state: %c\n"
					"	utime: %lu\n"
					"	stime: %lu\n"
					"	rss: %ld\n"
					"	voluntary_ctxt_switches: %ld\n"
					"	nonvoluntary_ctxt_switches: %ld\n",
					proc_info.pid,
					proc_info.comm,
					proc_info.state,
					proc_info.utime,
					proc_info.stime,
					proc_info.rss,
					proc_info.vol_ctxt_switches,
					proc_info.nonvol_ctxt_switches);
	printf(tempstr);
}

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