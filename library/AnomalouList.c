#include "AnomalouList.h"



void CreateList(){
    head = NULL;
}

void AddItem(HWND item){
    if(head == NULL){
            head = malloc(sizeof(struct NODE));
            head->header = item;
            head->editFlag = 1;
            head->next = NULL;
    }else{
        struct NODE *node = head;
        while(node->next != NULL){
            node = node->next;
        }
        node->next = malloc(sizeof(struct NODE));
        node = node->next;
        node->header = item;
        node->editFlag = 1;
        node->next = NULL;
    }
}

void RemoveItem(HWND item){
    if(head != NULL){
        struct NODE *node = head;
        if(head->header == item){
            head = head->next;
            return;
        }
        while(node->next != NULL){
            if(node->next->header == item){
                node->next = node->next->next;
                return;
            }
            node = node->next;
        }
    }
}

struct NODE* GetNode(HWND item){
    struct NODE* node = head;
    while(node != NULL){
        if(node->header == item){
            return node;
        }
        node = node->next;
    }
    return NULL;
}