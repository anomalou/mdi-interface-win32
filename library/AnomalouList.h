#ifndef ANOMALOU_LIST
#define ANOMALOU_LIST

#include <windows.h>

struct NODE{
    HWND header;
    struct NODE *next;
};

struct NODE *head;

void CreateList();
void AddItem(HWND item);
void RemoveItem(HWND item);
struct NODE* GetNode(HWND item);

#endif