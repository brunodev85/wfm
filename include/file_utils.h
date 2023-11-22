#ifndef FILE_UTILS_H
#define FILE_UTILS_H

void formatFileSize(uint64_t size, wchar_t* formattedSize);
void getParentDirFromPath(wchar_t* path, wchar_t* result);
void getBasenameFromPath(wchar_t* path, wchar_t* result, bool removeExt);

#endif