#include <stdio.h>
#include <assert.h>
#include <pthread.h>

void *mythread(void *arg) {
    printf("%s\n", (char *) arg);
}

int main() {
    pthread_t threads[2];
    printf("Main begin\n");
    char * a = "A";
    for(int i = 0; i < 2; i++) {
        pthread_create(&threads[i], NULL, mythread, a);
        a = "B";
    }
    pthread_join(threads[0], NULL);
    
    printf("Main end\n");
    return 0;
}