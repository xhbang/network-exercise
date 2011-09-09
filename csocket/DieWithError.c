#include <stdio.h>  /* for perror() */
#include <stdlib.h> /* for exit() */

void DieWithUserMessage(const char *msg, const char *detail){
    fputs(msg, stderr);
    fputs(":",stderr);
    fputs(detail,stderr);
    fputs("\n",stderr);
    exit(1);
}

void DieWithSystemMessage(const char *msg){
    perror(msg);
    exit(1);
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}
