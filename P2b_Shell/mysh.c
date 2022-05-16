#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <fcntl.h>


#define BUFFER 512
#define TOKEN_BUF 100

struct node {
	char* alias_name;
	char* exec_name;
	struct node *next;
};

struct node* head = NULL;

struct node* search(char* usr_alias_name) {
	struct node* current = head;
	if(head == NULL) {
		return NULL;
	}

	while(strncmp(current->alias_name, usr_alias_name, strlen(usr_alias_name))) {
		if(current->next == NULL) {
			return NULL;
		} else {
			current = current -> next;
		}
	}
	return current;
}

int insert(char* usr_alias_name, char* usr_exec_name) {
// 1 - success, 0 - otherwise
	struct node* current = search(usr_alias_name);
	if(head == NULL) {
		head = (struct node*)malloc(sizeof(struct node));
        	head->alias_name = strdup(usr_alias_name);
        	head->exec_name = strdup(usr_exec_name);
        	head->next = NULL;
		return 1;
	} else if (current != NULL) {
		current->exec_name = strdup(usr_exec_name);
	} else {
		current = head;
		while(strcmp(current->alias_name, usr_alias_name)) {
			if(current->next == NULL) {
				break;
			} else {
				current = current->next;
			}
		}
		struct node* new = (struct node*) malloc(sizeof(struct node));
        
		new->alias_name = strdup(usr_alias_name);
		new->exec_name = strdup(usr_exec_name);
		new->next = NULL;
		current->next = new;
		return 1;
	}
	return 0;
}

int delete(char* usr_alias_name) {
// 1 - success, 0 - otherwise
	struct node* current = head;
	struct node* previous = NULL;
	
	if(head == NULL) {
		return 0;
	}

	while(strncmp(current->alias_name, usr_alias_name, strlen(usr_alias_name) - 1)) {
		if(current->next == NULL) {
			return 0;
		} else {
			previous = current;
			current = current->next;
		}
	}

	if(current == head) {
		head = head-> next;
	} else {
		previous->next = current->next;
	}
	return 1;
}

void alias_to_string() {
    struct node* curr = head;
	while(curr != NULL) {	
		fprintf(stdout,"%s %s\n", curr->alias_name, curr->exec_name);
        fflush(stdout);
		curr = curr->next;
	}
}

void free_list() {
    struct node* current = head;
    struct node* temp;

    while(current != NULL) {
        temp = current;
        current = current ->next;
        free(temp);
    }

}

// remove extra white space
void strip_space(char* str) {
    int j = 0;
    for (int i = 0; str[i]; ++i) {
        if(!isspace(str[i]) || (i > 0 && !isspace(str[i-1]))) {
            if(str[i] == '\t') 
                str[i] = ' ';
            str[j++] = str[i];
        }
    }
    str[j] = '\0';
}

//checks if file is valid
int valid_file(char* path) {
    FILE* file = fopen(path, "w");
    if(file == NULL) {
        return 0;
    }
    fclose(file);
    return 1;

}

// checks if path is valid
int valid_path(char* path) {
    FILE* file = fopen(path, "r");
    if(file == NULL) {
        return 0;
    }
    fclose(file);
    return 1;

}

// run the command given 
int run_cmd(char* argv[], char* file) {
    pid_t child_pid;
    int child_status;

    child_pid = fork();
    int stdout_fd;
    int new_fd;
    if(child_pid == 0) {
        // change file descriptor if necessary 
        if(file != NULL) {
            stdout_fd =  dup(1);
            close(1);
            if(valid_file(file))
                new_fd = open(file, O_WRONLY|O_CREAT);
            
        }
        if(execv(argv[0], argv) == -1) {
            fprintf(stderr, "%s: Command not found.\n", argv[0]);
            fflush(stderr);
            _exit(EXIT_FAILURE);
        }
    } else if (child_pid < 0){
         _exit(EXIT_FAILURE);
         return 0;
    } else {
        do {
            waitpid(child_pid, &child_status, WUNTRACED);
        } while(!WIFEXITED(child_status) && !WIFSIGNALED(child_status));
    }
    if(file != NULL) {
        close(new_fd);
        dup2(stdout_fd, 1);
        close(stdout_fd);
    }
    return 1;
}

int main(int argc, char **argv){
    FILE *fstream;
    
    // 0 - interactive, 1 - batch
    int mode = 0;
    // incorrect number of args
    if(argc > 2) {
        fprintf(stderr, "Usage: mysh [batch-file]\n");
        fflush(stderr);
        return 1;
    // batch mode
    } else if (argc == 2) {
        fstream = fopen(argv[1], "r");
        if (fstream == NULL) {
            fprintf(stderr, "Error: Cannot open file %s.\n", argv[1]);
            fflush(stderr);
            return 1;
        }
        mode = 1;
    }
    
    // Starting shell
    if(mode == 0) {
        fprintf(stdout, "mysh> ");
        fflush(stdout);
    }
    
    char line[BUFFER];
    char dup_line[BUFFER];

    while(1) {
        // user input
        memset(line, '\0', BUFFER);
        // flag for user input
        char* flag; 

        // interactive
        if (mode == 0) {
            flag = fgets(line, BUFFER, stdin);
        // batch
        } else {
            flag = fgets(line, BUFFER, fstream);
        }
        // only prints if file
        if(mode == 1) {
                fprintf(stdout,"%s", line);
                fflush(stdout);
        }

        strip_space(line);

        if((strlen(line)) > BUFFER) {
            fprintf(stderr, "Error: Input is too long\n");
            fflush(stderr);
        } else if((strlen(line) == 4 || strlen(line) == 5) && !strncmp(line, "exit", 4)) {
                break;
        } else if (flag == NULL) {
            break;
        } else if (!strncmp(line, "alias", 5)) {
            char* temp = strdup(line);
            char* alias_ptr = NULL;
            char* token = strtok(temp, " ");
            int i = 0;
            while(token != NULL) {
                if(i != 0) {
                    alias_ptr = token;
                    break;
                }
                token = strtok(NULL, " ");
                i++; 
            }
            if(alias_ptr != NULL) {
                alias_ptr[strlen(alias_ptr)] = '\0';
                if(strlen(alias_ptr) == 6 && !strncmp(alias_ptr, "alias", strlen(alias_ptr) - 1)) {
                    fprintf(stderr, "alias: Too dangerous to alias that.\n");
                    fflush(stderr);
                } else if (strlen(alias_ptr) == 5 && !strncmp(alias_ptr, "exit", strlen(alias_ptr) - 1)) {
                    fprintf(stderr, "alias: Too dangerous to alias that.\n");
                    fflush(stderr);
                } else if (strlen(alias_ptr) == 8 && !strncmp(alias_ptr, "unalias", strlen(alias_ptr) - 1)) {
                    fprintf(stderr, "alias: Too dangerous to alias that.\n");
                    fflush(stderr);
                }
            }

            char* exec_ptr = NULL;
            int space_counter = 0;
            for(int i = 0; i < strlen(line); i++) {
                if (line[i] == ' ') {
                    space_counter++;
                }
                if (space_counter == 2) {
                    exec_ptr = &line[i+1];
                    break;
                }
            }
            if(exec_ptr != NULL) {
                exec_ptr[strlen(exec_ptr)-1] = '\0';
            }

            // 1) alias
            if(alias_ptr == NULL) {
                alias_to_string();
            // 2) alias alias_name
            } else if(exec_ptr == NULL) {
                char* alias_ptr_temp = strndup(alias_ptr, strlen(alias_ptr) - 1);
                struct node* curr = search(alias_ptr_temp);
                if(curr != NULL) {
                    fprintf(stdout, "%s %s\n", curr->alias_name, curr->exec_name);
                    fflush(stdout);
                }
            // 3) alias alias_name exec_name
            } else {
                insert(alias_ptr, exec_ptr);
            }
        } else if (!strncmp(line, "unalias", 7)) {
            char* temp = strdup(line);
            char* alias_ptr = NULL;
            char* token = strtok(temp, " ");
            int i = 0;
            while(token != NULL) {
                if(i == 1) {
                    alias_ptr = token;
                }
                token = strtok(NULL, " ");
                i++; 
            }
            if(i != 2) {
                fprintf(stderr, "unalias: Incorrect number of arguments.\n");
                fflush(stderr);
            } else {
                int success = delete(alias_ptr); 
                printf("%d\n", success);
            }
        } else if (!strncmp(line, "unalias", 7)){
            printf("this catches unalias");
        } else if (strlen(line) != 0) {
            // checks if line needs to be redirected
            memset(dup_line, '\0', BUFFER);
            strncpy(dup_line, line, strlen(line) - 1);

            // check alias list --> exist --> change dup line to exec_name
            struct node* alias_key = search(dup_line);
            if(alias_key != NULL) {
                strcpy(line, alias_key->exec_name);
                line[strlen(alias_key->exec_name)] = 'f';
                line[strlen(alias_key->exec_name) + 1] = '\0';
            }

            char array[2][TOKEN_BUF];
            int token_counter = 0;
            int redirection_flag = 1;
            
            // redirection
            for(int i = 0; i < strlen(line); i++) {
                if(i == 0 && line[0] == '>') {
                    // begins with '>'
                    redirection_flag = -1; 
                    break;
                redirection_flag = -1;
                }
                if(line[i] == '>') {
                    token_counter++;
                }
            }

            if(token_counter == 0 && redirection_flag != -1) { 
                for(int i = 0; i < strlen(line) - 1; i++) {
                    array[0][i] = line[i];
                }
                array[0][strlen(line) - 1] = '\0';
                array[1][0] = '\0';
                redirection_flag = 0;
            } else if(token_counter > 1) {
                    redirection_flag = -1;
            } else {
                int i = 0;
                char* token = strtok(line, ">");
                // get tokens
                while(token != NULL) {
                    if (i > 2) {
                        redirection_flag = -1;
                        break;
                    }
                    if (i == 0) {
                        for(int j = 0; j < strlen(token); j++) {
                            array[i][j] = token[j];
                        }
                        array[i][strlen(token)] = '\0';
                    } else {
                        for(int j = 0; j < strlen(token) - 1; j++) {
                            array[i][j] = token[j];
                        }
                        array[i][strlen(token) - 1] = '\0';
                    }
                    token = strtok(NULL, ">");
                    i++; 
                }

                if(i != 2 && redirection_flag != -1) {
                    redirection_flag = -1;  
                } else {
                    redirection_flag = 1;
                }
            }

            if(redirection_flag == 1) {
                strip_space(array[1]);
                if (*(array)[1] ==  '\0') {;
                    redirection_flag = -1;   
                } else if(strstr(array[1], " ")) { // if there are too many files
                    redirection_flag = -1; 
                }
            }

            if(redirection_flag == -1) {
                fprintf(stderr, "Redirection misformatted.\n"); 
                fflush(stderr);
            }

            //strip_space(array[0]);
            if(redirection_flag != -1) {
            // put commands into argv from array 
                char* cmd_argv[TOKEN_BUF];
                int i = 0;
                char* token = strtok(array[0], " ");
                while(token != NULL) {
                    cmd_argv[i] = token;
                    token = strtok(NULL, " ");
                    i++; 
                }
                
                cmd_argv[i] = NULL;
                if(redirection_flag == 0) {
                    run_cmd(cmd_argv, NULL);
                } else {
                    run_cmd(cmd_argv, array[1]);
                }
                
            }

        }
        if(mode == 0) {
            fprintf(stdout, "mysh> ");
            fflush(stdout);
        }
    }

    if(mode == 1) 
        fclose(fstream);
    if(head != NULL) {
        free_list();
    }
    return 0;
}


