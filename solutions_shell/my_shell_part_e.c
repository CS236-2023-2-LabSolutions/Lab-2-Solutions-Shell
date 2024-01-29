#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

/* Splits the string by space and returns the array of tokens
*
*/
char **tokenize(char *line)
{
  char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
  char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
  
  memset(tokens,0,sizeof(tokens)); // Initializing to NULL to check if tokens are empty

  int i, tokenIndex = 0, tokenNo = 0;

  for(i =0; i < strlen(line); i++){

    char readChar = line[i];

    if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
      token[tokenIndex] = '\0';
      if (tokenIndex != 0){
	tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
	strcpy(tokens[tokenNo++], token);
	tokenIndex = 0; 
      }
    } else {
      token[tokenIndex++] = readChar;
    }
  }
 
  free(token);
  tokens[tokenNo] = NULL ;
  return tokens;
}

void free_mem(char **tokens){

	for(int i=0;tokens[i]!=NULL;i++){
		free(tokens[i]);
	}
	free(tokens);
}

int fore_pgid=-1; //Foreground process pgid
int back_num=0; // Number of background processes
int back_pgid=-1; //Background process pgid

void signal_handler(int sig){

    if(fore_pgid==-1){
        return;
    }
    kill(-fore_pgid,SIGTERM); // Killing all processes in foreground process group id
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;

    signal(SIGINT,signal_handler); // Added signal handler


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line

		tokens = tokenize(line);

        if(tokens[0] && !strcmp(tokens[0],"exit") && !tokens[1]){

            // If first token is exit, and 2nd is NULL then free memory
            free_mem(tokens);

			if(fore_pgid!=-1){
	            kill(-fore_pgid,SIGINT);
			}
			if(back_pgid!=-1){
	            kill(-back_pgid,SIGINT);
			}
            kill(-getpid(),SIGTERM);
            exit(0);
        }

		// Reap any terminated background processes without waiting for them

		int check;

		while(check = waitpid(-1,NULL,WNOHANG)>0){
			
			// If any process terminated, print message
			printf("Shell : Background Process Finished\n");
            // Decrement number of background processes
            back_num--;
		}

		// When no background process, set to -1.

		if(back_num==0){

			back_pgid=-1;
		}
		
		// If empty, restart shell
		if(!tokens[0]){
			continue;
		}

		// Checking if background argument (&) is present
		int background=-1;

		for(int i=0;i<MAX_NUM_TOKENS;i++){

			if(tokens[i]==NULL){

				break;
			}else if(!strcmp(tokens[i],"&")){

				if(tokens[i+1]==NULL){

					background=i;
				}else{

					background=-2;
				}
				break;
			}
		}

		// If background term exists, and "&&" or "&&&" also exist, then throw error

        if(background>0){

            for(int i=0;i<MAX_NUM_TOKENS;i++){
                
                if(tokens[i]==NULL){

                    break;
                }
                if(!strcmp(tokens[i],"&&") || !strcmp(tokens[i],"&&&") ){

                    background=-2;
                    break;
                }
            }
        }

		if(background==-2){
			// If background is not last argument, error
            printf("Shell : Incorrect Usage of &\n");
			
            continue;
		}
   
		if(strcmp(tokens[0],"cd")){
			
			// Treat cd commands differently
			// Use strcmp to compare strings since normal comparision wouldn't work as "cd" and tokens[0] 
			// have different sizes
			

			// If background process, increment number of background processes by 1
			if(background>0){
				tokens[background]=NULL;
				back_num++;
			}

			int pid=fork();

			if(pid==0){

				// execvp is used as commands have to be looked for in Path

				if(background>0){

					// If first background process, assign pgid as pid
					// If not, assign as pid already present
					if(back_num==1){
						setpgid(0,0);
					}else{
	                    setpgid(0,back_pgid);
					}
				}else{

                    setpgid(0,0);
                }

                int co=1; // Count of number of subprocesses created

                int prev=0; // Index of the first token after the last seen && or &&&

                for(int i=0;i<MAX_NUM_TOKENS;i++){

                    if(tokens[i]==NULL){

                        break;
                    }

                    if(!strcmp(tokens[i],"&&")){

                        tokens[i]=NULL; //Null terminate the array, effectively splitting it
                        if(fork()==0){

                            execvp(*(tokens+prev),tokens+prev); // Exec statement
                            printf("Shell: Incorrect Command : %s\n",*(tokens+prev));
                            exit(0);
                        }else{

                            wait(NULL); // Since sequential, we are waiting
                            prev=i+1; // Update the value of prev
                        }
                        continue;
                    }

                    if(!strcmp(tokens[i],"&&&")){

						// Similar to &&, except we increment the number of processes since we will
						// wait only after all processes are created.
                        tokens[i]=NULL;
                        co++;
                        if(fork()==0){

                            execvp(*(tokens+prev),tokens+prev);
                            printf("Shell: Incorrect Command : %s\n",*(tokens+prev));
                            exit(0);
                        }else{

                            prev=i+1;
                        }
                        continue;
                    }
                }
				// Executing the final argument (since it does not end with a && or &&&)
                if(fork()==0){
                    execvp(*(tokens+prev),tokens+prev);
                    // Fork copies the heap memory as well, so this is memory safe
                    printf("Shell: Incorrect Command : %s\n",tokens[0]);
                    exit(0);
                }else{

                    for(int i=0;i<co;i++){

                        wait(NULL);
                    }
					// Must have as many wait statements as the processes we created
					// For && we were reaping instantly, for &&& we incremented the co counter
                }
                // After all subprocesses have ended, the original forked process can end as well
                exit(0);
			}else{

				// Shell waits for completion
				if(background==-1){
					// Wait for only the process that was just forked
					// Update value of foreground pgid
                    fore_pgid=pid;
                    waitpid(pid,NULL,0);
				}else if(back_num==1){

					// If first background process, update value of background pgid
					back_pgid=pid;
				}
				// No foreground process anymore, so change foreground pid to -1
                fore_pgid=-1;
			}
		}else{

			if(background>0){

				printf("Shell: cd cannot be executed in background :\n");
				free_mem(tokens);
				continue;
			}

			if(!tokens[1] || tokens[2]){

				// If first argument after cd is empty or 2nd argument contains data, then error
				printf("Shell: Incorrect Command : %s\n",tokens[0]);
				free_mem(tokens);

				continue;
			}else{

				int ret = chdir(tokens[1]);
				// If return value is not 0, error occurred
				if(ret){

					printf("Shell: Incorrect Command : %s\n",tokens[0]);
		            free_mem(tokens);
					continue;
				}
			}
		}
       
		// Freeing the allocated memory	
		free_mem(tokens);
	}
	return 0;
}
