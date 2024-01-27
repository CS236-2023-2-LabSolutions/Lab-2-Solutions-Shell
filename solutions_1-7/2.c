#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

int main(){
    int r = fork();
    if(r == 0){
        printf("%d\n", getpid());
    }
    else{
        int pid = wait(NULL);
        printf("%d\n", pid);
    }
}