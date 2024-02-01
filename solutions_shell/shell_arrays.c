#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>


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


int curr_pgid[64];
int mid_int = 0;

void sigint_handler(int sig){
	// printf("\n$ ");
	int flag = 0;
	for(int i = 0; i < 64; i++){
		if(curr_pgid[i] > 0){
			// printf("curr_pgid: %d\n", curr_pgid);
			kill(-curr_pgid[i], SIGINT);
			flag = 1;
		}
	}
	if(flag == 1){
		mid_int = 1;
		printf("\n");
	}
	else{
		printf("\n$ ");
		fflush(NULL);
		mid_int = 0;
	}
}

void tokfree(char** tokens){
	for(int i = 0; tokens[i] != NULL; i++){
		free(tokens[i]);
	}
	free(tokens);
}

int main(int argc, char* argv[]) {

	for(int i = 0; i < 64; i++){
		curr_pgid[i] = -10;
	}

	signal(SIGINT, sigint_handler);

	char  line[MAX_INPUT_SIZE];            
	char  **tokens;    

	int bkgd[64];
	int valid[64];
	for(int i = 0; i < 64; i++){
		valid[i] = 0;
		bkgd[i] = -2;
	}

	while(1) {			
		/* BEGIN: TAKING INPUT */
		assert(mid_int == 0);
		bzero(line, sizeof(line));
		printf("$ "); fflush(NULL);
		scanf("%[^\n]", line);
		getchar();

		/* END: TAKING INPUT */

		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		int toklen = 0;
		int frgd_seq = 0;
		int frgd_par = 0;

		for(int i = 0; tokens[i] != NULL; i++){
			toklen++;
			if(strcmp(tokens[i], "&&") == 0){
				frgd_seq = 1;
			}
			if(strcmp(tokens[i], "&&&") == 0){
				frgd_par = 1;
			}
		}

		if(frgd_par && frgd_seq){
			printf("Shell: Incorrect command\n");
			tokfree(tokens);
			continue;
		}

		if(toklen == 0){
			tokfree(tokens);
			continue;
		}

		if(strcmp(tokens[0], "cd") == 0){
			int ret = chdir(tokens[1]);
			if(ret < 0){
				printf("Shell: Incorrect command\n");
			}
		}

		else if(strcmp(tokens[0], "exit") == 0){
			for(int i = 0; i < 64; i++){
				if(valid[i] == 1){
					kill(bkgd[i], SIGTERM);
					valid[i] = 0;
				}
			}
			tokfree(tokens);
			return 0;
		}

		else if(strcmp(tokens[toklen - 1], "&") == 0){
			tokens[toklen - 1] = NULL;
			int r = fork();
			if(r == 0){
				setpgid(getpid(), 0);
				int ret = execvp(tokens[0], tokens);
				if(ret < 0){
					printf("Shell: Incorrect command\n");
					tokfree(tokens);
				}
				return 0;
			}
			else{
				for(int i = 0; i < 64; i++){
					if(valid[i] == 0){
						valid[i] = 1;
						bkgd[i] = r;
						break;
					}
				}
			}
		}

		else if(frgd_seq){
			char* toks[64];
			int tokind = 0;
			for(int i = 0; (i <= toklen) && (mid_int == 0); i++){
				if((i < toklen) && (strcmp(tokens[i], "&&") != 0)){
					toks[tokind] = tokens[i];
					tokind++;
				}
				else{
					toks[tokind] = NULL;
					tokind = 0;
					int r = fork();
					if(r == 0){
						setpgid(getpid(), 0);
						int ret = execvp(toks[0], toks);
						if(ret < 0){
							printf("Shell: Incorrect command %s\n", toks[0]);
							tokfree(tokens);
						}
						return 0;
					}
					else{
						curr_pgid[0] = r;
						waitpid(r, NULL, 0);
						curr_pgid[0] = -10;
					}
				}
			}
			mid_int = 0;
		}

		else if(frgd_par){
			char* toks[64];
			int tokind = 0;
			int fgd_procs = 0;
			for(int i = 0; (i <= toklen) && (mid_int == 0); i++){
				if((i < toklen) && (strcmp(tokens[i], "&&&") != 0)){
					toks[tokind] = tokens[i];
					tokind++;
				}
				else{
					toks[tokind] = NULL;
					tokind = 0;
					int r = fork();
					if(r == 0){
						setpgid(getpid(), 0);
						int ret = execvp(toks[0], toks);
						if(ret < 0){
							printf("Shell: Incorrect command %s\n", toks[0]);
							tokfree(tokens);
						}
						return 0;
					}
					else{
						curr_pgid[fgd_procs] = r;
						fgd_procs++;
					}
				}
			}
			while(1){
				int flag = 0;
				for(int i = 0; i < fgd_procs; i++){
					if(curr_pgid[i] > 0){
						int ret = waitpid(curr_pgid[i], NULL, WNOHANG);
						if(ret == 0){
							flag = 1;
						}
						else{
							curr_pgid[i] = -10;
						}
					}
				}
				if(!flag){
					break;
				}
			}
			mid_int = 0;
		}

		else{
			int r = fork();
			if(r == 0){
				setpgid(getpid(), 0);
				int ret = execvp(tokens[0], tokens);
				if(ret < 0){
					printf("Shell: Incorrect command\n");
					tokfree(tokens);
				}
				return 0;
			}
			else{
				curr_pgid[0] = r;
				waitpid(r, NULL, 0);
				curr_pgid[0] = -10;
			}
			mid_int = 0;
		}

		for(int i = 0; i < 64; i++){
			if(valid[i]){
				int ret = waitpid(bkgd[i], NULL, WNOHANG);
				if(ret == bkgd[i]){
					valid[i] = 0;
					printf("Shell: Background process finished [%d]\n", bkgd[i]);
					bkgd[i] = -2;
				}
			}
		}

		// Freeing the allocated memory	

		tokfree(tokens);

	}
	return 0;
}
