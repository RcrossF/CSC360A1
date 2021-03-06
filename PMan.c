#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <sys/times.h>
#include <sys/wait.h>
#include <readline/readline.h>

#include "helpers.h"

#define MAX_PROCS 50

#define CMD_BG 0
#define CMD_BGLIST 1
#define CMD_BGKILL 2
#define CMD_BGSTART 3
#define CMD_BGSTOP 4
#define CMD_PSTAT 5

struct process running_procs[MAX_PROCS];

int main(){
	char* cmd;
	char* token;
	int cmd_type;
	char temp[200];
	int pid = 0;

	for(int i = 0; i < MAX_PROCS-1;i++){
		running_procs[i].pid = 0;
		running_procs[i].killed = 0;
	}
	

	while(1){	
		cmd = readline("PMan: > ");
		strcpy(temp, cmd);

		// Count number of spaces and allocate argv accordingly
		int num_args = 0;
		for(int i=0;temp[i]!='\0';i++){
			if(temp[i] == ' ')	num_args++;
		}
		char* argv[num_args+1];

		// Parse command
		argv[0] = strtok(temp, " ");
   
		// Check if we got absolutely no input
		if(cmd[0] == '\0')	cmd_type = -1;
		else				cmd_type = parse_cmd_type(argv[0]);

		if(cmd_type == -1){
			printf("Command not found\n");
			continue;
		}

		// Walk through all passed params
		int i = 0;
		while(argv[i] != NULL ) {
			argv[++i] = strtok(NULL, " ");
		}
		
		// Make sure PID was input for commands that need it
		switch (cmd_type){
			case CMD_BGKILL:
			case CMD_BGSTOP:
			case CMD_BGSTART:
			case CMD_PSTAT:
				if(argv[1] != 0x0){
					pid = atoi(argv[1]);
				}
				else{
					printf("\nPlease specify a PID\n");
					continue;
				}
			default:
				break;
		}

		// Run the requested command
		switch (cmd_type){
			case CMD_BG:
				bg_entry(argv);
				break;
			
			case CMD_BGLIST:
				bglist_entry();
				break;

			case CMD_BGKILL:
				bgsig_entry(pid, CMD_BGKILL);
				break;

			case CMD_BGSTOP:
				bgsig_entry(pid, CMD_BGSTOP);
				break;

			case CMD_BGSTART:
				bgsig_entry(pid, CMD_BGSTART);
				break;

			case CMD_PSTAT:
				pstat_entry(pid);
				break;

			default:
				break;
		}

		// Only check for zombies if we have procs running
		for(int i = 0;i<MAX_PROCS-1;i++){
			if(running_procs[i].pid > 0 && !running_procs[i].killed){
				check_zombieProcess();
			}
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

void append_new_proc(int pid, char* path){
	struct process proc;
	for(int i=0;i<MAX_PROCS-1;i++){
		if(running_procs[i].pid == 0 || running_procs[i].killed){
			running_procs[i].pid = pid;
			running_procs[i].killed = 0;
			strcpy(running_procs[i].exec_path, path);
			break;
		}
	}
	
}

struct process query_proc(int pid){
	struct process proc;
	char temp[350];
	char comm[350];
	char state;
	long int rss;
	int vol_ctx;
	int nonvol_ctx;
	unsigned long int utime;
	unsigned long int stime;
	char cmd[80];
	

	// voluntary_ctxt_switches
	sprintf(cmd, "cat /proc/%d/status | grep voluntary_ctxt | grep -Eo '[0-9]{1,10}'", pid);
	singleLineCmd(cmd, temp);
	proc.vol_ctxt_switches = atoi(temp);

	// nonvoluntary_ctxt_switches
	sprintf(cmd, "cat /proc/%d/status | grep nonvoluntary_ctxt | grep -Eo '[0-9]{1,10}'", pid);
	singleLineCmd(cmd, temp);
	proc.nonvol_ctxt_switches = atoi(temp);

	sprintf(cmd, "cat /proc/%d/stat", pid);
	singleLineCmd(cmd, temp);

	// Parse output of /proc/{pid}/stat
	int d = 0; // For unused sscanf elems
	int n = sscanf(temp, "%d %s %c %d %d %d %d %d %u %lu %lu %lu %lu %lu %lu %ld %ld %ld %ld %ld %ld %llu %lu %ld", 
					&d, comm, &state, &d, &d, &d, &d, &d, &d, &d, &d, &d, &d, &utime, &stime, &d, &d, &d, &d, &d, &d, &d, &d, &rss);
	if(n < 24){
		printf("Error parsing command: ");
		printf("%s", cmd);
		printf("\n%d/24 values parsed sucessfully\n", n);
	}

	// Complete the struct
	int sysclk = sysconf(_SC_CLK_TCK);
	proc.pid = pid;
	strcpy(proc.comm, comm);
	proc.rss = rss;
	proc.state = state;
	proc.utime = utime / (float)sysclk;
	proc.stime = stime / (float)sysclk;
	
	return proc;
}


void bg_entry(char *argv[]){
	pid_t pid;

	// Calculate length of argv
	int size = 0;
	while(argv[size] != 0){
		size++;
	}
	// No file passed, return
	if(size == 1){
		printf("No program specified\n");
		return;
	}
	pid = fork();
	if(pid == 0){
		char *pass_args[size]; // 1 less than argv to remove "bg"
		// Slice off first argv element
		for(int i=1;i<size;i++){
			pass_args[i-1] = argv[i];
		}
		// Null terminate
		pass_args[size-1] = 0;
		// Run the program and pass args
		if(execvp(argv[1], pass_args) < 0){
			perror("Error on execvp");
		}
		exit(EXIT_SUCCESS);
	}
	else if(pid > 0) {
		// Get executable path then pass to append-new-proc
		append_new_proc(pid, argv[1]);
	}
	else {
		perror("fork failed");
		exit(EXIT_FAILURE);
	}
}

void bglist_entry(){
	int i = 0;
	int alive = 0;
	while(running_procs[i].pid != 0){
		if(!running_procs[i].killed){
			alive++;
			printf("%d:	%s\n",running_procs[i].pid, running_procs[i].exec_path);
		}
		else{
			i++;
			continue;
		}
		i++;
	}
	printf("Total background jobs:	%d\n", alive);
}


void bgsig_entry(int pid, int cmd_type){
	int sig_cmd;
	char* operation;

	switch (cmd_type){
		case CMD_BGKILL:
			sig_cmd = SIGKILL;
			operation = "killed";
			break;
		case CMD_BGSTOP:
			sig_cmd = SIGSTOP;
			operation = "stopped";
			break;
		case CMD_BGSTART:
			sig_cmd = SIGCONT;
			operation = "started";
			break;
		default:
			printf("Invalid bgsig command received");
			return;
	}

	int result = kill(pid, sig_cmd);
	if(result != 0){
		printf("Command failed, double check your PID");
	}
	if(sig_cmd == SIGKILL || sig_cmd == SIGSTOP){
		remove_proc_from_arr(pid);
	}
	printf("Process with pid %d %s\n", pid, operation);

	return;
}


void pstat_entry(int pid){
	if(pid_exists(pid) != 0){
		printf("\nError:	Process %d does not exist\n", pid);
		return;
	}
	struct process proc_info = query_proc(pid);

	printf("\nDetails for PID %d:\n"
			"	comm: %s\n"
			"	state: %c\n"
			"	utime: %lf\n"
			"	stime: %lf\n"
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
}

void check_zombieProcess(void){
	int status;
	int retVal = 0;
	int user_initiated_kill = 0;
	// Processes that we kill leave remnants in running_procs.
	// This tracks if they actually died in the background or if we're finding these remnants
	int actually_died = 0; 
	
	while(1) {
		usleep(1000);
		retVal = waitpid(-1, &status, WNOHANG);
		if(retVal > 0) {
			// If the killed PID has already been removed from running_procs then don't print our lovely message
			// because the kill was initiated by the user
			for (int i=0; i < MAX_PROCS-1; i++){	
				if (running_procs[i].pid == retVal){
					if(running_procs[i].killed){
						user_initiated_kill = 1;
						break;
					}
					else{
						actually_died = 1;
					}
				}
			}
			if(!user_initiated_kill && actually_died){
				remove_proc_from_arr(retVal);
				printf("\nProcess with pid %d terminated in the background\n", retVal);
				break;
			}
		}
		else if(retVal == 0){
			break;
		}
		else{
			// If we have no child processes ignore waitpid errors
			for(int i=0;i<MAX_PROCS-1;i++){
				if(running_procs[i].pid && !running_procs[i].killed){
					perror("waitpid failed");
					exit(EXIT_FAILURE);
				}
			}
		}
	}
	return;
}

void remove_proc_from_arr(int pid){
	int i = 0;
    while(running_procs[i].pid != 0){
        if(running_procs[i].pid == pid){
			running_procs[i].killed = 1;
			break;
        }
		i++;
    }
}