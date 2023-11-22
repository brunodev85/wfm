#ifndef TOOLBAR_H
#define TOOLBAR_H

HWND hwndToolbar;

void createToolbar();
void toolbarCommand(int command);
void onUpButtonClick();
void onCopyButtonClick();
void onCutButtonClick();
void onPasteButtonClick();
void onDeleteButtonClick();
void onNewFolderButtonClick();
void onNewFileButtonClick();

#endif