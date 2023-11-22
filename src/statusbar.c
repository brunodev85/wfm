#include "main.h"

void setStatusbarText(wchar_t* text) {
	SetWindowText(hwndStatusbar, text); 
}

void createStatusbar() {
    hwndStatusbar = CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
								   0, 0, 0, 0, hwndMain, (HMENU)NULL, globalHInstance, NULL);
	UpdateWindow(hwndStatusbar);
}