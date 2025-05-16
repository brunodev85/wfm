#include "main.h"

#define ID_EVENT_PRELOADER 100
#define PRELOADER_PERIOD 120
#define CEILING(x, y) ((x+(y-1))/y)

enum Msg {
    MSG_CLOSE = WM_APP,
    MSG_NAVIGATE_REFRESH
};

enum FileAction {
    ACTION_NONE,
    ACTION_DELETE,
    ACTION_COPY,
    ACTION_MOVE,
    ACTION_ISO_EXTRACT
};

struct ActionData {
    enum FileAction action;
    wchar_t** srcPaths;
    int numSrcPaths;
    wchar_t* dstPath;
    bool cancel;
    void* tag;
    void(*callback)();
};

static HWND hwndDlg;
static HICON preloaderIcons[8] = {0};
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
        MEMFREE(clipboard);
    }
    clipboardSize = 0;
}

static void freeActionData() {
    if (clipboardIsCut) clearClipboard();
    
    if (actionData) {
        if (actionData->action == ACTION_DELETE || actionData->action == ACTION_ISO_EXTRACT) {
            for (int i = 0; i < actionData->numSrcPaths; i++) {
                MEMFREE(actionData->srcPaths[i]);
            }
            
            actionData->numSrcPaths = 0;
            MEMFREE(actionData->srcPaths);
        }
        
        actionData->callback = NULL;
        MEMFREE(actionData->dstPath);
        MEMFREE(actionData->tag);
        MEMFREE(actionData);
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
                    SetWindowText(hwndDlg, STR_DELETING_FILES);
                    SetWindowText(hwndLabel, MSG_DELETING_FILES);                
                    break;
                }
                case ACTION_COPY: {
                    SetWindowText(hwndDlg, STR_COPYING_FILES);
                    SetWindowText(hwndLabel, MSG_COPYING_FILES);             
                    break;
                }
                case ACTION_MOVE: {
                    SetWindowText(hwndDlg, STR_MOVING_FILES);
                    SetWindowText(hwndLabel, MSG_MOVING_FILES);              
                    break;
                }
                case ACTION_ISO_EXTRACT: {
                    SetWindowText(hwndDlg, STR_PREPARING_FILES);
                    SetWindowText(hwndLabel, MSG_PREPARING_FILES);
                    break;
                }
                case ACTION_NONE:
                    return (INT_PTR)FALSE;
            }
            return (INT_PTR)TRUE;
        }
        case WM_TIMER: {
            if (wParam == ID_EVENT_PRELOADER) animatePreloader();
            break;
        }
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDCANCEL) {
                if (MessageBox(hwndDlg, MSG_CANCEL_FILE_OPERATION, STR_CANCEL, MB_YESNO | MB_ICONQUESTION) == IDYES) {
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

static bool writeOutISOFile(iso9660_t* iso, iso9660_stat_t* isoStat, wchar_t* dstPath) {
    char filename[MAX_PATH] = {0};
    WideCharToMultiByte(CP_ACP, 0, dstPath, -1, filename, MAX_PATH, NULL, NULL);
    
    FILE* outFile = fopen(filename, "wb");
    
    const uint32_t isoBlocks = CEILING(isoStat->total_size, ISO_BLOCKSIZE);
    for (int i = 0; i < isoBlocks; i++) {
        char buffer[ISO_BLOCKSIZE] = {0};
        const lsn_t lsn = isoStat->lsn + i;

        if (iso9660_iso_seek_read(iso, buffer, lsn, 1) != ISO_BLOCKSIZE) return false;

        fwrite(buffer, ISO_BLOCKSIZE, 1, outFile);
        if (ferror(outFile)) return false;
    }
    
    fflush(outFile);
    ftruncate(fileno(outFile), isoStat->total_size);
    fclose(outFile);
    return true;
}

static void extractAllISOFiles(iso9660_t* iso, char* srcPath, wchar_t* dstPath) {
    CdioISO9660FileList_t* isoFileList = iso9660_ifs_readdir(iso, srcPath);
    if (isoFileList) {
        CdioListNode_t* isoNode;
        char srcName[MAX_PATH] = {0};
        wchar_t dstName[MAX_PATH] = {0};
        char fullSrcPath[MAX_PATH] = {0};
        wchar_t fullDstPath[MAX_PATH] = {0};        
        
        _CDIO_LIST_FOREACH(isoNode, isoFileList) {
            iso9660_stat_t* isoStat = (iso9660_stat_t*)_cdio_list_node_data(isoNode);
            if (strcmp(isoStat->filename, ".") == 0 || strcmp(isoStat->filename, "..") == 0) continue;
            
            memset(srcName, 0, MAX_PATH);
            iso9660_name_translate(isoStat->filename, srcName);
            
            joinUnixPaths(srcPath, srcName, fullSrcPath);
            
            MultiByteToWideChar(CP_ACP, 0, srcName, -1, dstName, MAX_PATH);
            joinPaths(dstPath, dstName, fullDstPath);
            
            if (isoStat->type == _STAT_DIR) {
                CreateDirectory(fullDstPath, NULL);
                extractAllISOFiles(iso, fullSrcPath, fullDstPath);
            }
            else if (isoStat->type == _STAT_FILE) {
                writeOutISOFile(iso, isoStat, fullDstPath);
            }
        }

        iso9660_filelist_free(isoFileList);
    }    
}

static DWORD WINAPI fileActionTask(void* param) {
    struct ActionData* actionData = (struct ActionData*)param;
    
    if (actionData->action == ACTION_ISO_EXTRACT) {
        char filename[MAX_PATH] = {0};
        WideCharToMultiByte(CP_ACP, 0, actionData->srcPaths[0], -1, filename, MAX_PATH, NULL, NULL);
        iso9660_t* iso = iso9660_open(filename);
        
        char* localFile = actionData->tag;
        if (localFile) { 
            iso9660_stat_t* isoStat = iso9660_ifs_stat_translate(iso, localFile);
            writeOutISOFile(iso, isoStat, actionData->dstPath);
        }
        else extractAllISOFiles(iso, "/", actionData->dstPath);
        
        iso9660_close(iso);
    }
    else {
        DWORD lastTime = GetTickCount();
            
        for (int i = 0; i < actionData->numSrcPaths && !actionData->cancel; i++) {  
            if (actionData->action == ACTION_DELETE) {
                SHFILEOPSTRUCT sfo = {0};
                sfo.hwnd = hwndDlg;
                sfo.wFunc = FO_DELETE;
                sfo.fFlags = FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI;
                sfo.pTo = NULL;
                sfo.pFrom = actionData->srcPaths[i];
                
                int res = SHFileOperation(&sfo);
                if (res != 0) break;
            }
            else if (actionData->action == ACTION_COPY || actionData->action == ACTION_MOVE) {
                SHFILEOPSTRUCT sfo = {0};
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
    }
    
    if (actionData->callback) actionData->callback();
    SendMessage(hwndDlg, MSG_CLOSE, 0, 0);
    return 0;
}

static wchar_t** createPathsFromFileNodes(struct FileNode** nodes, int count) {
    wchar_t** paths = calloc(count, sizeof(wchar_t*));
    
    wchar_t tmp[MAX_PATH] = {0};
    for (int i = 0; i < count; i++) {
        getFileNodePath(nodes[i], tmp);
        int len = wcslen(tmp);
        wchar_t* path = calloc(len + 2, sizeof(wchar_t));
        wcscpy_s(path, len + 1, tmp);
        path[len+0] = L'\0';
        path[len+1] = L'\0';         
        paths[i] = path;
    }
    
    return paths;
}

void deleteFiles(struct FileNode** nodes, int count) {
    wchar_t msg[128] = {0};
    if (count == 1) {
        swprintf_s(msg, 128, MSG_CONFIRM_DELETE_ITEM, nodes[0]->name);
    }
    else swprintf_s(msg, 128, MSG_CONFIRM_DELETE_MULTIPLE_ITEMS, count);

    if (MessageBox(NULL, msg, STR_CONFIRM_DELETE, MB_YESNO | MB_ICONQUESTION) == IDYES) {
        actionData = calloc(1, sizeof(struct ActionData));
        
        actionData->srcPaths = createPathsFromFileNodes(nodes, count);
        actionData->numSrcPaths = count;
        actionData->action = ACTION_DELETE;
        
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
    
    actionData = calloc(1, sizeof(struct ActionData));
    
    int len = wcslen(dstDir);
    actionData->dstPath = calloc(len + 2, sizeof(wchar_t));
    wcscpy_s(actionData->dstPath, len + 1, dstDir);
    actionData->dstPath[len+0] = L'\0';
    actionData->dstPath[len+1] = L'\0';
    
    actionData->action = clipboardIsCut ? ACTION_MOVE : ACTION_COPY;
    actionData->srcPaths = clipboard;
    actionData->numSrcPaths = clipboardSize;
    
    hwndDlg = CreateDialogParam(globalHInstance, MAKEINTRESOURCE(IDD_FILE_ACTION), hwndMain, &FileActionDialogProc, 0);     
    SetTimer(hwndDlg, ID_EVENT_PRELOADER, PRELOADER_PERIOD, NULL);
    CreateThread(NULL, 0, fileActionTask, actionData, 0, NULL);
    ShowWindow(hwndDlg, SW_SHOW);   
}

static void createShortcut(wchar_t* srcPath, wchar_t* dstPath) {
    HRESULT hres;

    IShellLinkW* isl;
    hres = CoCreateInstance(&CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, &IID_IShellLinkW, (LPVOID*)&isl);
    if (SUCCEEDED(hres)) {
        wchar_t workingDir[MAX_PATH] = {0};
        getParentDirFromPath(srcPath, workingDir);
        
        IShellLinkW_SetPath(isl, srcPath);
        IShellLinkW_SetWorkingDirectory(isl, workingDir);
        IShellLinkW_SetDescription(isl, L"");
    
        IPersistFile* ipf;
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
    
    wchar_t dstPath[MAX_PATH] = {0};
    wchar_t basename[80] = {0};
    
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
    wchar_t dstPath[MAX_PATH] = {0};
    wchar_t basename[80] = {0};
    
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

void extractFilesFromISOImage(wchar_t* isoPath, char* localFile, wchar_t* dstPath, void* callback) {
    actionData = calloc(1, sizeof(struct ActionData));
    
    actionData->srcPaths = calloc(1, sizeof(wchar_t*));
    actionData->srcPaths[0] = wcsdup(isoPath);

    actionData->dstPath = wcsdup(dstPath);
    actionData->action = ACTION_ISO_EXTRACT;
    actionData->tag = localFile ? strdup(localFile) : NULL;
    actionData->callback = callback;
    
    hwndDlg = CreateDialogParam(globalHInstance, MAKEINTRESOURCE(IDD_FILE_ACTION), hwndMain, &FileActionDialogProc, 0);     
    SetTimer(hwndDlg, ID_EVENT_PRELOADER, PRELOADER_PERIOD, NULL);
    CreateThread(NULL, 0, fileActionTask, actionData, 0, NULL);
    ShowWindow(hwndDlg, SW_SHOW);
}