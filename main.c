#ifndef UINCODE
#define UNICODE


#include "library/AnomalouList.h"
#include <windows.h>
#include <winuser.h>
#include <commdlg.h>
#include <stdio.h>
#include <richedit.h>

#define TBM_OPEN 100
#define TBM_CASCADE 101
#define TBM_TILE 102
#define AWM_TEXT 2000
#define AWM_SENDTEXT 2001


HINSTANCE hInst;

HWND mainHWND;
HWND clientHWND;

int num;

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
LRESULT CALLBACK ChildProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

void RegMainWindow();
void RegClientWindow(HWND hwnd);


WINAPI int WinMain(HINSTANCE hInstance, HINSTANCE lhInstance, LPSTR commands, INT comNum){
    hInst = hInstance;

    RegMainWindow();

    MSG msg = {};

    while(GetMessage(&msg, NULL, 0, 0)){
        if(!TranslateMDISysAccel(clientHWND, &msg)){
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return 0;
}

void RegMainWindow(){
    WNDCLASS class = {};
    class.hInstance = hInst;
    class.lpfnWndProc = WinProc;
    class.lpszClassName = L"BaseWindow";
    class.hCursor = LoadCursor(NULL, IDC_ARROW);
    class.style = CS_HREDRAW | CS_VREDRAW;

    RegisterClass(&class);

    mainHWND = CreateWindow(L"BaseWindow", L"Main window", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInst, NULL);
    ShowWindow(mainHWND, SW_NORMAL);

    HMENU hMenu = CreateMenu();

    HMENU hSubMenu = CreatePopupMenu();
    AppendMenu(hSubMenu, MF_STRING, TBM_OPEN, L"&Open");
    AppendMenu(hSubMenu, MF_STRING, TBM_CASCADE, L"&Cascade");
    AppendMenu(hSubMenu, MF_STRING, TBM_TILE, L"&Tile");
    AppendMenu(hMenu, MF_STRING | MF_POPUP, (UINT)hSubMenu, L"&MDI");

    SetMenu(mainHWND, hMenu);
}

void RegClientWindow(HWND hwnd){
    WNDCLASS childClass = {};
    childClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    childClass.lpszClassName = L"MDIChild";
    childClass.lpfnWndProc = ChildProc;
    RegisterClass(&childClass);
    CLIENTCREATESTRUCT ccs;
    ccs.hWindowMenu = GetSubMenu(GetMenu(hwnd), 1);
    ccs.idFirstChild = 1000;
    clientHWND = CreateWindow(L"MDICLIENT", (LPCWSTR)NULL, WS_CHILD | WS_CLIPCHILDREN | WS_VSCROLL | WS_HSCROLL | WS_VISIBLE | MDIS_ALLCHILDSTYLES, 0, 0, 0, 0, hwnd, NULL, NULL, &ccs);
}

LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    LPWSTR filePath = malloc(sizeof(WCHAR) * 256);
    HFILE hFile;
    HWND mdiHWND;
    DWORD ol;

    LPSTR str = malloc(sizeof(CHAR) * 1024);

    LONG_PTR data = GetWindowLongPtr(hwnd, GWLP_USERDATA);
    
    if(data != 0){
        str = (LPSTR)data;
    }

    switch(msg){
        case WM_CREATE:
            RegClientWindow(hwnd);
            CreateList();
        break;
        case AWM_SENDTEXT:
        {
            struct NODE *node = head;
            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)lparam);
            while(node != NULL){
                SendMessageA(node->header, AWM_TEXT, 0, (LPSTR)lparam);
                node = node->next;
            }
        }
        break;
        case WM_COMMAND:
        {
            switch(LOWORD(wparam)){
                case TBM_OPEN:
                {
                    MDICREATESTRUCT mdc = {};
                    mdc.szClass = L"MDIChild";
                    mdc.szTitle = L"MDI window";
                    mdc.x = CW_USEDEFAULT;
                    mdc.y = CW_USEDEFAULT;
                    mdc.cx = CW_USEDEFAULT;
                    mdc.cy = CW_USEDEFAULT;
                    mdc.style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
                    mdc.hOwner = hInst;
                    if(FindWindowEx(clientHWND, NULL, L"MDIChild", L"MDI window") == NULL){
                        OPENFILENAME ofn = {};
                        ofn.hInstance = hInst;
                        ofn.lStructSize = sizeof(OPENFILENAME);
                        ofn.hwndOwner = clientHWND;
                        ofn.lpstrFile = filePath;
                        ofn.nMaxFile = sizeof(WCHAR) * 256;
                        ofn.lpstrFilter = L"all\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.lpstrFileTitle = NULL;
                        ofn.nMaxFileTitle = 0;
                        ofn.lpstrInitialDir = NULL;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
                        ofn.lpstrTitle = L"Select file";
                        ZeroMemory(filePath, sizeof(filePath));
                        if(GetOpenFileNameW(&ofn) == TRUE){
                            ZeroMemory(str, sizeof(str));
                            hFile = CreateFile(filePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
                            ReadFile(hFile, str, 255, &ol, NULL);
                            str[ol] = '\0';
                            SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)str);
                            CloseHandle(hFile);
                        }else{
                            break;
                        }
                    }
                    mdiHWND = SendMessage(clientHWND, WM_MDICREATE, 0, &mdc);
                    AddItem(mdiHWND);
                    SendMessage(mdiHWND, AWM_TEXT, 0, str);
                }
                break;
                case TBM_CASCADE:
                    SendMessage(clientHWND, WM_MDICASCADE, MDITILE_SKIPDISABLED, 0);
                break;
                case TBM_TILE:
                    SendMessage(clientHWND, WM_MDITILE, MDITILE_HORIZONTAL, 0);
                break;
            }

        }
        break;
        case WM_DESTROY:
            PostQuitMessage(0);
        break;
        case WM_CLOSE:
            DestroyWindow(hwnd);
        break;
    }
    return DefFrameProc(hwnd, clientHWND, msg, wparam, lparam);
}

LRESULT CALLBACK ChildProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam){
    HDC hdc;
    PAINTSTRUCT ps;

    HWND textEdit;

    RECT size;

    LPSTR str = malloc(sizeof(CHAR) * 1024);

    if(msg == WM_CREATE){
        LoadLibrary(TEXT("Msftedit.dll"));
        textEdit = CreateWindow(MSFTEDIT_CLASS, L"", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | ES_AUTOVSCROLL | ES_MULTILINE, 5, 5, 100, 60, hwnd, NULL, hInst, NULL);
        SendMessage(textEdit, EM_SETEVENTMASK, 0, ENM_CHANGE);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)textEdit);
    }else{
        textEdit = (HWND)GetWindowLongPtr(hwnd, GWLP_USERDATA);
    }

    switch(msg){
        case WM_COMMAND:
            switch(HIWORD(wparam)){
                case EN_CHANGE:
                {
                    LPSTR str = malloc(sizeof(CHAR) * 1024);
                    GetWindowTextA(textEdit, str, 1024);
                    SendMessageA(mainHWND, AWM_SENDTEXT, 0, str);
                }
                break;
            }
        break;
        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
            EndPaint(hwnd, &ps);
        break;
        case AWM_TEXT:
            SendMessageA(textEdit, WM_SETTEXT, 0, (LPSTR)lparam);
            SendMessageA(textEdit, EM_SETSEL, strlen((LPSTR)lparam), strlen((LPSTR)lparam));
        break;
        case WM_SIZE:
            GetClientRect(hwnd, &size);
            MoveWindow(textEdit, 5, 5, size.right - 10, size.bottom - 10, TRUE);
        break;
        case WM_CLOSE:
            RemoveItem(hwnd);
            SendMessage(clientHWND, WM_MDIDESTROY, hwnd, 1);
        break;
    }
    return DefMDIChildProc(hwnd, msg, wparam, lparam);
}

#endif