#define IDI_MAIN 101
#define IDD_ABOUT 102
#define IDI_GO 103
#define IDI_REFRESH 104
#define IDI_NAV_ARROW 105
#define IDD_INPUT 106
#define IDC_LABEL 107
#define IDC_EDIT 108
#define IDC_APP_NAME 109
#define IDC_APP_VERSION 110
#define IDC_APP_DEV_NAME 111
#define IDD_FILE_ACTION 112
#define IDC_PRELOADER 113
#define IDI_SEARCH 114
#define IDI_CANCEL 115
#define IDI_UP 116
#define IDI_COPY 117
#define IDI_CUT 118
#define IDI_PASTE 119
#define IDI_DELETE 120
#define IDI_NEW_FOLDER 121
#define IDI_NEW_FILE 122

#define IDI_PRELOADER_1 201
#define IDI_PRELOADER_2 202
#define IDI_PRELOADER_3 203
#define IDI_PRELOADER_4 204
#define IDI_PRELOADER_5 205
#define IDI_PRELOADER_6 206
#define IDI_PRELOADER_7 207
#define IDI_PRELOADER_8 208

#define ID_EDIT_CUT 301
#define ID_EDIT_COPY 302
#define ID_EDIT_PASTE 303
#define ID_VIEW_LARGEICONS 304
#define ID_VIEW_SMALLICONS 305
#define ID_VIEW_LIST 306
#define ID_VIEW_DETAILS 307
#define ID_HELP_ABOUT 308
#define ID_FILE_EXIT 309
#define ID_EDIT_PASTE_SHORTCUT 310
#define ID_EDIT_SELECT_ALL 311

#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif

#define APP_NAME L"Winlator File Manager"
#define APP_VERSION L"1.5"
#define APP_DEV_NAME L"BrunoSX"

#ifndef RESOURCE_H
#define RESOURCE_H

struct LC_STR {
    wchar_t* app_name;
    wchar_t* app_version;
    wchar_t* app_dev_name;
    wchar_t* application;
    wchar_t* shortcut;
    wchar_t* file;
    wchar_t* folder;
    wchar_t* local_drive;
    wchar_t* cd_drive;
    wchar_t* computer;
    wchar_t* desktop;
    wchar_t* documents;
    wchar_t* exit;
    wchar_t* edit;
    wchar_t* cut;
    wchar_t* copy;
    wchar_t* paste;
    wchar_t* paste_shortcut;
    wchar_t* select_all;
    wchar_t* view;
    wchar_t* large_icons;
    wchar_t* small_icons;
    wchar_t* list;
    wchar_t* details;
    wchar_t* help;
    wchar_t* about;
    wchar_t* ok;
    wchar_t* cancel;
    wchar_t* loading;
    wchar_t* open;
    wchar_t* create_shortcut;
    wchar_t* delete;
    wchar_t* rename;
    wchar_t* new_folder;
    wchar_t* new_file;
    wchar_t* items;
    wchar_t* load_iso_image;
    wchar_t* unload_iso_image;
    wchar_t* no_media;
    wchar_t* alert;
    wchar_t* enter_folder_name;
    wchar_t* enter_file_name;
    wchar_t* enter_new_name;
    wchar_t* name;
    wchar_t* type;
    wchar_t* size;
    wchar_t* date;
    wchar_t* path;
    wchar_t* deleting_files;
    wchar_t* copying_files;
    wchar_t* moving_files;
    wchar_t* extracting_files;
    wchar_t* confirm_delete;
    wchar_t* confirm_exit;
    wchar_t* search;
    wchar_t* up;
    
    wchar_t* fmt_file;
    
    wchar_t* msg_invalid_iso_image_file;
    wchar_t* msg_deleting_files;
    wchar_t* msg_copying_files;
    wchar_t* msg_moving_files;
    wchar_t* msg_extracting_files;
    wchar_t* msg_cancel_file_operation;
    wchar_t* msg_confirm_delete_item;
    wchar_t* msg_confirm_delete_multiple_items;
    wchar_t* msg_confirm_exit_app;
};

extern struct LC_STR lc_str;

#include "locale/strings_en.h"
#include "locale/strings_pt.h"
#include "locale/strings_ru.h"

#define STARTS_WITH(a, b) (a[0] == b[0] && a[1] == b[1])

static inline void loadLCStrings(wchar_t* localeName) {
    if (STARTS_WITH(localeName, L"pt")) {
        loadStrings_pt();
    }
    else if (STARTS_WITH(localeName, L"ru")) {
        loadStrings_ru();
    }    
    else loadStrings_en();
}

#undef STARTS_WITH

#endif