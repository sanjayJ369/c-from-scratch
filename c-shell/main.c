#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

int session();
int get_tokens(char *str, char *delim, char ***tokens);
void free_string_array(char **tokens, int count);
int get_files_in_dir(char *dir, char ***files);
int check_string_in_string_array(char **arr, int count, char *str);
char *get_cmd_dir(char *cmd);

int  main(void)
{
    while (1) {
        session();
    }
}


int get_tokens(char *str, char *delim, char ***tokens)
{
    int capacity = 8;
    int count = 0;
    *tokens = malloc(capacity * sizeof(char *));
    if (tokens == NULL)
    {
        printf("get tokens could not allocate memory ");
        return -1;
    }

    char *duplicate = (char *)malloc(sizeof(char) * (strlen(str) + 1));
    if (duplicate == NULL)
    {
        printf("get tokens could not allocate memory");
        return -1;
    }
    strcpy(duplicate, str);

    char *p = strtok(duplicate, delim);
    while (p != NULL)
    {
        if (count == capacity)
        {
            capacity *= 2;
            *tokens = realloc(*tokens, capacity * sizeof(char *));
            if (*tokens == NULL)
            {
                printf("get tokens failed to allocate memory");
            }
        }

        (*tokens)[count] = (char *) malloc(sizeof(char) * (strlen(p) + 1));
        strcpy((*tokens)[count], p);
        count++;
        p = strtok(NULL, delim);
    }
    free(duplicate);
    return count;
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

void free_string_array(char **tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

int get_files_in_dir(char *dirpath, char ***files) {
    int capacity = 8;
    int count = 0;
    *files = malloc(sizeof(char *) * capacity);
    DIR *d;
    struct dirent *dir;
    d = opendir(dirpath);
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (count == capacity) {
                *files = realloc(*files, capacity * 2 * sizeof(char *));
                if (files == NULL) {
                    printf("get_files_in_dir, error reallocating memory");
                }
                capacity *= 2;
            }
            (*files)[count] = malloc(sizeof(char) * (strlen(dir->d_name) + 1));
            strcpy((*files)[count], dir->d_name);
            count++;
        }
        closedir(d);
    }

    return count;
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

char *get_cmd_dir(char *cmd) {
    char *paths = getenv("PATH");
    char **dirs = NULL;
    int dir_count = get_tokens(paths, ":", &dirs);

    // for each dir
    for (int i = 0; i < dir_count; i++) {
        // get files in director
        char *execpath = malloc(sizeof(char) * (strlen(dirs[i]) + strlen(cmd) + 2));
        strcpy(execpath, dirs[i]);
        strcat(execpath, "/");
        strcat(execpath, cmd);
        if (access(execpath, X_OK) == 0) {
            return execpath;
        }
        free(execpath);
    }
    free_string_array(dirs, dir_count);
    return NULL;
}
