#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

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

void free_string_array(char **tokens, int count) {
    for (int i = 0; i < count; i++) {
        free(tokens[i]);
    }
    free(tokens);
}