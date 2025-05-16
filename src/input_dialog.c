#include "main.h"

struct Options {
    wchar_t* title;
    wchar_t* label;
    wchar_t* defaultText;
    bool selectAll;
};

static struct Options options = {0};
static wchar_t* result = NULL;

extern HINSTANCE globalHInstance;
extern HWND hwndMain;

INT_PTR CALLBACK InputDialogProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {      
        case WM_COMMAND: {
            if (LOWORD(wParam) == IDOK) {
                HWND hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT);
                int len = GetWindowTextLength(hwndEdit);
                if (len > 0) {
                    result = calloc(len + 1, sizeof(wchar_t));
                    SendMessage(hwndEdit, WM_GETTEXT, len + 1, (LPARAM)result);
                }
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            } 
            else if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(hwndDlg, LOWORD(wParam));
                return (INT_PTR)TRUE;
            }
            break;
        }
        case WM_INITDIALOG: {
            RECT rect, rect1;
            GetWindowRect(GetParent(hwndDlg), &rect);
            GetClientRect(hwndDlg, &rect1);
            SetWindowPos(hwndDlg, NULL, (rect.right + rect.left) / 2 - (rect1.right - rect1.left) / 2,
                         (rect.bottom + rect.top) / 2 - (rect1.bottom - rect1.top) / 2,
                         0, 0, SWP_NOZORDER | SWP_NOSIZE);

            SetWindowText(hwndDlg, options.title);
            SetWindowText(GetDlgItem(hwndDlg, IDC_LABEL), options.label);
            
            HWND hwndEdit = GetDlgItem(hwndDlg, IDC_EDIT);
            if (options.defaultText) SetWindowText(hwndEdit, options.defaultText);

            SendMessage(hwndDlg, WM_NEXTDLGCTL, (WPARAM)hwndEdit, TRUE);
            if (options.selectAll) SendMessage(hwndEdit, EM_SETSEL, 0, -1);
            break;
        }
    }

    return (INT_PTR)FALSE;  
}

wchar_t* InputDialog(wchar_t* title, wchar_t* label, wchar_t* defaultText, bool selectAll) {
    options.title = title;
    options.label = label;
    options.defaultText = defaultText;
    options.selectAll = selectAll;
    result = NULL;
    
    DialogBox(globalHInstance, MAKEINTRESOURCE(IDD_INPUT), hwndMain, &InputDialogProc);
    
    options.title = NULL;
    options.label = NULL;
    options.defaultText = NULL;
    options.selectAll = false;
    return result;
}