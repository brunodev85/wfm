#include <cdio/cdio.h>
#include <cdio/iso9660.h>

#include "main.h"

static const wchar_t mainWndClass[] = L"WFM-MainWnd";

extern struct FileNode* currPathFileNode;
extern HWND hwndContentView;
extern HWND hwndNavbar;
extern HWND hwndSizebar;
extern HWND hwndStatusbar;
extern HWND hwndToolbar;
extern HWND hwndTreeview;

HINSTANCE globalHInstance = NULL;
HWND hwndMain = NULL;

static wchar_t* targetPath = NULL;

void GetWindowRectInParent(HWND hwnd, RECT* rect) {
    GetWindowRect(hwnd, rect);
    MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)rect, 2);
}

INT_PTR CALLBACK AboutDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {      
        case WM_COMMAND: {
            switch (LOWORD(wParam)) {
                case IDOK:
                case IDCANCEL: {
                  EndDialog(hwndDlg, (INT_PTR) LOWORD(wParam));
                  return (INT_PTR) TRUE;
                }
            }
            break;
        }
        case WM_INITDIALOG: {
            RECT rect, rect1;
            GetWindowRect(GetParent(hwndDlg), &rect);
            GetClientRect(hwndDlg, &rect1);
            SetWindowPos(hwndDlg, NULL, (rect.right + rect.left) / 2 - (rect1.right - rect1.left) / 2, (rect.bottom + rect.top) / 2 - (rect1.bottom - rect1.top) / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            return (INT_PTR)TRUE;
        }
    }

    return (INT_PTR)FALSE;
}

void mainMenuCommand(WPARAM wParam) {
    switch (LOWORD(wParam)) {
        case ID_EDIT_CUT:
            onMenuItemCutClick();
            break;
        case ID_EDIT_COPY:
            onMenuItemCopyClick();
            break;
        case ID_EDIT_PASTE:
            onMenuItemPasteClick();
            break;
        case ID_EDIT_PASTE_SHORTCUT:
            onMenuItemPasteShortcutClick();
            break;
        case ID_EDIT_SELECT_ALL:
            onMenuItemSelectAllClick();
            break;                  
        case ID_HELP_ABOUT:
            DialogBox(globalHInstance, MAKEINTRESOURCE(IDD_ABOUT), hwndMain, &AboutDialogProc);
            break;
        case ID_FILE_EXIT:
            DestroyWindow(hwndMain);
            break;
        case ID_VIEW_LARGEICONS:
            setViewStyle(STYLE_LARGE_ICON);
            break;
        case ID_VIEW_SMALLICONS:
            setViewStyle(STYLE_SMALL_ICON);
            break;
        case ID_VIEW_LIST:
            setViewStyle(STYLE_LIST);
            break;
        case ID_VIEW_DETAILS:
            setViewStyle(STYLE_DETAILS);
            break;                      
    }   
}

void resizeControls() {
    RECT rect;
    GetClientRect(hwndMain, &rect);

    RECT toolbarRect;
    RECT buttonRect;
    SendMessage(hwndToolbar, TB_GETITEMRECT, 0, (LPARAM)&buttonRect);
    SetWindowPos(hwndToolbar, NULL, 0, 0, rect.right, buttonRect.bottom + 3, SWP_NOZORDER);
    GetWindowRectInParent(hwndToolbar, &toolbarRect);

    RECT statusbarRect;
    SendMessage(hwndStatusbar, WM_SIZE, 0, 0);
    GetWindowRectInParent(hwndStatusbar, &statusbarRect);

    RECT navbarRect;
    int navbarHeight = getNavbarHeight();
    SetWindowPos(hwndNavbar, NULL, 0, toolbarRect.bottom, rect.right, navbarHeight, SWP_NOZORDER);
    GetWindowRectInParent(hwndNavbar, &navbarRect);
    
    RECT treeviewRect;
    GetWindowRectInParent(hwndTreeview, &treeviewRect);
    int treeviewHeight = statusbarRect.top - navbarRect.bottom;
    SetWindowPos(hwndTreeview, NULL, 0, navbarRect.bottom, treeviewRect.right, treeviewHeight, SWP_NOZORDER);
    
    const int sizebarWidth = 5;
    SetWindowPos(hwndSizebar, NULL, treeviewRect.right, navbarRect.bottom, sizebarWidth, treeviewHeight, SWP_NOZORDER);
    
    int contentViewX = treeviewRect.right + sizebarWidth;
    SetWindowPos(hwndContentView, NULL, contentViewX, navbarRect.bottom, rect.right - contentViewX, treeviewHeight, SWP_NOZORDER);  
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) { 
        case WM_SIZE: {
            resizeControls();
            break;
        }   
        case WM_COMMAND: {
            if (lParam == 0 && HIWORD(wParam) == 0) {
                mainMenuCommand(wParam);
                return 0;
            }
            else if ((HWND)lParam == hwndToolbar) {
                toolbarCommand(LOWORD(wParam));         
            }
            break;
        }
        case WM_SYSCOMMAND: {
            switch (LOWORD(wParam)) {
                case ID_HELP_ABOUT: {
                    DialogBox(globalHInstance, MAKEINTRESOURCE(IDD_ABOUT), hwnd, &AboutDialogProc);
                    return 0;
                }
            }
            break;
        }
        case WM_CLOSE: {
            wchar_t msg[] = MSG_CONFIRM_EXIT_APP;
            if (MessageBox(NULL, msg, STR_CONFIRM_EXIT, MB_YESNO | MB_ICONQUESTION) == IDYES) {
                PostQuitMessage(0);
            }
            return 0;
        }       
        case WM_DESTROY: {
            PostQuitMessage(0);
            return 0;
        }
        case WM_NOTIFY: {
            NMHDR* nmhdr = (NMHDR*)lParam;
            if (nmhdr->hwndFrom == hwndContentView) {
                return contentViewNotify(nmhdr);
            }
            else if (nmhdr->hwndFrom == hwndTreeview) {
                return treeviewNotify(nmhdr);
            }
            else return 0;
        }       
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

void navigateToFileNode(struct FileNode* node) {
    if (node) {
        clearAddrButtons();
        clearContentView();
        setCurrPathFileNode(node);
        navigateRefresh();
    }
}

void navigateToPath(wchar_t* path) {
    if (path) {
        clearAddrButtons();
        clearContentView();   
        setCurrPathFromString(path);
        navigateRefresh();
    }
}

void navigateUp() {
    if (currPathFileNode->parent) {
        clearAddrButtons();
        clearContentView();
        setCurrPathFileNode(currPathFileNode->parent);
        navigateRefresh();
    }
}

void navigateRefresh() {
    if (currPathFileNode) {
        buildChildNodes(currPathFileNode, false);
        SetWindowText(hwndMain, currPathFileNode->name);    
        updateAddrButtons();
        refreshContentView();
    }
}

static void onExtractISOFiles() {
    if (!targetPath) return;
    wchar_t parentPath[MAX_PATH] = {0};
    getParentDirFromPath(targetPath, parentPath);
    
    ShellExecute(hwndMain, L"open", targetPath, NULL, parentPath, SW_SHOW);
    MEMFREE(targetPath);
}

static void openCDDriveFile(wchar_t* path) {
    wchar_t mountFile[MAX_PATH] = {0};
    if (!getCDDriveMountFile(mountFile)) return;
    
    char filename[MAX_PATH] = {0};
    toUnixPath(path, filename);
    
    targetPath = wcsdup(path);
    
    clearDirectory(L"X:");
    if (hasFileExtension(path, L"exe")) {
        extractFilesFromISOImage(mountFile, NULL, L"X:\\", &onExtractISOFiles);
    }
    else extractFilesFromISOImage(mountFile, filename, path, &onExtractISOFiles);
}

void openFileNode(struct FileNode* node) {
    if (node->type == TYPE_FILE) {
        wchar_t path[MAX_PATH] = {0};
        wchar_t parentPath[MAX_PATH] = {0};
        getFileNodePath(node, path);
        
        if (isCDDrivePath(path) && !isPathExists(path)) {
            openCDDriveFile(path);
        }
        else {
            getFileNodePath(node->parent, parentPath);
            ShellExecute(hwndMain, L"open", path, NULL, parentPath, SW_SHOW);
        }
    }
    else navigateToFileNode(node);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
    int numArgs;
    wchar_t** args = CommandLineToArgvW(GetCommandLineW(), &numArgs);
    
    globalHInstance = hInstance;

    WNDCLASSEX wcx = {0};
    wcx.cbSize = sizeof(wcx);
    wcx.style = CS_HREDRAW | CS_VREDRAW;
    wcx.lpfnWndProc = &MainWndProc;
    wcx.cbClsExtra = 0;
    wcx.cbWndExtra = 0;
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));
    wcx.hCursor = LoadCursor(hInstance, IDC_ARROW);
    wcx.hbrBackground = (HBRUSH)COLOR_WINDOW;
    wcx.lpszMenuName = MAKEINTRESOURCE(IDM_MAIN_MENU);
    wcx.lpszClassName = mainWndClass;
    wcx.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN));

    if (!RegisterClassEx(&wcx)) return 0;
    
    initFileNodes();

    HWND hwndDesktop = GetDesktopWindow();
    RECT desktopRect;
    GetWindowRect(hwndDesktop, &desktopRect);
    int hwndWidth = (desktopRect.right - desktopRect.left) * 0.8f;
    int hwndHeight = (desktopRect.bottom - desktopRect.top) * 0.8f;
    
    hwndMain = CreateWindowEx(0, mainWndClass, L"", WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW, 
                              0, 0, hwndWidth, hwndHeight, NULL, NULL, hInstance, NULL);
    if (!hwndMain) return 0;
    
    createToolbar();
    createNavbar();
    createTreeview();
    createSizebar();
    createContentView();
    createStatusbar();
    
    setViewStyle(STYLE_DETAILS);
    int treeviewWidth = hwndWidth * 0.2f;
    SetWindowPos(hwndTreeview, NULL, 0, 0, treeviewWidth, 0, SWP_NOZORDER | SWP_NOMOVE);    
    
    if (numArgs > 1) {
        navigateToPath(args[1]);
    }
    else navigateRefresh();

    ShowWindow(hwndMain, SW_SHOW);
    UpdateWindow(hwndMain);

    MSG msg;
    while(GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    
    return (int)msg.wParam;
}
