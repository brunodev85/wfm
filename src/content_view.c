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
};

struct ContextMenuItem cmiOpen = {L"Open", &onMenuItemOpenClick};
struct ContextMenuItem cmiEdit = {L"Edit", &onMenuItemEditClick};
struct ContextMenuItem cmiCut = {L"Cut", &onMenuItemCutClick};
struct ContextMenuItem cmiCopy = {L"Copy", &onMenuItemCopyClick};
struct ContextMenuItem cmiCreateShortcut = {L"Create Shortcut", &onMenuItemCreateShortcutClick};
struct ContextMenuItem cmiDelete = {L"Delete", &onMenuItemDeleteClick};
struct ContextMenuItem cmiRename = {L"Rename", &onMenuItemRenameClick};
struct ContextMenuItem cmiPaste = {L"Paste", &onMenuItemPasteClick};
struct ContextMenuItem cmiPasteShortcut = {L"Paste Shortcut", &onMenuItemPasteShortcutClick};
struct ContextMenuItem cmiNewFolder = {L"New Folder", &onMenuItemNewFolderClick};
struct ContextMenuItem cmiNewFile = {L"New File", &onMenuItemNewFileClick};

static WNDPROC OrigWndProc;
static struct ListItem* items = NULL;
static int numItems = 0;
static enum ViewStyle viewStyle = STYLE_DETAILS;
static HMENU hContextMenu;
static char sortColumnIdx = COLUMN_NAME_IDX;
static bool sortAscending = true;

static struct FileNode** selectedItems = NULL;
static int numSelectedItems = 0;

static struct SearchData* searchData;

static void updateStatusbar() {
	wchar_t statusText[32];
	swprintf_s(statusText, 32, L"%d Items", numItems);
	setStatusbarText(statusText);	
}

static freeItems() {
	if (items) {
		for (int i = 0; i < numItems; i++) {
			if (items[i].path) free(items[i].path);
		}
		free(items);
		items = NULL;
	}
	numItems = 0;	
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
				cmItem->proc();
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
				item->size = 0;
				item->path = NULL;
				item->loaded = false;

				if (node->type == TYPE_FILE) {
					LARGE_INTEGER filesize;
					WIN32_FILE_ATTRIBUTE_DATA info;		
					
					wchar_t path[MAX_PATH];
					getFileNodePath(node, path);
					GetFileAttributesEx(path, GetFileExInfoStandard, &info);
					
					if ((info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {
						filesize.LowPart = info.nFileSizeLow;
						filesize.HighPart = info.nFileSizeHigh;
						item->size = filesize.QuadPart;
					}
					
					item->modifiedTime = info.ftLastWriteTime;
				}
				
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
	if (selectedItems) {
		free(selectedItems);
		selectedItems = NULL;
	}
	numSelectedItems = 0;

	int i = ListView_GetNextItem(hwndContentView, -1, LVNI_SELECTED);
	while (i != -1) {		
		int index = numSelectedItems++;
		selectedItems = realloc(selectedItems, numSelectedItems * sizeof(struct FileNode*));
		selectedItems[index] = items[i].node;
		i = ListView_GetNextItem(hwndContentView, i, LVNI_SELECTED);
	}
}

static void addContextMenuItem(int id, struct ContextMenuItem* cmItem, bool separate) {
	MENUITEMINFO item = {};
	item.cbSize = sizeof(MENUITEMINFO);
	item.fMask = MIIM_TYPE | MIIM_DATA | MIIM_ID;
	item.fType = MFT_STRING;

	item.dwTypeData = cmItem->text;
	item.cch = wcslen(cmItem->text);
	item.wID = id;
	item.dwItemData = (ULONG_PTR)cmItem;

	InsertMenuItem(hContextMenu, -1, TRUE, &item);

	if (separate) {
		item.fType = MFT_SEPARATOR;
		InsertMenuItem(hContextMenu, -1, TRUE, &item);
	}
}

static void createContextMenu(enum ContextMenuType type) {
	HMENU hMenu = CreatePopupMenu();
	hContextMenu = hMenu;

	int id = 0;

	if (type == MENU_SINGLE || type == MENU_MULTIPLE) {
		if (type == MENU_SINGLE) {
			if (selectedItems[0]->type == TYPE_FILE) {
				addContextMenuItem(id++, &cmiOpen, false);
				addContextMenuItem(id++, &cmiEdit, true);
			}
			else addContextMenuItem(id++, &cmiOpen, true);
		}
		addContextMenuItem(id++, &cmiCut, false);
		addContextMenuItem(id++, &cmiCopy, true);
		addContextMenuItem(id++, &cmiCreateShortcut, false);
		addContextMenuItem(id++, &cmiDelete, false);
		if (type == MENU_SINGLE) addContextMenuItem(id++, &cmiRename, false);
	}
	else {
		addContextMenuItem(id++, &cmiPaste, false);
		addContextMenuItem(id++, &cmiPasteShortcut, true);
		addContextMenuItem(id++, &cmiNewFolder, false);
		addContextMenuItem(id++, &cmiNewFile, false);
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
				SHFILEINFO sfi;
				wchar_t path[MAX_PATH];
				getFileNodePath(item->node, path);
				
				if (viewStyle == STYLE_LARGE_ICON) {
					SHGetFileInfo(path, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_ICON | SHGFI_TYPENAME);
				}
				else SHGetFileInfo(path, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME);

				if (item->node->type == TYPE_FILE) {
					formatFileSize(item->size, item->formattedSize);
					
					SYSTEMTIME systemTime;
					FILETIME localFiletime;
					if (FileTimeToLocalFileTime(&item->modifiedTime, &localFiletime) && FileTimeToSystemTime(&localFiletime, &systemTime)) {
						swprintf_s(&item->formattedDate, 32, L"%02d/%02d/%04d %02d:%02d", systemTime.wMonth, systemTime.wDay, systemTime.wYear, systemTime.wHour, systemTime.wMinute);
					}
				}

				item->icon = sfi.iIcon;
				wcscpy_s(item->type, 80, sfi.szTypeName);
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
							wchar_t path[MAX_PATH];
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

static void searchTask(void* data) {
	struct SearchData* searchData = (struct SearchData*)data;
	
	const int maxStackSize = 50;
	struct FileNode* stack[maxStackSize];
	int stackSize = 0;
	stack[stackSize++] = currPathFileNode->children;
	
	while (stackSize > 0 && numItems < 10000 && searchData->active) {
		struct FileNode* node = stack[--stackSize];
		while (node && searchData->active) {
			if (wcsstr(node->name, searchData->keyword)) {
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
}

void searchFor(wchar_t* keyword) {
	if (wcslen(keyword) == 0) return;
	if (searchData != NULL && searchData->active) {
		searchData->active = false;
		return;
	}
	
	freeItems();
	
	ListView_SetItemCountEx(hwndContentView, 0, 0);
	ListView_DeleteColumn(hwndContentView, COLUMN_PATH_IDX);
	
	LVCOLUMN column = {};
	column.mask = LVCF_WIDTH | LVCF_TEXT;
	column.cx = 250;
	column.pszText = L"Path";
	ListView_InsertColumn(hwndContentView, COLUMN_PATH_IDX, &column);
	UpdateWindow(hwndContentView);
	
	searchData = malloc(sizeof(struct SearchData));
	searchData->keyword = keyword;
	searchData->active = true;
	searchData->canceled = false;
	_beginthread(&searchTask, 0, searchData);	
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
	LVCOLUMN column = {};
	column.mask = LVCF_WIDTH | LVCF_TEXT;

	column.cx = 220;
	column.pszText = L"Name";
	ListView_InsertColumn(hwndContentView, COLUMN_NAME_IDX, &column);

	column.cx = 100;
	column.pszText = L"Type";
	ListView_InsertColumn(hwndContentView, COLUMN_TYPE_IDX, &column);

	column.cx = 60;
	column.pszText = L"Size";
	ListView_InsertColumn(hwndContentView, COLUMN_SIZE_IDX, &column);

	column.cx = 100;
	column.pszText = L"Date";
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
		
		wchar_t path[MAX_PATH];
		wchar_t parameters[MAX_PATH];
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
		wchar_t* result = InputDialog(L"Rename", L"Enter new name:", selectedItems[0]->name, true);
		if (result) {
			wchar_t newFilename[MAX_PATH];
			getFileNodePath(selectedItems[0]->parent, newFilename);
			wcscat_s(newFilename, MAX_PATH, L"\\");
			wcscat_s(newFilename, MAX_PATH, result);
			free(result);
			
			wchar_t oldFilename[MAX_PATH];
			getFileNodePath(selectedItems[0], oldFilename);
			MoveFileW(oldFilename, newFilename);
			navigateRefresh();
		}
	}
}

void onMenuItemPasteClick() {
	wchar_t path[MAX_PATH];
	getFileNodePath(currPathFileNode, path);
	if (!isPathExists(path)) return;
	pasteFiles(path);
}

void onMenuItemPasteShortcutClick() {
	wchar_t path[MAX_PATH];
	getFileNodePath(currPathFileNode, path);
	if (!isPathExists(path)) return;
	pasteShortcuts(path);	
}

void onMenuItemNewFolderClick() {
	wchar_t path[MAX_PATH];
	getFileNodePath(currPathFileNode, path);
	if (!isPathExists(path)) return;
	
	wchar_t* result = InputDialog(L"New Folder", L"Enter folder name:", NULL, false);
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
	wchar_t path[MAX_PATH];
	getFileNodePath(currPathFileNode, path);
	if (!isPathExists(path)) return;
	
	wchar_t* result = InputDialog(L"New File", L"Enter file name:", NULL, false);
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
	
	freeItems();
	
	ListView_DeleteColumn(hwndContentView, COLUMN_PATH_IDX);
	ListView_SetItemCountEx(hwndContentView, 0, 0);
	UpdateWindow(hwndContentView);
	
	wchar_t path[MAX_PATH];
	struct FileNode* child = currPathFileNode->children;
	LARGE_INTEGER filesize;
	WIN32_FILE_ATTRIBUTE_DATA info;
	HIMAGELIST himl = 0;
	
	numItems = getChildNodeCount(currPathFileNode);
	items = malloc(numItems * sizeof(struct ListItem));
	int index = 0;
	
	while (child) {
		getFileNodePath(child, path);
		GetFileAttributesEx(path, GetFileExInfoStandard, &info);
		struct ListItem* item = &items[index++];
		item->node = child;
		item->size = 0;
		
		if ((info.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {
			filesize.LowPart = info.nFileSizeLow;
			filesize.HighPart = info.nFileSizeHigh;
			item->size = filesize.QuadPart;
		}
		
		item->modifiedTime = info.ftLastWriteTime;
		item->path = NULL;
		item->loaded = false;
		
		if (himl == 0) {
			SHFILEINFO sfi;				
			if (viewStyle == STYLE_LARGE_ICON) {
				himl = (HIMAGELIST)SHGetFileInfo(path, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_ICON);
			}
			else himl = (HIMAGELIST)SHGetFileInfo(path, 0, &sfi, sizeof(SHFILEINFO), SHGFI_SYSICONINDEX | SHGFI_SMALLICON);
		}
		child = child->sibling;
	}

	if (viewStyle == STYLE_LARGE_ICON) {
		ListView_SetImageList(hwndContentView, himl, LVSIL_NORMAL);
	}
	else ListView_SetImageList(hwndContentView, himl, LVSIL_SMALL);

	if (sortColumnIdx != -1) sortItems();
	ListView_SetItemCountEx(hwndContentView, numItems, 0);
	
	updateStatusbar();	
}