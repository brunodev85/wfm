#ifndef STRINGS_EN_H
#define STRINGS_EN_H

#include "resource.h"

static inline void loadStrings_en() {
    lc_str.app_name = APP_NAME;
    lc_str.app_version = L"Version " APP_VERSION;
    lc_str.app_dev_name = L"by " APP_DEV_NAME;
    lc_str.application = L"Application";
    lc_str.shortcut = L"Shortcut";
    lc_str.file = L"File";
    lc_str.folder = L"Folder";
    lc_str.local_drive = L"Local Drive";
    lc_str.cd_drive = L"CD Drive";
    lc_str.computer = L"Computer";
    lc_str.desktop = L"Desktop";
    lc_str.documents = L"Documents";
    lc_str.exit = L"Exit";
    lc_str.edit = L"Edit";
    lc_str.cut = L"Cut";
    lc_str.copy = L"Copy";
    lc_str.paste = L"Paste";
    lc_str.paste_shortcut = L"Paste Shortcut";
    lc_str.select_all = L"Select All";
    lc_str.view = L"View";
    lc_str.large_icons = L"Large Icons";
    lc_str.small_icons = L"Small Icons";
    lc_str.list = L"List";
    lc_str.details = L"Details";
    lc_str.help = L"Help";
    lc_str.about = L"About";
    lc_str.ok = L"OK";
    lc_str.cancel = L"Cancel";
    lc_str.loading = L"Loading...";
    lc_str.open = L"Open";
    lc_str.create_shortcut = L"Create Shortcut";
    lc_str.delete = L"Delete";
    lc_str.rename = L"Rename";
    lc_str.new_folder = L"New Folder";
    lc_str.new_file = L"New File";
    lc_str.items = L"Items";
    lc_str.load_iso_image = L"Load ISO Image";
    lc_str.unload_iso_image = L"Unload ISO Image";
    lc_str.no_media = L"No media";
    lc_str.alert = L"Alert";
    lc_str.enter_folder_name = L"Enter folder name:";
    lc_str.enter_file_name = L"Enter file name:";
    lc_str.enter_new_name = L"Enter new name:";
    lc_str.name = L"Name";
    lc_str.type = L"Type";
    lc_str.size = L"Size";
    lc_str.date = L"Date";
    lc_str.path = L"Path";
    lc_str.deleting_files = L"Deleting files";
    lc_str.copying_files = L"Copying files";
    lc_str.moving_files = L"Moving files";
    lc_str.extracting_files = L"Extracting files";
    lc_str.confirm_delete = L"Confirm Delete";
    lc_str.confirm_exit = L"Confirm Exit";
    lc_str.search = L"Search";
    lc_str.up = L"Up";
    
    lc_str.fmt_file = L"%ls File";
    
    lc_str.msg_invalid_iso_image_file = L"Invalid ISO Image file!";
    lc_str.msg_deleting_files = L"Deleting files, please wait...";
    lc_str.msg_copying_files = L"Copying files, please wait...";
    lc_str.msg_moving_files = L"Moving files, please wait...";
    lc_str.msg_extracting_files = L"Extracting files, please wait...";
    lc_str.msg_cancel_file_operation = L"Do you want to cancel the operation?";
    lc_str.msg_confirm_delete_item = L"Are you sure you want to delete \"%ls\"?";
    lc_str.msg_confirm_delete_multiple_items = L"Are you sure you want to delete %d items?";
    lc_str.msg_confirm_exit_app = L"Are you sure you want to exit?";
}

#endif