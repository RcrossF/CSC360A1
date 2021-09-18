#include <stdio.h>
#include <string.h>

void strToUpper(char *str)
{
    for(int i = 0; str[i] != '\0'; i++){
        str[i] = toupper(str[i]);
    }
}