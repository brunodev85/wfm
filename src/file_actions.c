#include "main.h"

#define ID_EVENT_PRELOADER 100
#define PRELOADER_PERIOD 120

enum Msg {
    MSG_CLOSE = WM_APP,
    MSG_NAVIGATE_REFRESH
};

enum FileAction {
    ACTION_NONE,
    ACTION_DELETE,
    ACTION_COPY,
    ACTION_MOVE
};

struct ActionData {
    enum FileAction action;
    wchar_t** srcPaths;
    int numSrcPaths;
    wchar_t* dstPath;
    bool cancel;
};

static HWND hwndDlg;
static HICON preloaderIcons[8];
static int preloaderIconIndex = 0;
static wchar_t** clipboard = NULL;
static int clipboardSize = 0;
static bool clipboardIsCut = false;
static struct ActionData* actionData = NULL;

extern HINSTANCE globalHInstance;
extern HWND hwndMain;

static void animatePreloader() {
    SendDlgItemMessage(hwndDlg, IDC_PRELOADER, STM_SETICON, (WPARAM)preloaderIcons[preloaderIconIndex], 0);
    preloaderIconIndex = (preloaderIconIndex + 1) % 8;
}

void clearClipboard() {
    if (clipboard) {
        for (int i = 0; i < clipboardSize; i++) free(clipboard[i]);
        free(clipboard);
        clipboard = NULL;
    }
    clipboardSize = 0;
}

static void freeActionData() {
    if (clipboardIsCut) clearClipboard();
    
    if (actionData) {
        if (actionData->action == ACTION_DELETE) {
            for (int i = 0; i < actionData->numSrcPaths; i++) free(actionData->srcPaths[i]);
            free(actionData->srcPaths);
        }
        
        free(actionData->dstPath);
        free(actionData);
        actionData = NULL;
    }
}

INT_PTR CALLBACK FileActionDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    UNREFERENCED_PARAMETER(lParam);

    switch (msg) {
        case WM_INITDIALOG: {
            RECT rect, rect1;
            GetWindowRect(GetParent(hwndDlg), &rect);
            GetClientRect(hwndDlg, &rect1);
            SetWindowPos(hwndDlg, NULL, (rect.right + rect.left) / 2 - (rect1.right - rect1.left) / 2, (rect.bottom + rect.top) / 2 - (rect1.bottom - rect1.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            
            for (int i = 0; i < 8; i++) {
                preloaderIcons[i] = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_PRELOADER_1 + i), IMAGE_ICON, 64, 64, 0);
            }
            
            HWND hwndLabel = GetDlgItem(hwndDlg, IDC_LABEL);
            switch (actionData->action) {
                case ACTION_DELETE: {
                    SetWindowText(hwndDlg, L"Deleting files");
                    SetWindowText(hwndLabel, L"Deleting files, please wait...");                
                    break;
                }
                case ACTION_COPY: {
                    SetWindowText(hwndDlg, L"Copying files");
                    SetWindowText(hwndLabel, L"Copying files, please wait...");             
                    break;
                }
                case ACTION_MOVE: {
                    SetWindowText(hwndDlg, L"Moving files");
                    SetWindowText(hwndLabel, L"Moving files, please wait...");              
                    break;
                }               
            }
            return (INT_PTR)TRUE;
        }
        case WM_TIMER: {
            if (wParam == ID_EVENT_PRELOADER) animatePreloader();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDCANCEL) {
                if (MessageBox(hwndDlg, L"Do you want to cancel the operation?", L"Cancel", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    actionData->cancel = true;
                }
            }
            break;
        }
        case MSG_CLOSE: {
            freeActionData();
            DestroyWindow(hwndDlg);
            hwndDlg = NULL;
            navigateRefresh();
            break;
        }
        case MSG_NAVIGATE_REFRESH: {
            navigateRefresh();
            break;
        }
    }

    return (INT_PTR)FALSE;
}

static DWORD WINAPI fileActionTask(void* param) {
    struct ActionData* actionData = (struct ActionData*)param;
    DWORD lastTime = GetTickCount();

    for (int i = 0; i < actionData->numSrcPaths && !actionData->cancel; i++) {  
        if (actionData->action == ACTION_DELETE) {
            SHFILEOPSTRUCT sfo;
            memset(&sfo, 0, sizeof(sfo));
            sfo.hwnd = hwndDlg;
            sfo.wFunc = FO_DELETE;
            sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
            sfo.pTo = NULL;
            sfo.pFrom = actionData->srcPaths[i];
            
            int res = SHFileOperation(&sfo);
            if (res != 0) break;
        }
        else if (actionData->action == ACTION_COPY || actionData->action == ACTION_MOVE) {
            SHFILEOPSTRUCT sfo;
            memset(&sfo, 0, sizeof(sfo));
            sfo.hwnd = hwndDlg;
            sfo.wFunc = actionData->action == ACTION_COPY ? FO_COPY : FO_MOVE;
            sfo.fFlags = FOF_SILENT;
            sfo.pTo = actionData->dstPath;
            sfo.pFrom = actionData->srcPaths[i];

            int res = SHFileOperation(&sfo);
            if (res != 0) break;            
        }

        DWORD currTime = GetTickCount();
        if ((currTime - lastTime) >= 3000) {
            SendMessage(hwndDlg, MSG_NAVIGATE_REFRESH, 0, 0);
            lastTime = currTime;
        }
    }
    
    SendMessage(hwndDlg, MSG_CLOSE, 0, 0);
    return 0;
}

static wchar_t** createPathsFromFileNodes(struct FileNode** nodes, int count) {
    wchar_t** paths = malloc(count * sizeof(wchar_t*));
    
    wchar_t tmp[MAX_PATH];
    for (int i = 0; i < count; i++) {
        getFileNodePath(nodes[i], tmp);
        int len = wcslen(tmp);
        wchar_t* path = malloc((len + 2) * sizeof(wchar_t));
        wcscpy_s(path, len + 1, tmp);
        path[len+0] = '\0';
        path[len+1] = '\0';         
        paths[i] = path;
    }
    
    return paths;
}

void deleteFiles(struct FileNode** nodes, int count) {
    wchar_t msg[128];
    if (count == 1) {
        swprintf_s(msg, 128, L"Are you sure you want to delete \"%ls\"?", nodes[0]->name);
    }
    else swprintf_s(msg, 128, L"Are you sure you want to delete the %d items?", count);

    if (MessageBox(NULL, msg, L"Confirm Delete", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        actionData = malloc(sizeof(struct ActionData));
        
        actionData->srcPaths = createPathsFromFileNodes(nodes, count);
        actionData->numSrcPaths = count;
        actionData->action = ACTION_DELETE;
        actionData->cancel = false;
        
        hwndDlg = CreateDialogParam(globalHInstance, MAKEINTRESOURCE(IDD_FILE_ACTION), hwndMain, &FileActionDialogProc, 0);     
        SetTimer(hwndDlg, ID_EVENT_PRELOADER, PRELOADER_PERIOD, NULL);
        CreateThread(NULL, 0, fileActionTask, actionData, 0, NULL);
        ShowWindow(hwndDlg, SW_SHOW);
    }
}

void copyFiles(struct FileNode** nodes, int count) {
    clearClipboard();
    clipboard = createPathsFromFileNodes(nodes, count);
    clipboardSize = count;
    clipboardIsCut = false;
}

void cutFiles(struct FileNode** nodes, int count) {
    clearClipboard();
    clipboard = createPathsFromFileNodes(nodes, count);
    clipboardSize = count;  
    clipboardIsCut = true;
}

void pasteFiles(wchar_t* dstDir) {
    if (clipboardSize == 0) return; 
    
    actionData = malloc(sizeof(struct ActionData));
    
    int len = wcslen(dstDir);
    actionData->dstPath = malloc((len + 2) * sizeof(wchar_t));
    wcscpy_s(actionData->dstPath, len + 1, dstDir);
    actionData->dstPath[len+0] = '\0';
    actionData->dstPath[len+1] = '\0';
    
    actionData->action = clipboardIsCut ? ACTION_MOVE : ACTION_COPY;
    actionData->srcPaths = clipboard;
    actionData->numSrcPaths = clipboardSize;
    actionData->cancel = false;
    
    hwndDlg = CreateDialogParam(globalHInstance, MAKEINTRESOURCE(IDD_FILE_ACTION), hwndMain, &FileActionDialogProc, 0);     
    SetTimer(hwndDlg, ID_EVENT_PRELOADER, PRELOADER_PERIOD, NULL);
    CreateThread(NULL, 0, fileActionTask, actionData, 0, NULL);
    ShowWindow(hwndDlg, SW_SHOW);   
}

static void createShortcut(wchar_t* srcPath, wchar_t* dstPath) {
    HRESULT hres;

    IShellLinkW *isl;
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (LPVOID*)&isl);
    if (SUCCEEDED(hres)) {
        wchar_t workingDir[MAX_PATH];
        getParentDirFromPath(srcPath, workingDir);
        
        IShellLinkW_SetPath(isl, srcPath);
        IShellLinkW_SetWorkingDirectory(isl, workingDir);
        IShellLinkW_SetDescription(isl, L"");
    
        IPersistFile *ipf;
        hres = IShellLinkW_QueryInterface(isl, &IID_IPersistFile, (void **)&ipf);

        if (SUCCEEDED(hres)) {
            IPersistFile_Save(ipf, dstPath, TRUE);
            IPersistFile_Release(ipf);
        }
        
        IShellLinkW_Release(isl);
    }
}

void pasteShortcuts(wchar_t* dstDir) {
    if (clipboardSize == 0) return;
    
    wchar_t dstPath[MAX_PATH];
    wchar_t basename[80];
    
    for (int i = 0; i < clipboardSize; i++) {
        wchar_t* srcPath = clipboard[i];
        if (wcscmp(srcPath, L".lnk") != 0) {
            getBasenameFromPath(srcPath, basename, true);
            swprintf_s(dstPath, MAX_PATH, L"%ls\\%ls.lnk", dstDir, basename);
            createShortcut(srcPath, dstPath);           
        }
    }
    
    if (clipboardIsCut) clearClipboard();
    navigateRefresh();
}

void createDesktopShortcuts(struct FileNode** nodes, int count) {
    wchar_t** srcPaths = createPathsFromFileNodes(nodes, count);
    wchar_t* desktopPath = getDesktopPath();
    wchar_t dstPath[MAX_PATH];
    wchar_t basename[80];
    
    for (int i = 0; i < count; i++) {
        wchar_t* srcPath = srcPaths[i];
        if (wcscmp(srcPath, L".lnk") != 0) {
            getBasenameFromPath(srcPath, basename, true);
            swprintf_s(dstPath, MAX_PATH, L"%ls\\%ls.lnk", desktopPath, basename);
            createShortcut(srcPath, dstPath);           
        }
        free(srcPaths[i]);
    }
    
    free(srcPaths); 
}