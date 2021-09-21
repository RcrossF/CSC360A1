#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[]){
	if (argc != 3) {
		char temp[100];
		sprintf(temp, "argc: %d \n", argc);
		printf(temp);
		printf("argv: ");
		sprintf(temp, "%s %s %s %s", argv[0], argv[1], argv[2], argv[3]);
		printf(temp);
		fprintf(stderr, "Usage: inf tag interval\n");
	} else {
		const char* tag = argv[1];
		int interval = atoi(argv[2]);
		for(int i=0;i<60;i++) {
			printf("%s\n", tag);
			sleep(interval);
		}
	}
}

