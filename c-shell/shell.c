#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "parser.h"
#include "path_search.h"

int session();
int check_string_in_string_array(char **arr, int count, char *str);

void start_shell() {
    while (1) {
        session();
    }
}

int session() {
    // print terminal
    printf("\n-> ");
    char * cmd = NULL, * delim = " ";
    char **cmd_tokens = NULL;
    int tokens_len = 0;
    size_t buff_len = 0;
    ssize_t cmd_len;

    cmd_len = getline(&cmd, &buff_len, stdin);
    if (cmd_len == -1) {
        perror("failed to get command");
        return -1;
    }

    if (cmd[cmd_len - 1] == '\n') {
        cmd[cmd_len - 1] = '\0';
    }

    tokens_len = get_tokens(cmd, delim , &cmd_tokens);
    int wstatus;
    pid_t cpid, w;

    switch (cpid = fork()) {
        case -1:
            printf("\ncouldn't start the child process");
            return -1;
        case 0:
            // child process execute program
            // to do parse command

            char *program = get_cmd_dir(cmd_tokens[0]);
            if (program == NULL) {
                printf("command not found");
                break;
            }
            char *pgm_name = cmd_tokens[0];

            char **args = cmd_tokens;
            char *envp[] = {NULL};
            execve(program, args, envp);
            perror("\nexecve failed");
            break;
        default:
            w = waitpid(cpid, &wstatus, WUNTRACED);
            if (w == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }

            if (WIFEXITED(wstatus)) {
                // printf("terminatted normally \n");
            } else {
                printf("exited with status %d\n", wstatus);
            }
    }
    free_string_array(cmd_tokens, tokens_len);
    return 0;
}



// idx if present, -1 if it not present
int check_string_in_string_array(char **arr, int count, char *str) {
    for (int i = 0; i < count; i++) {
        if ( strcmp(arr[i], str) == 0) {
            return i;
        }
    }
    return -1;
}