#ifndef TREEVIEW_H
#define TREEVIEW_H

HWND hwndTreeview;

LRESULT treeviewNotify(NMHDR* nmhdr);
void createTreeview();

#endif