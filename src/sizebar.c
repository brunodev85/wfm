#include "main.h"

static WNDPROC OrigWndProc;
static bool isTracking = false;
static bool isSizing = false;
static POINTS currPos;
static int startX;
static HCURSOR cursorArrow;
static HCURSOR cursorSizeLR;
static HWND hwndPrevCapture;

extern HINSTANCE globalHInstance;
extern HWND hwndMain;
extern HWND hwndTreeview;

HWND hwndSizebar = NULL;

static void setMouseTracking() {
    TRACKMOUSEEVENT tme = {};
    tme.cbSize = sizeof(TRACKMOUSEEVENT);
    tme.dwFlags = TME_LEAVE;
    tme.hwndTrack = hwndSizebar;
    TrackMouseEvent(&tme);
    isTracking = true;
}

LRESULT CALLBACK SizebarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_NCHITTEST:
            return HTCLIENT;
        case WM_MOVE: {
            currPos = MAKEPOINTS(lParam);
            break;
        }
        case WM_SETCURSOR: {
            return 1;
        }       
        case WM_LBUTTONDOWN: {
            startX = MAKEPOINTS(lParam).x;
            hwndPrevCapture = SetCapture(hwnd);
            isSizing = true;
            break;
        }
        case WM_LBUTTONUP: {
            ReleaseCapture();
            if (hwndPrevCapture) SetCapture(hwndPrevCapture);
            isSizing = false;
            break;
        }       
        case WM_MOUSELEAVE: {
            SetCursor(cursorArrow);
            isTracking = false;
            break;
        }
        case WM_MOUSEMOVE: {
            POINTS point = MAKEPOINTS(lParam);
            if (isSizing) {
                int dx = point.x - startX;
                RECT rect;
                GetWindowRectInParent(hwndTreeview, &rect);
                int width = (rect.right - rect.left) + dx;
                if (width > 50) {
                    SetWindowPos(hwndTreeview, NULL, 0, 0, width, rect.bottom - rect.top, SWP_NOZORDER | SWP_NOMOVE);
                    resizeControls();
                }
            } 
            else if (!isTracking) {
                SetCursor(cursorSizeLR);
                setMouseTracking();
            }
            break;
        }       
    }
    return OrigWndProc(hwnd, msg, wParam, lParam);
}

void createSizebar() {
    hwndSizebar = CreateWindowEx(0, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS, 
                                 0, 0, 0, 0, hwndMain, (HMENU)NULL, globalHInstance, NULL);
                                 
    cursorArrow = LoadCursor(NULL, IDC_ARROW);
    cursorSizeLR = LoadCursor(NULL, IDC_SIZEWE);                                 
                                 
    OrigWndProc = (WNDPROC)SetWindowLongPtr(hwndSizebar, GWLP_WNDPROC, (LONG_PTR)SizebarWndProc);
    UpdateWindow(hwndSizebar);
}