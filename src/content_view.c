#include "main.h"

#define COLUMN_NAME_IDX 0
#define COLUMN_TYPE_IDX 1
#define COLUMN_SIZE_IDX 2
#define COLUMN_DATE_IDX 3
#define COLUMN_PATH_IDX 4

enum Msg {
    MSG_ADD_ITEM = WM_APP,
    MSG_SEARCH_DONE
};

enum ContextMenuType {
    MENU_SINGLE,
    MENU_MULTIPLE,
    MENU_EMPTY
};

struct ListItem {
    int icon;
    struct FileNode* node;
    wchar_t type[64];
    wchar_t formattedSize[32];
    wchar_t formattedDate[32];
    bool loaded;
    uint64_t size;
    wchar_t* path;
    FILETIME modifiedTime;
};

struct SearchData {
    wchar_t* keyword;
    bool active;
    bool canceled;
};

struct ContextMenuItem {
    wchar_t* text;
    void(*proc)();
    wchar_t* cmdData;
};

static void onMenuItemMountISOImageClick();
static void onMenuItemUnmountISOImageClick();

static struct ContextMenuItem cmiOpen = {STR_OPEN, &onMenuItemOpenClick, NULL};
static struct ContextMenuItem cmiEdit = {STR_EDIT, &onMenuItemEditClick, NULL};
static struct ContextMenuItem cmiCut = {STR_CUT, &onMenuItemCutClick, NULL};
static struct ContextMenuItem cmiCopy = {STR_COPY, &onMenuItemCopyClick, NULL};
static struct ContextMenuItem cmiCreateShortcut = {STR_CREATE_SHORTCUT, &onMenuItemCreateShortcutClick, NULL};
static struct ContextMenuItem cmiDelete = {STR_DELETE, &onMenuItemDeleteClick, NULL};
static struct ContextMenuItem cmiRename = {STR_RENAME, &onMenuItemRenameClick, NULL};
static struct ContextMenuItem cmiPaste = {STR_PASTE, &onMenuItemPasteClick, NULL};
static struct ContextMenuItem cmiPasteShortcut = {STR_PASTE_SHORTCUT, &onMenuItemPasteShortcutClick, NULL};
static struct ContextMenuItem cmiNewFolder = {STR_NEW_FOLDER, &onMenuItemNewFolderClick, NULL};
static struct ContextMenuItem cmiNewFile = {STR_NEW_FILE, &onMenuItemNewFileClick, NULL};
static struct ContextMenuItem cmiMountISOImage = {NULL, &onMenuItemMountISOImageClick, NULL};
static struct ContextMenuItem cmiUnmountISOImage = {STR_UNMOUNT_IMAGE, &onMenuItemUnmountISOImageClick, NULL};

static WNDPROC OrigWndProc;
static struct ListItem* items = NULL;
static int numItems = 0;
static enum ViewStyle viewStyle = STYLE_DETAILS;
static HMENU hContextMenu;
static char sortColumnIdx = COLUMN_NAME_IDX;
static bool sortAscending = true;

static struct FileNode** selectedItems = NULL;
static int numSelectedItems = 0;

static struct ContextMenuItem* menuItems = NULL;
static int numMenuItems = 0;

static struct SearchData* searchData;

extern struct FileNode* currPathFileNode;
extern HINSTANCE globalHInstance;
extern HWND hwndMain;

HWND hwndContentView = NULL;

static void fillFileInfo(struct FileNode* node, struct ListItem* item) {
    item->size = 0;
    memset(&item->modifiedTime, 0, sizeof(FILETIME));
    
    if (node->type == TYPE_FILE) {
        if (node->metadata) {
            item->size = node->metadata->size;
            timetToFileTime(node->metadata->modifiedTime, &item->modifiedTime);
        }
        else {
            LARGE_INTEGER filesize;
            WIN32_FILE_ATTRIBUTE_DATA info = {0};

            wchar_t path[MAX_PATH] = {0};
            getFileNodePath(node, path);
            GetFileAttributesEx(path, GetFileExInfoStandard, &info);        
            
            if ((info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {
                filesize.LowPart = info.nFileSizeLow;
                filesize.HighPart = info.nFileSizeHigh;
                item->size = filesize.QuadPart;
            }
            
            memcpy(&item->modifiedTime, &info.ftLastWriteTime, sizeof(FILETIME));
        }        
    }
}

static void updateStatusbar() {
    wchar_t statusText[32] = {0};
    swprintf_s(statusText, 32, L"%d " STR_ITEMS, numItems);
    setStatusbarText(statusText);   
}

static void freeMenuItems() {
    if (menuItems) {
        for (int i = 0; i < numMenuItems; i++) {
            if (menuItems[i].cmdData) {
                free(menuItems[i].cmdData);
                menuItems[i].cmdData = NULL;
            }
        }
        free(menuItems);
        menuItems = NULL;        
    }
    numMenuItems = 0;    
}

void clearContentView() {    
    ListView_SetItemCountEx(hwndContentView, 0, 0);
    ListView_DeleteColumn(hwndContentView, COLUMN_PATH_IDX);    
    
    if (items) {
        for (int i = 0; i < numItems; i++) {
            if (items[i].path) {
                free(items[i].path);
                items[i].path = NULL;
            }
        }
        free(items);
        items = NULL;
    }
    numItems = 0;
    
    freeMenuItems();
}

static void execCommandLine(wchar_t *command) {
    SHELLEXECUTEINFO shExecInfo = {0};
    shExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    shExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shExecInfo.hwnd = hwndMain;
    shExecInfo.lpVerb = NULL;
    shExecInfo.lpFile = L"C:\\windows\\system32\\cmd.exe";        
    shExecInfo.lpParameters = command;   
    shExecInfo.lpDirectory = NULL;
    shExecInfo.nShow = SW_SHOW;
    shExecInfo.hInstApp = NULL; 
    ShellExecuteEx(&shExecInfo);
    WaitForSingleObject(shExecInfo.hProcess, INFINITE);
    CloseHandle(shExecInfo.hProcess);    
}

LRESULT CALLBACK ContentViewWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND: {
            if ((HWND)lParam == 0) {
                MENUITEMINFO item;
                item.cbSize = sizeof(MENUITEMINFO);
                item.fMask = MIIM_DATA;
                GetMenuItemInfo(hContextMenu, LOWORD(wParam), FALSE, &item);
                struct ContextMenuItem* cmItem = (struct ContextMenuItem*)item.dwItemData;
                
                if (cmItem->cmdData) {
                    wchar_t command[MAX_PATH];
                    wcscat_s(command, MAX_PATH, L"/C ");
                    wcscat_s(command, MAX_PATH, cmItem->cmdData);
                    execCommandLine(command);
                    navigateRefresh();
                }
                else cmItem->proc();
            }           
            break;
        }
        case MSG_ADD_ITEM: {
            if (searchData != NULL && searchData->active) {
                struct FileNode* node = (struct FileNode*)lParam;
                int index = numItems++;
                items = realloc(items, numItems * sizeof(struct ListItem));     
                struct ListItem* item = &items[index];
                item->node = node;
                item->path = NULL;
                item->loaded = false;
                
                fillFileInfo(node, item);
                
                ListView_SetItemCountEx(hwndContentView, numItems, LVSICF_NOINVALIDATEALL);
                updateStatusbar();
            }
            break;
        }
        case MSG_SEARCH_DONE: {
            searchData->active = false;
            bool canceled = searchData->canceled;
            free(searchData);
            searchData = NULL;
            if (canceled) {
                refreshContentView();
            }
            else updateStatusbar();
            break;
        }
    }
    return OrigWndProc(hwnd, msg, wParam, lParam);  
}

void updateSelectedItems() {
    MEMFREE(selectedItems);
    numSelectedItems = 0;

    int i = ListView_GetNextItem(hwndContentView, -1, LVNI_SELECTED);
    while (i != -1) {       
        int index = numSelectedItems++;
        selectedItems = realloc(selectedItems, numSelectedItems * sizeof(struct FileNode*));
        selectedItems[index] = items[i].node;
        i = ListView_GetNextItem(hwndContentView, i, LVNI_SELECTED);
    }
}

static void addContextMenuItem(HMENU hMenu, int id, struct ContextMenuItem* cmItem, bool separate) {
    MENUITEMINFO item = {0};
    item.cbSize = sizeof(MENUITEMINFO);
    item.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
    item.fType = MFT_STRING;
    item.dwTypeData = cmItem->text;
    item.cch = wcslen(cmItem->text);
    item.wID = id;
    item.dwItemData = (ULONG_PTR)cmItem;

    InsertMenuItem(hMenu, -1, TRUE, &item);

    if (separate) {
        item.fMask = MIIM_TYPE;
        item.fType = MFT_SEPARATOR;
        InsertMenuItem(hMenu, -1, TRUE, &item);
    }
}

static void createContextMenuFromRegistry(int* id) {
    freeMenuItems();
    HKEY hkeyContextMenu, hkeyItem;
    if (RegOpenKey(HKEY_CURRENT_USER, L"SOFTWARE\\Winlator\\WFM\\ContextMenu", &hkeyContextMenu) != ERROR_SUCCESS) return;
    
    WCHAR itemName[30] = {0};
    WCHAR subitemName[100] = {0};
    WCHAR itemValue[MAX_PATH];
    DWORD i, j, itemNameLen, itemValueLen;
    
    i = 0;
    while (i < 10) {
        itemNameLen = 30;    
        if (RegEnumKey(hkeyContextMenu, i++, itemName, itemNameLen) != ERROR_SUCCESS) break;
        if (RegOpenKey(hkeyContextMenu, itemName, &hkeyItem) == ERROR_SUCCESS) {
            MENUITEMINFO item = {0};
            item.cbSize = sizeof(MENUITEMINFO);
            item.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
            item.fType = MFT_STRING;
            item.dwTypeData = itemName;
            item.cch = itemNameLen;
            item.wID = ++(*id);
            
            HMENU hSubmenu = CreatePopupMenu();
            
            j = 0;
            while (j < 10) {
                itemNameLen = 100;
                itemValueLen = MAX_PATH;
                if (RegEnumValue(hkeyItem, j++, subitemName, &itemNameLen, NULL, NULL, (LPBYTE)itemValue, &itemValueLen) != ERROR_SUCCESS) break;
                
                int index = numMenuItems++;
                menuItems = realloc(menuItems, numMenuItems * sizeof(struct ContextMenuItem));     
                
                struct ContextMenuItem* cmItem = &menuItems[index];
                cmItem->text = subitemName;
                cmItem->proc = NULL;
                
                wchar_t *cmdData = malloc(1024);
                wcscpy_s(cmdData, MAX_PATH, itemValue);
                
                wchar_t path[MAX_PATH] = {0};
                getFileNodePath(selectedItems[0], path);
                cmdData = strReplace(cmdData, L"%FILE%", path, true);
                
                wchar_t basename[80] = {0};
                getBasenameFromPath(path, basename, true);
                cmdData = strReplace(cmdData, L"%BASENAME%", basename, true);                
                
                getFileNodePath(selectedItems[0]->parent, path);
                cmdData = strReplace(cmdData, L"%DIR%", path, true);
                
                cmItem->cmdData = cmdData;
                addContextMenuItem(hSubmenu, (*id)++, cmItem, false);
            }
            
            item.hSubMenu = hSubmenu;
            
            InsertMenuItem(hContextMenu, -1, TRUE, &item);
            
            item.fMask = MIIM_TYPE;
            item.fType = MFT_SEPARATOR;
            InsertMenuItem(hContextMenu, -1, TRUE, &item);
            
            RegCloseKey(hkeyItem);
        }
    }
    
    RegCloseKey(hkeyContextMenu);
}

static void createISOImageContextMenu(int* id) {
    wchar_t* itemText = STR_ISO_IMAGE;
    MENUITEMINFO item = {0};
    item.cbSize = sizeof(MENUITEMINFO);
    item.fMask = MIIM_TYPE | MIIM_ID | MIIM_SUBMENU;
    item.fType = MFT_STRING;
    item.dwTypeData = itemText;
    item.cch = wcslen(itemText);
    item.wID = ++(*id);
    
    HMENU hSubmenu = CreatePopupMenu();
    
    wchar_t currentISOPath[MAX_PATH] = {0};
    int currentISOPathLen = MAX_PATH;
    HKEY hkey;
    if (RegOpenKey(HKEY_CURRENT_USER, L"SOFTWARE\\Winlator\\WFM\\CurrentISOPath", &hkey) == ERROR_SUCCESS) {
        RegQueryValue(hkey, NULL, currentISOPath, (PLONG)&currentISOPathLen);
        RegCloseKey(hkey);
    }
    
    itemText = calloc(MAX_PATH, sizeof(wchar_t));
    swprintf_s(itemText, MAX_PATH, STR_MOUNT_IMAGE L" <%ls>", currentISOPathLen != MAX_PATH ? currentISOPath : STR_NO_MEDIA);
    cmiMountISOImage.text = itemText;
    addContextMenuItem(hSubmenu, (*id)++, &cmiMountISOImage, false);
    MEMFREE(cmiMountISOImage.text);
    
    addContextMenuItem(hSubmenu, (*id)++, &cmiUnmountISOImage, false);    
    
    item.hSubMenu = hSubmenu;
    InsertMenuItem(hContextMenu, -1, TRUE, &item);    
    
    item.fMask = MIIM_TYPE;
    item.fType = MFT_SEPARATOR;
    InsertMenuItem(hContextMenu, -1, TRUE, &item);    
}

static void createContextMenu(enum ContextMenuType type) {
    HMENU hMenu = CreatePopupMenu();
    hContextMenu = hMenu;

    int id = 0;

    if (type == MENU_SINGLE || type == MENU_MULTIPLE) {
        if (type == MENU_SINGLE) {
            if (selectedItems[0]->type == TYPE_FILE) {
                addContextMenuItem(hMenu, id++, &cmiOpen, false);
                addContextMenuItem(hMenu, id++, &cmiEdit, true);
                createISOImageContextMenu(&id);
                createContextMenuFromRegistry(&id);
            }
            else addContextMenuItem(hMenu, id++, &cmiOpen, true);
        }
        addContextMenuItem(hMenu, id++, &cmiCut, false);
        addContextMenuItem(hMenu, id++, &cmiCopy, true);
        addContextMenuItem(hMenu, id++, &cmiCreateShortcut, false);
        addContextMenuItem(hMenu, id++, &cmiDelete, false);
        
        if (type == MENU_SINGLE) addContextMenuItem(hMenu, id++, &cmiRename, false);
    }
    else {
        addContextMenuItem(hMenu, id++, &cmiPaste, false);
        addContextMenuItem(hMenu, id++, &cmiPasteShortcut, true);
        createISOImageContextMenu(&id);
        addContextMenuItem(hMenu, id++, &cmiNewFolder, false);
        addContextMenuItem(hMenu, id++, &cmiNewFile, false);
    }

    POINT cursor;
    GetCursorPos(&cursor);
    TrackPopupMenu(hMenu, 0, cursor.x, cursor.y, 0, hwndContentView, NULL);
}

LRESULT contentViewNotify(NMHDR* nmhdr) {
    switch (nmhdr->code) {
        case LVN_GETDISPINFO: {
            NMLVDISPINFO* nmlvdi = (NMLVDISPINFO*)nmhdr;
            UINT mask = nmlvdi->item.mask;
            struct ListItem* item = &items[nmlvdi->item.iItem];
            
            if (!item->loaded) {
                wchar_t path[MAX_PATH] = {0};
                getFileNodePath(item->node, path);
                
                struct FileInfo fi = {0};
                getFileInfo(path, item->node->type, viewStyle == STYLE_LARGE_ICON, &fi);

                if (item->node->type == TYPE_FILE) {
                    formatFileSize(item->size, item->formattedSize);
                    
                    SYSTEMTIME systemTime = {0};
                    FILETIME localFiletime;
                    if (FileTimeToLocalFileTime(&item->modifiedTime, &localFiletime) && FileTimeToSystemTime(&localFiletime, &systemTime)) {
                        formatModifiedDate(systemTime.wMonth, systemTime.wDay, systemTime.wYear, systemTime.wHour, systemTime.wMinute, item->formattedDate, 32);
                    }
                }

                item->icon = fi.icon;
                wcscpy_s(item->type, 80, fi.typeName);
                item->loaded = true;                
            }
            
            if (mask & LVIF_STATE) {
                nmlvdi->item.state = 0;
            }

            if (mask & LVIF_IMAGE) {
                nmlvdi->item.iImage = item->icon;
            }           
            
            if (mask & LVIF_TEXT) {
                switch (nmlvdi->item.iSubItem) {
                    case COLUMN_NAME_IDX:
                        nmlvdi->item.pszText = item->node->name;
                        break;
                    case COLUMN_TYPE_IDX:
                        nmlvdi->item.pszText = item->type;
                        break;
                    case COLUMN_SIZE_IDX:
                        nmlvdi->item.pszText = item->node->type == TYPE_FILE ? item->formattedSize : L"";
                        break;
                    case COLUMN_DATE_IDX:
                        nmlvdi->item.pszText = item->node->type == TYPE_FILE ? item->formattedDate : L"";
                        break;
                    case COLUMN_PATH_IDX: {
                        if (!item->path) {
                            wchar_t path[MAX_PATH] = {0};
                            getFileNodePath(item->node, path);
                            item->path = wcsdup(path);
                        }
                        nmlvdi->item.pszText = item->path;
                        break;
                    }                       
                }
            }           
            break;
        }
        case NM_RCLICK: {
            NMITEMACTIVATE* nmia = (NMITEMACTIVATE*)nmhdr;

            if (nmia->iItem != -1 && nmia->iSubItem == 0) {
                updateSelectedItems();
                
                bool show = true;
                for (int i = 0; i < numSelectedItems; i++) {
                    if (!(selectedItems[i]->type == TYPE_FILE || selectedItems[i]->type == TYPE_DIR)) {
                        show = false;
                        break;
                    }
                }
                if (show) createContextMenu(numSelectedItems == 1 ? MENU_SINGLE : MENU_MULTIPLE);
            }
            else createContextMenu(MENU_EMPTY);         
            break;
        }
        case NM_DBLCLK: {
            NMITEMACTIVATE* nmia = (NMITEMACTIVATE*)nmhdr;
            if (nmia->iItem == -1 || nmia->iSubItem != 0) break;

            struct ListItem* item = &items[nmia->iItem];            
            openFileNode(item->node);
            break;
        }
        case LVN_COLUMNCLICK: {
            LPNMLISTVIEW plvInfo = (LPNMLISTVIEW)nmhdr;

            if (plvInfo->iSubItem == sortColumnIdx) {
                sortAscending = !sortAscending;
            }
            else {
                sortColumnIdx = plvInfo->iSubItem;
                sortAscending = true;
            }

            refreshContentView();
            break;
        }       
    }

    return 0;   
}

static DWORD WINAPI searchTask(void* param) {
    struct SearchData* searchData = (struct SearchData*)param;
    
    const int maxStackSize = 50;
    struct FileNode* stack[maxStackSize];
    int stackSize = 0;
    stack[stackSize++] = currPathFileNode->children;
    
    wchar_t keyword[64] = {0};
    wchar_t name[64] = {0};
    
    strToLower(searchData->keyword, keyword);
    
    while (stackSize > 0 && numItems < 10000 && searchData->active) {
        struct FileNode* node = stack[--stackSize];
        while (node && searchData->active) {
            strToLower(node->name, name);
            if (wcsstr(name, keyword)) {
                SendMessage(hwndContentView, MSG_ADD_ITEM, 0, (LPARAM)node);
            }
            
            if (numItems >= 10000) break;
            
            if (node->type == TYPE_DIR && stackSize < maxStackSize) {
                buildChildNodes(node, false);
                if (node->children) stack[stackSize++] = node->children;
            }
            node = node->sibling;       
        }
    }
    
    SendMessage(hwndContentView, MSG_SEARCH_DONE, 0, 0);
    return 0;
}

void searchFor(wchar_t* keyword) {
    if (wcslen(keyword) == 0) return;
    if (searchData != NULL && searchData->active) {
        searchData->active = false;
        return;
    }
    
    clearContentView();
    
    LVCOLUMN column = {0};
    column.mask = LVCF_WIDTH | LVCF_TEXT;
    column.cx = 250;
    column.pszText = STR_PATH;
    ListView_InsertColumn(hwndContentView, COLUMN_PATH_IDX, &column);
    UpdateWindow(hwndContentView);
    
    searchData = malloc(sizeof(struct SearchData));
    searchData->keyword = keyword;
    searchData->active = true;
    searchData->canceled = false;

    CreateThread(NULL, 0, searchTask, searchData, 0, NULL);
}

void setViewStyle(enum ViewStyle newViewStyle) {
    LONG_PTR wndstyle = GetWindowLongPtr(hwndContentView, GWL_STYLE);
    wndstyle &= ~LVS_TYPEMASK;

    switch (newViewStyle) {
        case STYLE_LARGE_ICON:
            wndstyle |= LVS_ICON;
            break;
        case STYLE_SMALL_ICON:
            wndstyle |= LVS_SMALLICON;
            break;
        case STYLE_LIST:
            wndstyle |= LVS_LIST;
            break;
        case STYLE_DETAILS:
            wndstyle |= LVS_REPORT;
            break;
    }

    SetWindowLongPtr(hwndContentView, GWL_STYLE, wndstyle);

    viewStyle = newViewStyle;
    refreshContentView();
}

void createLVColumns() {
    LVCOLUMN column = {0};
    column.mask = LVCF_WIDTH | LVCF_TEXT;

    column.cx = 220;
    column.pszText = STR_NAME;
    ListView_InsertColumn(hwndContentView, COLUMN_NAME_IDX, &column);

    column.cx = 100;
    column.pszText = STR_TYPE;
    ListView_InsertColumn(hwndContentView, COLUMN_TYPE_IDX, &column);

    column.cx = 60;
    column.pszText = STR_SIZE;
    ListView_InsertColumn(hwndContentView, COLUMN_SIZE_IDX, &column);

    column.cx = 100;
    column.pszText = STR_DATE;
    ListView_InsertColumn(hwndContentView, COLUMN_DATE_IDX, &column);
}

void createContentView() {
    hwndContentView = CreateWindowEx(0, WC_LISTVIEW, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER | LVS_OWNERDATA | LVS_REPORT | LVS_SHAREIMAGELISTS,
                      0, 0, 0, 0, hwndMain, (HMENU)NULL, globalHInstance, NULL);
    
    OrigWndProc = (WNDPROC)SetWindowLongPtr(hwndContentView, GWLP_WNDPROC, (LONG_PTR)ContentViewWndProc);
    createLVColumns();
    UpdateWindow(hwndContentView);
}

void onMenuItemUpClick() {
    navigateUp();
}

void onMenuItemOpenClick() {
    if (numSelectedItems == 1) openFileNode(selectedItems[0]);
}

void onMenuItemEditClick() {
    if (numSelectedItems == 1 && selectedItems[0]->type == TYPE_FILE) {
        static const wchar_t editorPath[] = L"C:\\windows\\notepad.exe";
        
        wchar_t path[MAX_PATH] = {0};
        wchar_t parameters[MAX_PATH] = {0};
        getFileNodePath(selectedItems[0], path);
        swprintf_s(parameters, MAX_PATH, L"\"%ls\"", path);
        getFileNodePath(selectedItems[0]->parent, path);
        ShellExecute(hwndMain, L"open", editorPath, parameters, path, SW_SHOW);     
    }   
}

void onMenuItemCutClick() {
    updateSelectedItems();
    if (numSelectedItems > 0) cutFiles(selectedItems, numSelectedItems);    
}

void onMenuItemCopyClick() {
    updateSelectedItems();
    if (numSelectedItems > 0) copyFiles(selectedItems, numSelectedItems);
}

void onMenuItemCreateShortcutClick() {
    updateSelectedItems();
    if (numSelectedItems > 0) createDesktopShortcuts(selectedItems, numSelectedItems);  
}

void onMenuItemDeleteClick() {
    updateSelectedItems();
    if (numSelectedItems > 0) deleteFiles(selectedItems, numSelectedItems); 
}

void onMenuItemRenameClick() {
    if (numSelectedItems == 1) {
        wchar_t* result = InputDialog(STR_RENAME, STR_ENTER_NEW_NAME, selectedItems[0]->name, true);
        if (result) {
            wchar_t newFilename[MAX_PATH] = {0};
            getFileNodePath(selectedItems[0]->parent, newFilename);
            wcscat_s(newFilename, MAX_PATH, L"\\");
            wcscat_s(newFilename, MAX_PATH, result);
            free(result);
            
            wchar_t oldFilename[MAX_PATH] = {0};
            getFileNodePath(selectedItems[0], oldFilename);
            MoveFileW(oldFilename, newFilename);
            navigateRefresh();
        }
    }
}

void onMenuItemPasteClick() {
    wchar_t path[MAX_PATH] = {0};
    getFileNodePath(currPathFileNode, path);
    if (!isPathExists(path)) return;
    pasteFiles(path);
}

void onMenuItemPasteShortcutClick() {
    wchar_t path[MAX_PATH] = {0};
    getFileNodePath(currPathFileNode, path);
    if (!isPathExists(path)) return;
    pasteShortcuts(path);   
}

void onMenuItemNewFolderClick() {
    wchar_t path[MAX_PATH] = {0};
    getFileNodePath(currPathFileNode, path);
    if (!isPathExists(path)) return;
    
    wchar_t* result = InputDialog(STR_NEW_FOLDER, STR_ENTER_FOLDER_NAME, NULL, false);
    if (result) {
        wcscat_s(path, MAX_PATH, L"\\");
        wcscat_s(path, MAX_PATH, result);
        free(result);
        
        if (!isPathExists(path)) {
            CreateDirectory(path, NULL);
            navigateRefresh();          
        }
    }   
}

void onMenuItemNewFileClick() {
    wchar_t path[MAX_PATH] = {0};
    getFileNodePath(currPathFileNode, path);
    if (!isPathExists(path)) return;
    
    wchar_t* result = InputDialog(STR_NEW_FILE, STR_ENTER_FILE_NAME, NULL, false);
    if (result) {
        wcscat_s(path, MAX_PATH, L"\\");
        wcscat_s(path, MAX_PATH, result);
        free(result);
        
        if (!isPathExists(path)) {
            HANDLE handle = CreateFile(path, GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
            if (handle != INVALID_HANDLE_VALUE) CloseHandle(handle);
            navigateRefresh();  
        }
    }       
}

void onMenuItemSelectAllClick() {
    ListView_SetItemState(hwndContentView, -1, 0, LVIS_SELECTED);
    ListView_SetItemState(hwndContentView, -1, LVIS_SELECTED, LVIS_SELECTED);
    SetFocus(hwndContentView);
}

static void onMenuItemMountISOImageClick() {
    if (numSelectedItems != 1) {
        MessageBox(NULL, MSG_INVALID_ISO_IMAGE_FILE, STR_ALERT, MB_OK);
        return;
    }
    
    wchar_t currentISOPath[MAX_PATH] = {0};
    HKEY hkey;
    getFileNodePath(selectedItems[0], currentISOPath);
    
    if (!isPathExists(currentISOPath) || !hasFileExtension(currentISOPath, L"iso")) {
        MessageBox(NULL, MSG_INVALID_ISO_IMAGE_FILE, STR_ALERT, MB_OK);
        return;
    }
    
    if (RegCreateKey(HKEY_CURRENT_USER, L"SOFTWARE\\Winlator\\WFM\\CurrentISOPath", &hkey) == ERROR_SUCCESS) {
        RegSetValue(hkey, NULL, REG_SZ, currentISOPath, (wcslen(currentISOPath) + 1) * sizeof(wchar_t));
        RegCloseKey(hkey);
    }
    
    navigateRefresh();
}

static void onMenuItemUnmountISOImageClick() {
    RegDeleteKey(HKEY_CURRENT_USER, L"SOFTWARE\\Winlator\\WFM\\CurrentISOPath");
    navigateRefresh();
}

static int compareType(const void* a, const void* b) {
    struct ListItem* ia = (struct ListItem*)a;
    struct ListItem* ib = (struct ListItem*)b;
    return sortAscending ? ia->node->type - ib->node->type : ib->node->type - ia->node->type;
}

static int compareName(const void* a, const void* b) {
    struct ListItem* ia = (struct ListItem*)a;
    struct ListItem* ib = (struct ListItem*)b;
    int res = compareType(a, b);
    if (res == 0) res = sortAscending ? wcscoll(ia->node->name, ib->node->name) : wcscoll(ib->node->name, ia->node->name);
    return res;
}

static int compareSize(const void* a, const void* b) {
    struct ListItem* ia = (struct ListItem*)a;
    struct ListItem* ib = (struct ListItem*)b;
    int res = compareType(a, b);
    if (res == 0) res = sortAscending ? ia->size - ib->size : ib->size - ia->size;
    return res;
}

static int compareDate(const void* a, const void* b) {
    struct ListItem* ia = (struct ListItem*)a;
    struct ListItem* ib = (struct ListItem*)b;
    int res = compareType(a, b);
    if (res == 0) res = sortAscending ? CompareFileTime(&ia->modifiedTime, &ib->modifiedTime) : CompareFileTime(&ib->modifiedTime, &ia->modifiedTime);
    return res;
}

void sortItems() {
    switch (sortColumnIdx) {
        case COLUMN_NAME_IDX:
            qsort(items, numItems, sizeof(struct ListItem), compareName);
            break;
        case COLUMN_TYPE_IDX:
            qsort(items, numItems, sizeof(struct ListItem), compareType);
            break;
        case COLUMN_SIZE_IDX:
            qsort(items, numItems, sizeof(struct ListItem), compareSize);
            break;
        case COLUMN_DATE_IDX:
            qsort(items, numItems, sizeof(struct ListItem), compareDate);
            break;
    }
}

void refreshContentView() {
    if (searchData != NULL && searchData->active) {
        searchData->active = false;     
        searchData->canceled = true;
        return;
    }
    
    clearContentView();
    UpdateWindow(hwndContentView);
    
    struct FileNode* child = currPathFileNode->children;
    
    numItems = getChildNodeCount(currPathFileNode);
    items = calloc(numItems, sizeof(struct ListItem));
    int index = 0;
    
    while (child) {
        struct ListItem* item = &items[index++];
        item->node = child;
        item->loaded = false;

        fillFileInfo(child, item);
        
        child = child->sibling;
    }

    HIMAGELIST himlBig, himlSmall;
    Shell_GetImageLists(&himlBig, &himlSmall);
    
    if (viewStyle == STYLE_LARGE_ICON) {
        ListView_SetImageList(hwndContentView, himlBig, LVSIL_NORMAL);
    }
    else ListView_SetImageList(hwndContentView, himlSmall, LVSIL_SMALL);

    if (sortColumnIdx != -1) sortItems();
    ListView_SetItemCountEx(hwndContentView, numItems, 0);
    
    updateStatusbar();  
}