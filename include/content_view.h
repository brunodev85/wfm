#ifndef CONTENT_VIEW_H
#define CONTENT_VIEW_H

enum ViewStyle {
	STYLE_LARGE_ICON,
	STYLE_SMALL_ICON,
	STYLE_LIST,
	STYLE_DETAILS
};

HWND hwndContentView;

LRESULT contentViewNotify(NMHDR* nmhdr);
void createContentView();
void refreshContentView();
void setViewStyle(enum ViewStyle newViewStyle);
void searchFor(wchar_t* keyword);

void onMenuItemUpClick();
void onMenuItemOpenClick();
void onMenuItemEditClick();
void onMenuItemCutClick();
void onMenuItemCopyClick();
void onMenuItemCreateShortcutClick();
void onMenuItemDeleteClick();
void onMenuItemRenameClick();
void onMenuItemPasteClick();
void onMenuItemPasteShortcutClick();
void onMenuItemNewFolderClick();
void onMenuItemNewFileClick();
void onMenuItemSelectAllClick();

#endif