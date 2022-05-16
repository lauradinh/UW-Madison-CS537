#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int fsize(FILE *file){
    int pos = ftell(file);
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file,pos,SEEK_SET);
    return size;
}

int main(int argc, char* argv[]) {
    char** new_argv = malloc((argc+1) * sizeof *new_argv);
    for(int i = 1; i < argc; ++i)
    {
        size_t length = strlen(argv[i])+1;
        new_argv[i-1] = malloc(length);
        memcpy(new_argv[i-1], argv[i], length);
    }
    new_argv[argc-1] = NULL;


    for(int i = 0; i < argc-1; i++) {
        int index = i;
        char* temp;
        for(int j = i + 1; j < argc - 1; j++) {
            FILE *file1 = fopen(new_argv[j], "r");
            FILE *file2 = fopen(new_argv[index], "r");
            if(fsize(file1) < fsize(file2)) {
                index = j;
                fclose(file1);
                fclose(file2);
            }
            temp = strdup(new_argv[i]);
            new_argv[i] = strdup(new_argv[index]);
            new_argv[index] = strdup(temp);
        }
    }

    // do operations on new_argv
    for(int i = 0; i < argc-1; ++i)
    {
        printf("%s\n", new_argv[i]);
    }


    // free memory
    for(int i = 0; i < argc-1; ++i)
    {
        free(new_argv[i]);
    }
    free(new_argv);
    //printf("size= %d\n", size);
}