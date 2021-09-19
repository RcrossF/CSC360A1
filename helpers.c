#include <stdio.h>
#include <string.h>

void strToUpper(char *str)
{
    for(int i = 0; str[i] != '\0'; i++){
        str[i] = toupper(str[i]);
    }
}

void singleLineCmd(char *cmd, char* res){
    char buff[350];

    FILE *fp = popen(cmd, "r");
	if (fp == NULL) {
		printf("Error calling process info\n" );
		exit(1);
	}
	fgets(buff, sizeof(buff), fp);
    pclose(fp);

    strcpy(res, buff);
}
