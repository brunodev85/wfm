#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <wchar.h>
#include <math.h>

#include "file_utils.h"

void formatFileSize(uint64_t size, wchar_t* formattedSize) {
	static const wchar_t units[5][11] = {L"bytes", L"KB", L"MB", L"GB", L"TB"};
	
	if (size > 0) {
		int digitGroups = (int)(log10(size) / log10(1024));
		swprintf_s(formattedSize, 32, L"%.2f %ls", size / pow(1024, digitGroups), units[digitGroups]);
		
		wchar_t *lastDot = wcsrchr(formattedSize, '.');
		if (lastDot != NULL) {
			int offset = lastDot - formattedSize;
			if (formattedSize[offset+1] == '0' && formattedSize[offset+2] == '0') {
				int len = wcslen(formattedSize);
				for (int i = 0; i < len-3; i++) formattedSize[offset+i] = formattedSize[offset+i+3];
			}
		}
	}
	else wcscpy_s(formattedSize, 32, L"0 bytes");
}

void getParentDirFromPath(wchar_t* path, wchar_t* result) {
	wchar_t* lastSlash = wcsrchr(path, '\\');
	int len = lastSlash != NULL ? lastSlash - path + 1 : 1;

	memcpy(result, path, (len - 1) * sizeof(wchar_t));
	result[len-1] = '\0';
}

void getBasenameFromPath(wchar_t* path, wchar_t* result, bool removeExt) {
	wchar_t* lastSlash = wcsrchr(path, '\\');
	int offset = lastSlash != NULL ? lastSlash - path + 1 : 0;
	int len = (wcslen(path) - offset) + 1;
	
	memcpy(result, path + offset, len * sizeof(wchar_t));
	result[len-1] = '\0';
	
	if (removeExt) {
		wchar_t *lastDot = wcsrchr(result, '.');
		offset = lastDot - result;
		result[offset] = '\0';		
	}
}