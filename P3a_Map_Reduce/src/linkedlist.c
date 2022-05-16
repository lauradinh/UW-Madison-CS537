#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct node {
	char* key;
	char* value;
	struct node *next;
};

extern struct node* head = NULL;

struct node* search(char* key) {
	struct node* current = head;
	if(head == NULL) {
		return NULL;
	}

	while(strncmp(current->key, key, strlen(key))) {
		if(current->next == NULL) {
			return NULL;
		} else {
			current = current -> next;
		}
	}
	return current;
}

int insert(char* key, char* value) {
// 1 - success, 0 - otherwise
	struct node* current = search(key);
	if(head == NULL) {
		head = (struct node*)malloc(sizeof(struct node));
        	head->key = strdup(key);
        	head->value = strdup(value);
        	head->next = NULL;
		return 1;
	} else if (current != NULL) {
		current->value = strdup(value);
	} else {
		current = head;
		while(strcmp(current->key, key)) {
			if(current->next == NULL) {
				break;
			} else {
				current = current->next;
			}
		}
		struct node* new = (struct node*) malloc(sizeof(struct node));
        
		new->key = strdup(key);
		new->value = strdup(value);
		new->next = NULL;
		current->next = new;
		return 1;
	}
	return 0;
}

int delete(char* key) {
// 1 - success, 0 - otherwise
	struct node* current = head;
	struct node* previous = NULL;
	
	if(head == NULL) {
		return 0;
	}

	while(strncmp(current->key, key, strlen(key) - 1)) {
		if(current->next == NULL) {
			return 0;
		} else {
			previous = current;
			current = current->next;
		}
	}

	if(current == head) {
		head = head-> next;
	} else {
		previous->next = current->next;
	}
	return 1;
}

int size(void){
    int size = 0;
    struct node *current;

    if(head == NULL) {
        return size;
    } else {
        current = head;
        size++;
    }
    while(current != NULL) {
        current = current->next;
        size++;
    }
    return size;
}

int cmp(const void* a, const void* b) {
    char* str1 = (*(struct Node **)a)->key;
    char* str2 = (*(struct Node **)b)->key;
    return strcmp(str1, str2);
}

void sort(void){
    struct node *current;
    struct node *next;
    char* tempvalue, tempkey;
    int length = size();
    int k = size();
    for(int i = 0; i < length - 1; i++, k--){
        current = head;
        next = head->next;
        for(int j = 1; j < k; j++) {
            if(strcmp(current->key, next->key) > 0) {
                tempkey = current->key;
                current->key = next->key;
                next->key = tempkey;
                tempvalue = current->value;
                current->value = next->value;
                next->value = tempvalue;

            }
            current = current->next;
            next = next->next;
        }
    }
}

int main(){
	insert("1", "one");
	insert("5", "five");
	insert("2", "two");
	sort();

	printf("\n[ ");
	struct node *current = head;
   //start from the beginning
   while(current != NULL) {
      printf("(%s,%s) ",current->key,current->value);
      current = current->next;
   }
	
   printf(" ]");
	
}