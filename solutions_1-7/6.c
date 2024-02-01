#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

int main(){
	int r = fork();
	if(r == 0){
		sleep(10);
	}
	else{
		printf("Forked child %d\n", r);
		sleep(2);
		kill(r, SIGTERM);
		int pid = wait(NULL);
		printf("Killed child %d\n", pid);
	}
}
