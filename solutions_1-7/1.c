#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
	int r = fork();
	if(r == 0){
		printf("I am child\n");
	}
	else{
		wait(NULL);
		printf("I am parent\n");
	}
}
