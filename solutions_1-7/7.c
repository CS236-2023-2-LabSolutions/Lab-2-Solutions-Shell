#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

void sigint_handler(int sig){
    printf("I will run forever\n");
}

int main(){
    signal(SIGINT, sigint_handler);
    while(1);
    // kill using `kill -9 <pid>` from terminal
}