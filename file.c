#include <stdio.h>
#include "file.h"

int openfile(char *file_name)
{
    fptr = fopen(file_name, "r");
    if (fptr != NULL)
    {
        printf("File opened Successfully !\n");
        return 1;
    }
    else
    {
        printf("Error : Unable to open the file\n");
        return 0;
    }
}