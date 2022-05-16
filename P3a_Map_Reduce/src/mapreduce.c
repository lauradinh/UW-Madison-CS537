#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "mapreduce.h"
#include "hashmap.h"

#define BUFFER  1024

struct kv {
    char* key;
    char* value;
};

struct file {
    char *name;
};

struct kv** kvm;            // 2D matrix
struct file* filenames;
int* partition_count;
int* partition_size;
int* partition_access;
pthread_mutex_t lock;       //lock
Mapper map_func;
int n_mappers;
Reducer reduce_func;
int n_reducers;
Partitioner part_func;
int n_partitions;
int file_count;
int file_curr;


char* get_func(char* key, int partition_number) {
    int num = partition_count[partition_number];
    if (num < partition_access[partition_number] && strcmp(key, kvm[partition_number][num].key) == 0) {
        partition_count[partition_number]++;
        return kvm[partition_number][num].value;
    }
    return NULL;
}

int cmp(const void* a, const void* b) {
    char* str1 = ((struct kv *)a)->key;
    char* str2 = ((struct kv *)b)->key;
    return strcmp(str1, str2);
}

int fsize(FILE *file){
    int pos = ftell(file);
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file,pos,SEEK_SET);
    return size;
}

int cmp_file(const void* a, const void* b) {
    struct file* file1 = (struct file*) a;
    struct file* file2 = (struct file*) b;
    FILE *fileA = fopen(file1->name, "r");
    FILE *fileB = fopen(file2->name, "r");
    int size1 = fsize(fileA);
    int size2 = fsize(fileB);
    fclose(fileA);
    fclose(fileB);
    return (size1 - size2);
}

void* map_wrapper(void* args) {
    while(file_curr < file_count) {
        char* name = NULL;
        pthread_mutex_lock(&lock);
        if (file_curr < file_count) {
            name = filenames[file_curr].name;
            file_curr++;
        }
        pthread_mutex_unlock(&lock);
        if(name != NULL) {
            (*map_func)(name);
        }
    }
    return NULL;
}

void* reducer_wrapper(void* args) {
    int* partition = (int *) args;
    for(int i = 0; i < partition_access[*partition]; i++) {
        if (i == partition_count[*partition]) {
            (* reduce_func)(kvm[*partition][i].key, get_func, *partition);
        }
    }
    return NULL;
}

void MR_Emit(char* key, char* value)
{
    unsigned long partition = (part_func)(key, n_partitions);
    pthread_mutex_lock(&lock);
    if (partition_access[partition] == partition_size[partition]) {
        partition_size[partition] *= 2;
        kvm[partition] = (struct kv*)realloc(kvm[partition], partition_size[partition] * sizeof(struct kv));
    }
    partition_access[partition]++;
    kvm[partition][partition_access[partition] - 1].key = (char *)malloc((strlen(key)+1 * sizeof(char)));
    strcpy(kvm[partition][partition_access[partition] - 1].key, key);
    kvm[partition][partition_access[partition] - 1].value = (char *)malloc((strlen(value)+1 * sizeof(char)));
    strcpy(kvm[partition][partition_access[partition] - 1].value, value);
    // printf("key: %s value: %s\n", kvm[partition][partition_size[partition] - 1].key, kvm[partition][partition_size[partition] - 1].value);
    pthread_mutex_unlock(&lock);
    return;
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return hash % num_partitions;
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers,
	    Reducer reduce, int num_reducers, Partitioner partition)
{
    pthread_mutex_init(&lock, NULL);
    map_func = map;
    reduce_func = reduce;
    n_mappers = num_mappers;
    n_reducers = num_reducers;
    part_func = partition;
    n_partitions = num_reducers;

    file_count = argc - 1;
    file_curr = 0;

    //  initialized the arrays
    kvm = (struct kv**) malloc(n_reducers * sizeof(struct kv*));
    filenames = (struct file*)malloc(file_count * sizeof(struct file));
    partition_count = (int *)malloc(n_reducers * sizeof(int));
    partition_size = (int *)malloc(n_reducers * sizeof(int));
    partition_access = (int *)malloc(n_reducers * sizeof(int));
    int position[n_reducers];

    for(int i = 0; i < n_reducers; i++) {
        kvm[i] = malloc(BUFFER * sizeof(struct kv));
        partition_count[i] = 0;
        partition_size[i] = BUFFER;
        partition_access[i] = 0;
        position[i] = i;

    }

    // copy files
    for(int i = 0; i < file_count; i++) {
		filenames[i].name = malloc((strlen(argv[i+1])+1) * sizeof(char));
		strcpy(filenames[i].name, argv[i+1]);
	}
    
    qsort(&filenames[0], file_count, sizeof(struct file), cmp_file);

    pthread_t mapthreads[n_mappers];
    for(int i = 0; i < n_mappers ; i++) {
        pthread_create(&mapthreads[i], NULL, map_wrapper, NULL);
    }

    for(int i = 0; i < n_mappers ; i++) {
        pthread_join(mapthreads[i], NULL);
    }

    for(int i = 0; i < n_reducers; i++) {
        qsort(kvm[i], partition_access[i], sizeof(struct kv), cmp);
    }

    // for(int i = 0; i < num_reducers; i++) {
	// 	printf("Reducer number: %d\n", i);
	// 	for(int j = 0; j < partition_access[i]; j++) {
	// 		printf("%s ", (kvm[i][j].key));
	// 		printf("%s\n", (kvm[i][j].value));
	// 	}
	// }

    pthread_t reducethreads[n_reducers];
    for(int i = 0; i < n_reducers; i++) {
        pthread_create(&reducethreads[i], NULL, reducer_wrapper, &position[i]);
    }

    for(int i = 0; i < n_reducers; i++) {
        pthread_join(reducethreads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    for(int i  = 0; i < file_count; i++) {
        free(filenames[i].name);
    }
    free(filenames);

    for(int i = 0; i < num_reducers; i++) {
        for(int j = 0; j < partition_access[i]; j++) {
            if(kvm[i][j].key != NULL && kvm[i][j].value != NULL) {
                free(kvm[i][j].key);
                free(kvm[i][j].value);
            }
        }
        free(kvm[i]);
    }
    free(kvm);
    free(partition_count);
    free(partition_access);
    free(partition_size);
}