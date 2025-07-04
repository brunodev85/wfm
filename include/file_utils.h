#ifndef FILE_UTILS_H
#define FILE_UTILS_H

#include "main.h"
#include "string_utils.h"

enum FileType {
    TYPE_DIR,
    TYPE_FILE,
    TYPE_DRIVE,
    TYPE_DESKTOP,
    TYPE_PERSONAL,
    TYPE_COMPUTER
};

struct FileInfo {
    int icon;
    wchar_t typeName[80];
};

static inline bool isPathExists(wchar_t* path) {
    DWORD dwAttrib = GetFileAttributes(path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && (
           (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) || (dwAttrib & FILE_ATTRIBUTE_ARCHIVE)));
}

static inline void formatFileSize(uint64_t size, wchar_t* formattedSize) {
    static const wchar_t units[5][11] = {L"bytes", L"KB", L"MB", L"GB", L"TB"};
    
    if (size > 0) {
        int digitGroups = (int)(log10(size) / log10(1024));
        swprintf_s(formattedSize, 32, L"%.2f %ls", size / pow(1024, digitGroups), units[digitGroups]);
        
        wchar_t* lastDot = wcsrchr(formattedSize, L'.');
        if (lastDot) {
            int offset = lastDot - formattedSize;
            if (formattedSize[offset+1] == L'\0' && formattedSize[offset+2] == L'\0') {
                int len = wcslen(formattedSize);
                for (int i = 0; i < len-3; i++) formattedSize[offset+i] = formattedSize[offset+i+3];
            }
        }
    }
    else wcscpy_s(formattedSize, 32, L"0 bytes");
}

static inline void getParentDirFromPath(wchar_t* path, wchar_t* result) {
    wchar_t* lastSlash = wcsrchr(path, L'\\');
    int len = lastSlash ? lastSlash - path + 1 : 1;

    memcpy(result, path, (len - 1) * sizeof(wchar_t));
    result[len-1] = L'\0';
}

static inline void getBasenameFromPath(wchar_t* path, wchar_t* result, bool removeExt) {
    wchar_t* lastSlash = wcsrchr(path, L'\\');
    int offset = lastSlash ? lastSlash - path + 1 : 0;
    int len = (wcslen(path) - offset) + 1;
    
    memcpy(result, path + offset, len * sizeof(wchar_t));
    result[len-1] = L'\0';
    
    if (removeExt) {
        wchar_t* lastDot = wcsrchr(result, L'.');
        if (lastDot) {
            offset = lastDot - result;
            result[offset] = L'\0';
        }     
    }
}

static inline void toUnixPath(wchar_t* dosPath, char* result) {
    wchar_t unixPath[MAX_PATH] = {0};
    wcscpy_s(unixPath, MAX_PATH, dosPath + 2);
    
    int count = 0;
    wchar_t* ptr = unixPath;
    while (*ptr && count++ < MAX_PATH) {
        if (*ptr == L'\\') *ptr = L'/';
        ptr++;
    }
    
    if (unixPath[0] != L'/') unixPath[0] = L'/';
    WideCharToMultiByte(CP_ACP, 0, unixPath, -1, result, MAX_PATH, NULL, NULL);       
}

static inline void formatModifiedDate(int month, int day, int year, int hour, int minute, wchar_t* result, int size) {
    swprintf_s(result, size, L"%02d/%02d/%04d %02d:%02d", month, day, year, hour, minute);
}

static inline wchar_t* getFileExtension(wchar_t* path) {
    wchar_t* ext = wcsrchr(path, L'.');
    return ext && *ext++ != L'\0' ? ext : NULL;
}

static inline bool isCDDrivePath(wchar_t* path) {
    return (path[0] == L'x' || path[0] == L'X') && path[1] == L':';
}

static inline bool hasFileExtension(wchar_t* path, wchar_t* targetExt) {
    wchar_t* ext = getFileExtension(path);
    return ext && wcsicmp(ext, targetExt) == 0;
}

static inline void getFileInfo(wchar_t* path, enum FileType type, bool largeIcon, struct FileInfo* result) {
    SHFILEINFO sfi = {0};
    result->icon = 0;
    DWORD flags = SHGFI_SYSICONINDEX | (largeIcon ? SHGFI_ICON : SHGFI_SMALLICON);
    if (SHGetFileInfo(path, 0, &sfi, sizeof(SHFILEINFO), flags)) {
        result->icon = sfi.iIcon;
    }
    else {        
        if (type == TYPE_DIR) {
            flags |= SHGFI_USEFILEATTRIBUTES;
            SHGetFileInfo(L"dir", FILE_ATTRIBUTE_DIRECTORY, &sfi, sizeof(SHFILEINFO), flags);
            result->icon = sfi.iIcon;
        }
        else if (type == TYPE_FILE) {
            flags |= SHGFI_USEFILEATTRIBUTES;
            SHGetFileInfo(path, FILE_ATTRIBUTE_ARCHIVE, &sfi, sizeof(SHFILEINFO), flags);
            result->icon = sfi.iIcon;
        }       
    }
    
    switch (type) {
        case TYPE_DIR:
            wcscpy_s(result->typeName, 80, lc_str.folder);
            break;
        case TYPE_DRIVE: {
            if (isCDDrivePath(path)) {
                wcscpy_s(result->typeName, 80, lc_str.cd_drive);           
            }
            else wcscpy_s(result->typeName, 80, lc_str.local_drive);
            break;
        }
        case TYPE_DESKTOP:
            wcscpy_s(result->typeName, 80, lc_str.desktop);
            break;
        case TYPE_PERSONAL:
            wcscpy_s(result->typeName, 80, lc_str.folder);
            break;
        case TYPE_COMPUTER:
            wcscpy_s(result->typeName, 80, lc_str.computer);
            break;
        default: {
            wcscpy_s(result->typeName, 80, lc_str.file);
            wchar_t* ext = getFileExtension(path);
            
            if (ext) {
                if (wcsicmp(ext, L"exe") == 0) {
                    wcscpy_s(result->typeName, 80, lc_str.application);
                }
                else if (wcsicmp(ext, L"lnk") == 0) {
                    wcscpy_s(result->typeName, 80, lc_str.shortcut);
                }
                else {
                    wchar_t value[30] = {0};
                    strToUpper(ext, value);
                    swprintf_s(result->typeName, 80, lc_str.fmt_file, value);
                }
            }
            
            break;
        }
    }
}

static inline void makeDirs(wchar_t* path) {
    if (isPathExists(path)) return;
    wchar_t parentDir[MAX_PATH] = {0};
    getParentDirFromPath(path, parentDir);
    if (!isPathExists(parentDir)) makeDirs(parentDir);
    CreateDirectory(path, NULL);
}

static inline void joinPaths(wchar_t* pathA, wchar_t* pathB, wchar_t* result) {
    wmemset(result, L'\0', MAX_PATH);
    int pathLen = wcslen(pathA);
    wcscpy_s(result, MAX_PATH, pathA);
    
    if (result[pathLen-1] != L'\\') result[pathLen++] = L'\\';
    wcscpy_s(result + pathLen, MAX_PATH, pathB);
}

static inline void joinUnixPaths(char* pathA, char* pathB, char* result) {
    memset(result, 0, MAX_PATH);
    int pathLen = strlen(pathA);
    strcpy_s(result, MAX_PATH, pathA);
    
    if (result[pathLen-1] != '/') result[pathLen++] = '/';
    strcpy_s(result + pathLen, MAX_PATH, pathB);
}

static inline void clearDirectory(wchar_t* targetPath) {
    wchar_t path[MAX_PATH] = {0};
    wcscpy_s(path, MAX_PATH, targetPath);
    WIN32_FIND_DATA wfd = {0};
    if (!wcsstr(path, L"\\*")) wcscat_s(path, MAX_PATH, L"\\*");
    HANDLE handle = FindFirstFile(path, &wfd);

    if (handle != INVALID_HANDLE_VALUE) {
        do {
            if (wfd.cFileName[0] == L'.') continue;
            
            wchar_t fullPath[MAX_PATH] = {0};
            joinPaths(targetPath, wfd.cFileName, fullPath);
            bool isDir = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
            
            if (isDir) {
                clearDirectory(fullPath);
                RemoveDirectory(fullPath);
            }
            else DeleteFile(fullPath);
        }
        while (FindNextFile(handle, &wfd));
        FindClose(handle);
    }
}

static inline bool getCurrentISOPath(wchar_t* result) {
    wmemset(result, L'\0', MAX_PATH);
    int pathLen = MAX_PATH;
    HKEY hkey;
    if (RegOpenKey(HKEY_CURRENT_USER, L"SOFTWARE\\Winlator\\WFM\\CurrentISOPath", &hkey) == ERROR_SUCCESS) {
        RegQueryValue(hkey, NULL, result, (PLONG)&pathLen);
        RegCloseKey(hkey);
    }
    
    return pathLen != MAX_PATH;
}

#endif