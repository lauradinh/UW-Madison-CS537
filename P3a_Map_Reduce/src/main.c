#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mapreduce.h"
#include "hashmap.h"

HashMap* hashmap;

void Map(char *file_name) {
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1) {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL) {
	        if (!strcmp(token, ""))
		        break;
            // printf("%s\n", token);
            MR_Emit(token, "1");
        }
    }
    free(line);
    fclose(fp);
}

/* takes the key, a getter function, and a partition number to 
 * get the value of the kv pair 
 * i.e. in this partition that has this key, count the number
   of times this key appears */
   
void Reduce(char *key, Getter get_next, int partition_number) {
    // printf("Enter Reduce\n");
    // HashMap take a (void *) as value
    int *count = (int*)malloc(sizeof(int));
    *count = 0;
    char *value;
    // printf("Key: %s Partition: %d\n", key, partition_number);
    while ((value = get_next(key, partition_number)) != NULL) {
        // printf("Key: %s Partition: %d Value: %s\n", key, partition_number, value);
        (*count)++;
        //printf("key: %s value: %s count: %d\n", key, value, *count);
    }
    // printf("Key: %s count: %d\n", key,  *count);
    MapPut(hashmap, key, count, sizeof(int));
}

/* This program accepts a list of files and stores their words and
 * number of occurrences in a hashmap. After populating the hashmap,
 * it is search for a word (searchterm) and the number of occurrences
 * is printed. */

int main(int argc, char *argv[]) { 
    if (argc < 3) {
	    printf("Invalid usage: ./hashmap <filename> ... <searchterm>\n");       // needs the exec, file, and word to search for
	return 1;
    }
    
    hashmap = MapInit();              // creates hashmap
    // save the searchterm
    char* searchterm = argv[argc - 1]; // the search term ex. four
    argc -= 1; // removes search term ./mapreduce basic.txt

    // run mapreduce
    MR_Run(argc, argv, Map, 3, Reduce, 10, MR_DefaultHashPartition);
    // get the number of occurrences and print
    char *result;
    if ((result = MapGet(hashmap, searchterm)) != NULL) {
	    printf("Found %s %d times\n", searchterm, *(int*)result);
    } else {
	    printf("Word not found!\n");
    }
    
    return 0;
}
