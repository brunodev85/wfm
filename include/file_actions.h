#ifndef FILE_ACTIONS_H
#define FILE_ACTIONS_H

void deleteFiles(struct FileNode** nodes, int count);
void clearClipboard();
void copyFiles(struct FileNode** nodes, int count);
void cutFiles(struct FileNode** nodes, int count);
void pasteFiles(wchar_t* dstDir);
void pasteShortcuts(wchar_t* dstDir);
void createDesktopShortcuts(struct FileNode** nodes, int count);
void extractFilesFromISOImage(wchar_t* isoPath, char* localPath, wchar_t* dstPath, void* callback);

#endif