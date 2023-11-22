#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <windows.h>
#include <commctrl.h>
#include <wingdi.h>
#include <wchar.h>
#include <strsafe.h>
#include <shlobj.h>
#include <process.h>

#include "resource.h"
#include "content_view.h"
#include "toolbar.h"
#include "navbar.h"
#include "treeview.h"
#include "sizebar.h"
#include "statusbar.h"
#include "file_node.h"
#include "file_actions.h"
#include "file_utils.h"
#include "input_dialog.h"

HINSTANCE globalHInstance;
HWND hwndMain;

//#define DEBUG

#ifdef DEBUG
void debug_printf(const char *fmt, ...);
#endif

void navigateToFileNode(struct FileNode* node);
void navigateUp();
void navigateRefresh();
void openFileNode(struct FileNode* node);
void GetWindowRectInParent(HWND hwnd, RECT* rect);
void resizeControls();

#endif