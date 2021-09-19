#ifndef INCLUDE_H
#define INCLUDE_H

struct process {
    int pid;
    char* comm;
    char state;
    unsigned int utime;
    unsigned int stime;
    int rss;
    int vol_ctxt_switches;
    int nonvol_ctxt_switches;
};

/* Prototypes for the functions */
/* Capitalize a string */
void strToUpper(char* str);

/* Run a single line command and return the result */
void singleLineCmd(char* cmd, char* res);
#endif