#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include "mapreduce.h"
#include "hashmap.h"

struct kv {
    char* key;
    char* value;
    int partition;
};

struct node {
    struct kv* data;
    struct node* next;
};

struct args {
    char *key;
    Getter get_next;
    int partition_number;

};

struct node* head;       // head of linked list
size_t kvl_counter;     // number of kv in linked list
pthread_mutex_t lock;
Partitioner partition_func;
Reducer reduce_func;
int n_reducers;


// adds kv to list
void add_to_list(struct kv* elt) {

    if(head == NULL) {
        pthread_mutex_lock(&lock);
        // might need to add a temp variable
        head = (struct node*)malloc(sizeof(struct node));
        head->data = (struct kv*) malloc(sizeof(struct kv));
        head->data->key = strdup(elt->key);
        head->data->value = strdup(elt->value);
        head->data->partition = elt->partition;
        pthread_mutex_unlock(&lock);
    } else {
        pthread_mutex_lock(&lock);
        // may need to change so that we just insert at the head
        struct node* current = head;
        while(current->next != NULL) {
            current = current->next;
        }
        struct node* new = (struct node*) malloc(sizeof(struct node));
        new->data = (struct kv*) malloc(sizeof(struct kv));
        new->data->key = strdup(elt->key);
        new->data->value = strdup(elt->value);
        new->data->partition = elt->partition;
        new->next = NULL;
        current->next = new;
        pthread_mutex_unlock(&lock);
    }
}

int size(void) {
    int size = 0;
    struct node *current;
    // lock
    if(head == NULL) {
        return 0;
    }
    current = head; 
    while(current != NULL) {
        current = current->next;
        size++;
    }
    // unlock
    return size;
}

void sort_partition(void) {
    struct node *current;
    struct node *next;
    struct kv* temp;
    int length = size();
    int k = size();
    for(int i = 0; i < length - 1; i++, k--){
        // lock
        current = head;
        next = head->next;
        for(int j = 1; j < k; j++) {
            if(current->data->partition > next->data->partition) {
                temp = current->data;
                current->data = next->data;
                next->data = temp;
            }
            current = current->next;
            next = next->next;
        }
    }
    // unlcok
}

void sort_key(void) {
    struct node *current;
    struct node *next;
    struct kv* temp;
    int length = size();
    int k = size();
    for(int i = 0; i < length - 1; i++, k--){
        // lock
        current = head;
        next = head->next;
        for(int j = 1; j < k; j++) {
            if(strcmp(current->data->key, next->data->key) > 0 && current->data->partition == next->data->partition) {
                temp = current->data;
                current->data = next->data;
                next->data = temp;
            }
            current = current->next;
            next = next->next;
        }
    } 
    // unlock
}

struct kv* search(int index) {
    printf("Search Function\n");
    int count = 0;
    int length = size();
    if (index >= length) {
        printf("Index out of bounds\n");
        return NULL;
    }
    struct node *curr = head;
    while(curr != NULL) {
        if(index == count){
            return curr->data;
        }
        curr = curr->next;
        count++;
    }
}

int fsize(FILE *file){
    int pos = ftell(file);
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file,pos,SEEK_SET);
    return size;
}

// iterates through each key
char* get_func(char* key, int partition_number) {
    printf("get_func\n");
    if (kvl_counter == size()) {
	    return NULL;
    }
    struct kv *curr_elt = search(kvl_counter);
    // printf("curr_elt: %s key: %s\n", curr_elt->key, key);
    
    if (!strcmp(curr_elt->key, key)) {
        pthread_mutex_lock(&lock);
        kvl_counter++;
        pthread_mutex_unlock(&lock);
        return curr_elt->value;
    }
    return NULL;
}

// Puts the key and value into the data structure
void MR_Emit(char* key, char* value)
{
    struct kv *elt = (struct kv*) malloc(sizeof(struct kv));
    if (elt == NULL) {
	    printf("Malloc error! %s\n", strerror(errno));
	    exit(1);
    }
    elt->key = strdup(key);
    elt->value = strdup(value);
    elt->partition = (*partition_func)(elt->key, n_reducers);
    add_to_list(elt);

    return;
}

/*
 * Gives the partition number for a given key
*/
unsigned long MR_DefaultHashPartition(char *key, int num_partitions) {
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0') {
        hash = hash * 33 + c;
    }
    return hash % num_partitions;
}

void *wapper(void *arguments) {
    (*reduce_func)(((struct args*)arguments)->key, ((struct args*)arguments)->get_next, ((struct args*)arguments)->partition_number);
}

void MR_Run(int argc, char *argv[], Mapper map, int num_mappers,
    Reducer reduce, int num_reducers, Partitioner partition)
{
    pthread_mutex_init(&lock, NULL);
    // copies files from argv to new array
    char** files = malloc((argc-1) * sizeof *files);
    for(int i = 1; i < argc; ++i)
    {
        size_t length = strlen(argv[i])+1;
        files[i-1] = malloc(length);
        memcpy(files[i-1], argv[i], length);
    }
    files[argc-1] = NULL;

    // sorts the files by size
    for(int i = 0; i < argc-1; i++) {
        int index = i;
        char* temp;
        for(int j = i + 1; j < argc - 1; j++) {
            FILE *file1 = fopen(files[j], "r");
            FILE *file2 = fopen(files[index], "r");
            if(fsize(file1) < fsize(file2)) {
                index = j;
                fclose(file1);
                fclose(file2);
            }
            temp = strdup(files[i]);
            files[i] = strdup(files[index]);
            files[index] = strdup(temp);
        }
    }

    partition_func = partition;
    n_reducers = num_reducers;

    // creates mapper threads and runs new threads
    pthread_t mapthreads[num_mappers];
    int filecount = 0;
    int i = 0;
    while(filecount != argc-1) {
        if (i >= num_mappers) {
            i = 0;
            continue;
        }
        pthread_create(&mapthreads[i], NULL, (void *)map, files[filecount]);
        // printf("Index: %d file: %d thread: %ld\n", i, filecount, mapthreads[i]);
        i++;
        filecount++;
    }

    for(int j = 0; j < argc-1; j++) {
        pthread_join(mapthreads[j], NULL);
    }

    sort_partition();
    sort_key();

    // prints out linkedlist
    // struct node * curr = head;
    // while(curr != NULL) {
    //     printf("%s %d\n", curr->data->key, curr->data->partition);
    //     curr = curr->next;
    // }

    reduce_func = reduce;
    pthread_t reducethreads[num_reducers];
    kvl_counter = 0;
    struct kv* pair;
    // create array of threads
    i = 0;

    int part_ind = 0; // the partition
    while (i < num_reducers) {

        int part_count = 0;
        struct node * curr = head;
        while(curr != NULL){
            if(part_ind == curr->data->partition){
                part_count++; // number of keys in a partition
            }
            curr = curr->next;
        }
        printf("%d\n", part_count);
        // for(int j = 0; j < part_count; j++) {
        pair = search(kvl_counter);
        printf("Pair: %s %s %d\n", pair->key, pair->value, pair->partition);
        if(pair != NULL) {
            // create threads to run this 
            struct args* arg = (struct args*) malloc(sizeof(struct args));
            arg->key = strdup(pair->key);
            arg->get_next = get_func;
            arg->partition_number = pair->partition;
            // printf("%s %d\n", arg->key, arg->partition_number);
            pthread_create(&reducethreads[i], NULL, (void *)wapper, (void *)arg);
            // pthread_join(reducethreads[i], NULL);
        }
        // }
        part_ind++;
        i++;
    }
    for(int j = 0; j < argc-1; j++) {
        pthread_join(reducethreads[j], NULL);
    }
}

    // while (kvl_counter < size()) {
    //     pair = search(kvl_counter);
    //     if(pair != NULL) {
    //         // create threads to run this 
    //         (*reduce)(pair->key, get_func, pair->partition);       // goes thorugh each element and returns 
    //     }
    // }



