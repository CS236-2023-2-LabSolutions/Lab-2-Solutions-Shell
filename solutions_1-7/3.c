#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
	char* args[] = {"ls", "-l", NULL};
	execvp(args[0], args);
	printf("not expected\n");
}
