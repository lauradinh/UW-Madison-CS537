#ifndef __linkedlist_h__
#define __linkedlist_h__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
    char* key;
    void* value;
} Node;

typedef struct {
    MapPair** contents;
    size_t capacity;
    size_t size;
} LinkedList;

struct node* search(char* key);

int insert(char* key, char* value);

int delete(char* key);

int size(void);

int cmp(const void* a, const void* b);

void sort(void);

#endif // __linkedlist_h__