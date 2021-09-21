#ifndef INCLUDE_H
#define INCLUDE_H

struct process {
    int pid;
    char comm[350];
    char exec_path[200];
    char state;
    long unsigned int utime;
    long unsigned int stime;
    long int rss;
    long int vol_ctxt_switches;
    long int nonvol_ctxt_switches;
};

/* Prototypes for the functions */
/* Capitalize a string */
void strToUpper(char* str);

/* Run a single line command and return the result */
void singleLineCmd(char* cmd, char* res);

/* Check if process given by pid exists */
int pid_exists(int pid);
#endif