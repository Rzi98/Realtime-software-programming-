#include <stdio.h>
#include <stdlib.h>
#include <direct.h>
#include <errno.h>

void currPath(char **retptr);

int main(void)
{
    char *retptr;
    currPath(&retptr);
    printf("(MAIN) Current working directory: %s\n", retptr);

    free(retptr);
    return 0;
}

void currPath(char **retptr)
{
    *retptr = _getcwd(NULL, 1024); // dynamically allocate memory for characters of path even if exceeds 1024

    if (*retptr == NULL)
    {
        printf("Failed to get current working directory.\n");

        if (errno == ERANGE)
            printf("Path exceeds max buffer length.\n");
        else if (errno == ENOMEM)
            printf("Memory cannot be allocated for path.\n");
    }

    printf("(CurrPath) Current working directory: %s\n", *retptr);
}