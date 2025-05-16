#include "main.h"

#define TBBUTTON_COMMAND_OFFSET (WM_APP + 100)

struct ToolButton {
    wchar_t *text;
    int bmpId;
    void(*proc)();
    bool separate;
};

struct ToolButton buttons[] = {
    {STR_UP, 0, &onMenuItemUpClick, true},
    {STR_COPY, 1, &onMenuItemCopyClick, false},
    {STR_CUT, 2, &onMenuItemCutClick, false},
    {STR_PASTE, 3, &onMenuItemPasteClick, false},
    {STR_DELETE, 4, &onMenuItemDeleteClick, true},
    {STR_NEW_FOLDER, 5, &onMenuItemNewFolderClick, false},
    {STR_NEW_FILE, 6, &onMenuItemNewFileClick, false}
};

static WNDPROC OrigWndProc;

extern HINSTANCE globalHInstance;
extern HWND hwndMain;

HWND hwndToolbar = NULL;

LRESULT CALLBACK ToolbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    return OrigWndProc(hwnd, msg, wParam, lParam);
}

void createToolButtons() {
    const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

    SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);
    SendMessage(hwndToolbar, TB_SETINDENT, 2, 0);

    HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_COLOR32, numButtons, 0);
    
    HICON hiUp = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_UP), IMAGE_ICON, 16, 16, 0);
    HICON hiCopy = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_COPY), IMAGE_ICON, 16, 16, 0);
    HICON hiCut = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_CUT), IMAGE_ICON, 16, 16, 0);
    HICON hiPaste = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_PASTE), IMAGE_ICON, 16, 16, 0);
    HICON hiDelete = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_DELETE), IMAGE_ICON, 16, 16, 0);
    HICON hiNewFolder = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_NEW_FOLDER), IMAGE_ICON, 16, 16, 0);
    HICON hiNewFile = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_NEW_FILE), IMAGE_ICON, 16, 16, 0);
    
    ImageList_AddIcon(hImageList, hiUp);
    ImageList_AddIcon(hImageList, hiCopy);
    ImageList_AddIcon(hImageList, hiCut);
    ImageList_AddIcon(hImageList, hiPaste);
    ImageList_AddIcon(hImageList, hiDelete);
    ImageList_AddIcon(hImageList, hiNewFolder);
    ImageList_AddIcon(hImageList, hiNewFile);
    
    SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

    TBBUTTON tbbSeparator = {0};
    tbbSeparator.fsStyle = BTNS_SEP;

    TBBUTTON tbButton;
    tbButton.fsState = TBSTATE_ENABLED;
    tbButton.fsStyle = BTNS_BUTTON;

    for (int i = 0, j = 0; i < numButtons; i++, j++) {
        tbButton.iBitmap = buttons[i].bmpId;
        tbButton.idCommand = TBBUTTON_COMMAND_OFFSET + i;
        tbButton.iString = (INT_PTR)buttons[i].text;

        SendMessage(hwndToolbar, TB_INSERTBUTTON, j, (LPARAM)&tbButton);

        if (buttons[i].separate) SendMessage(hwndToolbar, TB_INSERTBUTTON, ++j, (LPARAM)&tbbSeparator);
    }   
}

void createToolbar() {
    hwndToolbar = CreateWindowEx(0, TOOLBARCLASSNAME, NULL, WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | TBSTYLE_FLAT | CCS_NORESIZE, 
                                 0, 0, 0, 0, hwndMain, NULL, globalHInstance, NULL);
    createToolButtons();
    
    OrigWndProc = (WNDPROC)SetWindowLongPtr(hwndToolbar, GWLP_WNDPROC, (LONG_PTR)ToolbarWndProc);
    UpdateWindow(hwndToolbar);
}

void toolbarCommand(int command) {
    (buttons[command - TBBUTTON_COMMAND_OFFSET].proc)();
}