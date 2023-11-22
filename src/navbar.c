#include "main.h"

struct AddrButton {	
	HWND hwnd;
	struct FileNode* node;
	int width;
	bool isArrow;
	bool isHidden;
};

static const int buttonSize = 30;

static WNDPROC OrigWndProc;
static WNDPROC AddrEditOrigWndProc;
static WNDPROC SearchEditOrigWndProc;
static WNDPROC SearchEditWrapperOrigWndProc;

static HWND hwndAddrEditWrapper;
static HWND hwndAddrEdit;
static HWND hwndSearchEditWrapper;
static HWND hwndSearchEdit;
static HWND hwndGoButton;
static HWND hwndRefreshButton;
static HWND hwndSearchButton;
static HANDLE hMorePopupMenu;

static bool isEditMode;
static wchar_t keyword[64];
static bool searchEditEmpty = true;
static struct AddrButton* addrButtons = NULL;
static int numAddrButtons = 0;

static void createMorePopupMenu(struct FileNode* parent) {	
	HMENU menu = CreatePopupMenu();

	MENUITEMINFO item = {};
	item.cbSize = sizeof(MENUITEMINFO);
	item.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
	item.fType = MFT_STRING;

	buildChildNodes(parent, false);
	
	struct FileNode* child = parent->children;
	int index = 0;
	while (child) {
		if (child->type != TYPE_FILE) {			
			item.dwTypeData = child->name;
			item.cch = wcslen(child->name);
			item.wID = index++;
			item.dwItemData = (ULONG_PTR)child;			

			InsertMenuItem(menu, -1, TRUE, &item);			
		}
		child = child->sibling;
	}		
	
	hMorePopupMenu = menu;
	POINT cursor;
	GetCursorPos(&cursor);
	TrackPopupMenu(menu, 0, cursor.x, cursor.y, 0, hwndNavbar, NULL);
}

static void resizeAddrButtons() {
	RECT addrEditRect;
	GetWindowRectInParent(hwndAddrEditWrapper, &addrEditRect);
	int maxWidth = (addrEditRect.right - addrEditRect.left) - 50;
	int currX = addrEditRect.left + 1;
	int height = (addrEditRect.bottom - addrEditRect.top) - 2;	

	int currWidth = 0;
	for (int i = 0; i < numAddrButtons; i++) {
		struct AddrButton* button = &addrButtons[i];
		currWidth += button->width;
		button->isHidden = currWidth > maxWidth;
		
		if (button->isHidden) {
			ShowWindow(button->hwnd, SW_HIDE);
		}
		else {
			SetWindowPos(button->hwnd, NULL, currX, addrEditRect.top + 1, button->width, height, SWP_SHOWWINDOW);
			currX += button->width;
		}		
	}
}

static void setEditMode(bool value) {
	isEditMode = value;
	for (int i = 0; i < numAddrButtons; i++) {
		struct AddrButton* button = &addrButtons[i];
		if (!button->isHidden) ShowWindow(button->hwnd, isEditMode ? SW_HIDE : SW_SHOW);
	}

	SendMessage(hwndAddrEdit, EM_SETREADONLY, isEditMode ? FALSE : TRUE, 0);

	if (isEditMode) {
		wchar_t path[MAX_PATH];
		getFileNodePath(currPathFileNode, path);
		SendMessage(hwndAddrEdit, WM_SETTEXT, 0, (LPARAM)path);
		PostMessage(hwndAddrEdit, EM_SETSEL, 0, -1);
	}
	else SendMessage(hwndAddrEdit, WM_SETTEXT, 0, (LPARAM)L"");

	if (!isEditMode) resizeAddrButtons();
	InvalidateRect(hwndAddrEditWrapper, NULL, TRUE);
}

static void updateSearchEdit() {
	SendMessage(hwndSearchEdit, WM_GETTEXT, 64, (LPARAM)keyword);
	searchEditEmpty = wcslen(keyword) == 0;
	if (searchEditEmpty) {
		swprintf_s(keyword, 64, L"Search %ls", currPathFileNode->name);
		SendMessage(hwndSearchEdit, WM_SETTEXT, 0, (LPARAM)keyword);
	}
}

LRESULT CALLBACK NavbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {	
	switch (msg) {
		case WM_CTLCOLORSTATIC: {
			HWND hwndControl = (HWND)lParam;
			if (hwndControl == hwndAddrEditWrapper) {
				return isEditMode ? (HBRUSH)GetStockObject(WHITE_BRUSH) : (HBRUSH)COLOR_WINDOW;
			}
			else if (hwndControl == hwndSearchEditWrapper) {
				return (HBRUSH)GetStockObject(WHITE_BRUSH);
			}
			break;
		}
		case WM_SIZE: {
			RECT rect;
			GetClientRect(hwnd, &rect);		
			
			const int margin = 4;
			const int searchEditWidth = 160;
			const int editWrapperHeight = buttonSize - margin;
			const int addrEditHeight = 16;
			const int addrEditY = (editWrapperHeight - addrEditHeight) / 2;
			
			int offsetX = rect.right - (margin + buttonSize);
			
			SetWindowPos(hwndSearchButton, NULL, offsetX, 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			offsetX -= searchEditWidth + margin;
			
			SetWindowPos(hwndSearchEditWrapper, NULL, offsetX, margin, searchEditWidth, editWrapperHeight, SWP_NOZORDER);
			SetWindowPos(hwndSearchEdit, NULL, margin, addrEditY, searchEditWidth, addrEditHeight, SWP_NOZORDER);
			offsetX -= buttonSize + margin;
			
			SetWindowPos(hwndRefreshButton, NULL, offsetX, 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			offsetX -= buttonSize + margin;
			
			SetWindowPos(hwndGoButton, NULL, offsetX, 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
			
			int addrEditWidth = offsetX - margin - margin;
			SetWindowPos(hwndAddrEditWrapper, NULL, margin, margin, addrEditWidth, editWrapperHeight, SWP_NOZORDER);
			SetWindowPos(hwndAddrEdit, NULL, margin, addrEditY, addrEditWidth, addrEditHeight, SWP_NOZORDER);
			break;
		}
		case WM_COMMAND: {
			HWND hwndControl = (HWND)lParam;
			for (int i = 0; i < numAddrButtons; i++) {
				struct AddrButton* button = &addrButtons[i];
				if (hwndControl == button->hwnd) {
					if (button->isArrow) {
						createMorePopupMenu(button->node);
					}
					else {
						navigateToFileNode(button->node);
						SetFocus(hwndContentView);
					}
					return 0;
				}
			}

			if (hwndControl == hwndGoButton) {
				int len = GetWindowTextLength(hwndAddrEdit);
				wchar_t path[len + 1];
				SendMessage(hwndAddrEdit, WM_GETTEXT, len + 1, (LPARAM)path);
				setEditMode(false);
				setCurrPathFromString(path);
				SetFocus(hwndContentView);
				navigateRefresh();
			}
			else if (hwndControl == hwndRefreshButton) {
				navigateRefresh();
				SetFocus(hwndContentView);
			}
			else if (hwndControl == hwndSearchButton) {
				if (!searchEditEmpty) {
					SendMessage(hwndSearchEdit, WM_GETTEXT, 64, (LPARAM)keyword);
					searchFor(keyword);
				}
				else refreshContentView();
			}
			else if (hwndControl == 0) {
				MENUITEMINFO item;
				item.cbSize = sizeof(MENUITEMINFO);
				item.fMask = MIIM_DATA;
				GetMenuItemInfo(hMorePopupMenu, LOWORD(wParam), FALSE, &item);
				struct FileNode* node = (struct FileNode*)item.dwItemData;
				
				wchar_t path[MAX_PATH];
				getFileNodePath(node, path);
				
				setCurrPathFromString(path);
				SetFocus(hwndContentView);
				navigateRefresh();
			}
			return 0;
		}
	}

	return OrigWndProc(hwnd, msg, wParam, lParam);	
}

LRESULT CALLBACK SearchEditWrapperWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_CTLCOLOREDIT) {
		HWND hwndControl = (HWND)lParam;
		if (hwndControl == hwndSearchEdit) {
			SetTextColor((HDC)wParam, searchEditEmpty ? RGB(128, 128, 128) : RGB(0, 0, 0));
			return (HBRUSH)GetStockObject(WHITE_BRUSH);
		}
	}
	return SearchEditWrapperOrigWndProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK AddrEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KILLFOCUS: {
			setEditMode(false);
			break;
		}
		case WM_LBUTTONDOWN: {
			if (!isEditMode) setEditMode(true);
			break;
		}
		case WM_SIZE: {
			resizeAddrButtons();
			break;
		}
	}

	return AddrEditOrigWndProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK SearchEditWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case WM_KILLFOCUS: {
			updateSearchEdit();
			break;
		}
		case WM_SETFOCUS:
		case WM_LBUTTONDOWN: {
			searchEditEmpty = false;
			SendMessage(hwndSearchEdit, WM_SETTEXT, 0, (LPARAM)L"");
			break;
		}
	}

	return SearchEditOrigWndProc(hwnd, msg, wParam, lParam);
}

int getNavbarHeight() {
	RECT rect;
	GetWindowRectInParent(hwndRefreshButton, &rect);
	return rect.bottom - rect.top + 6;	
}

static struct AddrButton* addAddrButton() {
	int index = numAddrButtons++;
	addrButtons = realloc(addrButtons, numAddrButtons * sizeof(struct AddrButton));
	struct AddrButton* button = &addrButtons[index];
	button->hwnd = CreateWindowEx(0, WC_BUTTON, NULL, WS_CHILD | WS_CLIPSIBLINGS, 0, 0, 0, 0, hwndNavbar, (HMENU)NULL, globalHInstance, NULL);
	button->isArrow = false;
	SendMessage(button->hwnd, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	return button;
}

static void addArrowAddrButton(struct FileNode* node) {
	struct AddrButton* button = addAddrButton();
	button->isArrow = true;
	button->node = node;

	SetWindowLongPtr(button->hwnd, GWL_STYLE, GetWindowLongPtr(button->hwnd, GWL_STYLE) | BS_ICON);

	HICON hiNavArrow = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_NAV_ARROW), IMAGE_ICON, 16, 16, 0);
	SendMessageW(button->hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hiNavArrow);
	button->width = 16;
}

static void calcAddrButtonWidth(struct AddrButton* button) {
	HDC hdc = GetDC(button->hwnd);
	HGDIOBJ prevFont = SelectObject(hdc, (HFONT)SendMessage(button->hwnd, WM_GETFONT, 0, 0));
	SIZE size;
	int textLen = wcslen(button->node->name);
	GetTextExtentPoint32(hdc, button->node->name, textLen, &size);
	button->width = size.cx + 12;
	SelectObject(hdc, prevFont);
	ReleaseDC(button->hwnd, hdc);
}

static void clearAddrButtons() {
	for (int i = 0; i < numAddrButtons; i++) {
		DestroyWindow(addrButtons[i].hwnd);
	}
	
	if (addrButtons) {
		free(addrButtons);
		addrButtons = NULL;
	}
	
	numAddrButtons = 0;	
}

void updateAddrButtons() {
	clearAddrButtons();
	
	int count = 0;
	struct FileNode* node = currPathFileNode;
	while (node) {
		count++;
		node = node->parent;
	}
	
	struct FileNode* nodes[count];
	node = currPathFileNode;
	int i = 0;
	while (node) {
		nodes[i++] = node;
		node = node->parent;
	}	

	for (i = count-1; i >= 0; i--) {
		struct AddrButton* button = addAddrButton();
		button->node = nodes[i];
		SendMessage(button->hwnd, WM_SETTEXT, 0, (LPARAM)nodes[i]->name);
		calcAddrButtonWidth(button);
		addArrowAddrButton(nodes[i]);
	}

	resizeAddrButtons();
	
	SendMessage(hwndSearchEdit, WM_SETTEXT, 0, (LPARAM)L"");
	updateSearchEdit();
}

static void createNavButtons() {
	hwndGoButton = CreateWindowEx(0, WC_BUTTON, NULL, WS_VISIBLE | WS_CHILD | BS_ICON, 
								  0, 0, buttonSize, buttonSize, hwndNavbar, NULL, globalHInstance, NULL);
	HICON hiGo = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_GO), IMAGE_ICON, 16, 16, 0);
	SendMessage(hwndGoButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hiGo);
	SetWindowPos(hwndGoButton, NULL, 0, 0, buttonSize, buttonSize, SWP_NOZORDER | SWP_NOMOVE);
	
	hwndRefreshButton = CreateWindowEx(0, WC_BUTTON, NULL, WS_VISIBLE | WS_CHILD | BS_ICON, 
									   0, 0, buttonSize, buttonSize, hwndNavbar, NULL, globalHInstance, NULL);
	HICON hiRefresh = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_REFRESH), IMAGE_ICON, 16, 16, 0);
	SendMessage(hwndRefreshButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hiRefresh);
	SetWindowPos(hwndRefreshButton, NULL, 0, 0, buttonSize, buttonSize, SWP_NOZORDER | SWP_NOMOVE);
	
	hwndSearchButton = CreateWindowEx(0, WC_BUTTON, NULL, WS_VISIBLE | WS_CHILD | BS_ICON, 
									  0, 0, buttonSize, buttonSize, hwndNavbar, NULL, globalHInstance, NULL);
	HICON hiSearch = (HICON)LoadImage(globalHInstance, MAKEINTRESOURCE(IDI_SEARCH), IMAGE_ICON, 16, 16, 0);
	SendMessage(hwndSearchButton, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hiSearch);
	SetWindowPos(hwndSearchButton, NULL, 0, 0, buttonSize, buttonSize, SWP_NOZORDER | SWP_NOMOVE);		
}

void createNavbar() {
	hwndNavbar = CreateWindowEx(0, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, 
								0, 0, 0, 0, hwndMain, (HMENU)NULL, globalHInstance, NULL);
								
	OrigWndProc = (WNDPROC)SetWindowLongPtr(hwndNavbar, GWLP_WNDPROC, (LONG_PTR)NavbarWndProc);

	hwndAddrEditWrapper = CreateWindowEx(0, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_BORDER, 
										 0, 0, 0, 0, hwndNavbar, (HMENU)NULL, globalHInstance, NULL);	
								
	hwndAddrEdit = CreateWindowEx(0, WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_LEFT,
								  0, 0, 0, 0, hwndAddrEditWrapper, (HMENU)NULL, globalHInstance, NULL);
	SendMessage(hwndAddrEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	AddrEditOrigWndProc = (WNDPROC)SetWindowLongPtr(hwndAddrEdit, GWLP_WNDPROC, (LONG_PTR)AddrEditWndProc);		
	
	hwndSearchEditWrapper = CreateWindowEx(0, WC_STATIC, NULL, WS_VISIBLE | WS_CHILD | WS_BORDER, 
										   0, 0, 0, 0, hwndNavbar, (HMENU)NULL, globalHInstance, NULL);
	SearchEditWrapperOrigWndProc = (WNDPROC)SetWindowLongPtr(hwndSearchEditWrapper, GWLP_WNDPROC, (LONG_PTR)SearchEditWrapperWndProc);
										   
	hwndSearchEdit = CreateWindowEx(0, WC_EDIT, NULL, WS_VISIBLE | WS_CHILD | ES_AUTOHSCROLL | ES_LEFT, 
								    0, 0, 0, 0, hwndSearchEditWrapper, (HMENU)NULL, globalHInstance, NULL);
	SendMessage(hwndSearchEdit, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);
	SearchEditOrigWndProc = (WNDPROC)SetWindowLongPtr(hwndSearchEdit, GWLP_WNDPROC, (LONG_PTR)SearchEditWndProc);			

	setEditMode(false);
	createNavButtons();	
	UpdateWindow(hwndNavbar);								
}