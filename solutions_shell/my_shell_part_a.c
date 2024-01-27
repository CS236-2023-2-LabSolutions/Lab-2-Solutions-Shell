#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

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


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];            
	char  **tokens;              
	int i;


	while(1) {			
		/* BEGIN: TAKING INPUT */
		bzero(line, sizeof(line));
		printf("$ ");
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line

		tokens = tokenize(line);

		// If empty, restart shell
		if(!tokens[0]){
			continue;
		}
   
		if(strcmp(tokens[0],"cd")){
			
			// Treat cd commands differently
			// Use strcmp to compare strings since normal comparision wouldn't work as "cd" and tokens[0] 
			// have different sizes
			
			if(fork()==0){

				// execvp is used as commands have to be looked for in Path
				execvp(tokens[0],tokens);
				printf("Shell: Incorrect Command : %s\n",tokens[0]);
				exit(0);
			}else{

				// Shell waits for completion
				wait(NULL);
			}
		}else{

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
