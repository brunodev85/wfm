#include "main.h"

#define TBBUTTON_COMMAND_OFFSET (WM_APP + 100)

struct ToolButton {
	wchar_t *text;
	int bmpId;
	void(*proc)();
	bool separate;
};

struct ToolButton buttons[] = {
	{L"Up", 0, &onMenuItemUpClick, true},
	{L"Copy", 1, &onMenuItemCopyClick, false},
	{L"Cut", 2, &onMenuItemCutClick, false},
	{L"Paste", 3, &onMenuItemPasteClick, false},
	{L"Delete", 4, &onMenuItemDeleteClick, true},
	{L"New Folder", 5, &onMenuItemNewFolderClick, false},
	{L"New File", 6, &onMenuItemNewFileClick, false}
};

static WNDPROC OrigWndProc;

LRESULT CALLBACK ToolbarWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	return OrigWndProc(hwnd, msg, wParam, lParam);
}

void createToolButtons() {
	const int numButtons = sizeof(buttons) / sizeof(buttons[0]);

	SendMessage(hwndToolbar, TB_BUTTONSTRUCTSIZE, (WPARAM)sizeof(TBBUTTON), 0);

	HBITMAP hbToolbar = (HBITMAP)LoadImage(globalHInstance, MAKEINTRESOURCE(IDB_TOOLBAR), IMAGE_BITMAP, 112, 16, 0);

	SendMessage(hwndToolbar, TB_SETINDENT, 2, 0);

	HIMAGELIST hImageList = ImageList_Create(16, 16, ILC_MASK | ILC_COLOR24, numButtons, 0);
	ImageList_AddMasked(hImageList, hbToolbar, RGB(212, 212, 212));
	SendMessage(hwndToolbar, TB_SETIMAGELIST, 0, (LPARAM)hImageList);

	TBBUTTON tbbSeparator = {};
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