#ifndef __hashmap_h__
#define __hashmap_h__

#define MAP_INIT_CAPACITY 11

typedef struct {
    char* key;
    void* value;
} MapPair;

typedef struct {
    MapPair** contents;
    size_t capacity;
    size_t size;
} HashMap;


// External Functions
HashMap* MapInit(void);
void MapPut(HashMap* map, char* key, void* value, int value_size);
char* MapGet(HashMap* map, char* key);
size_t MapSize(HashMap* map);

// Internal Functions
int resize_map(HashMap* map);
size_t Hash(char* key, size_t capacity);
// void BeginRead();                                   // CHANGED
// void EndRead();                                     // CHANGED
// void BeginWrite();                                  // CHANGED
// void EndWrite();                                    // CHANGED
//extern pthread_mutex_t lock;                               // added
pthread_rwlock_t read_lock;                                 // CHANGED





#endif // __hashmap_h__
