#include "main.h"

static const wchar_t desktopName[] = L"Desktop";
static const wchar_t documentsName[] = L"Documents";
static const wchar_t computerName[] = L"Computer";

static wchar_t desktopPath[MAX_PATH];
static wchar_t personalPath[MAX_PATH];

bool isPathExists(wchar_t* path) {
  DWORD dwAttrib = GetFileAttributes(path);
  return (dwAttrib != INVALID_FILE_ATTRIBUTES && (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

wchar_t* getDesktopPath() {
	return desktopPath;
}

static struct FileNode* allocFileNode(wchar_t* name, enum FileType type) {
	struct FileNode* node = malloc(sizeof(struct FileNode));
	node->name = name;
	node->type = type;
	node->parent = NULL;
	node->sibling = NULL;
	node->children = NULL;
	node->hasChildDirs = false;
	return node;
}

int getChildNodeCount(struct FileNode* parent) {
	int count = 0;
	struct FileNode* child = parent->children;
	while (child) {
		count++;
		child = child->sibling;
	}
	return count;
}

void freeChildNodes(struct FileNode* parent) {
	struct FileNode* child = parent->children;
	while (child) {
		freeChildNodes(child);
		free(child);
		child = child->sibling;
	}
	parent->children = NULL;
}

void buildChildNodes(struct FileNode* parent, bool onlyDirs) {
	freeChildNodes(parent);
	
	struct FileNode* firstChild = NULL;
	struct FileNode* lastChild = NULL;	
	
	if (parent->type == TYPE_COMPUTER) {
		wchar_t drives[MAX_PATH];
		GetLogicalDriveStrings(MAX_PATH, drives);
		
		int i = 0;
		while (drives[i] != L'\0') {
			wchar_t* drive = &drives[i];
			i += wcslen(drive) + 1;
			if (!isPathExists(drive)) continue;

			wchar_t* name = malloc(3 * sizeof(wchar_t));
			name[0] = drive[0];
			name[1] = drive[1];
			name[2] = L'\0';
			
			struct FileNode* child = allocFileNode(name, TYPE_DRIVE);
			child->parent = parent;
			
			if (!firstChild) firstChild = child;
			if (lastChild) lastChild->sibling = child;
			lastChild = child;
		}
	}
	else {
		wchar_t path[MAX_PATH];
		getFileNodePath(parent, path);
		if (!isPathExists(path)) return;
		
		WIN32_FIND_DATA wfd;
		wcscat_s(path, MAX_PATH, L"\\*");
		HANDLE handle = FindFirstFile(path, &wfd);
		
		do {
			if (wcscmp(wfd.cFileName, L".") == 0 || wcscmp(wfd.cFileName, L"..") == 0 ||
			   (onlyDirs && (wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE))) continue; 
			
			if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) || (wfd.dwFileAttributes & FILE_ATTRIBUTE_ARCHIVE)) {
				enum FileType type = (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? TYPE_DIR : TYPE_FILE;
				
				wchar_t* name = wcsdup(wfd.cFileName);
				struct FileNode* child = allocFileNode(name, type);
				child->parent = parent;
				
				if (!firstChild) firstChild = child;
				if (lastChild) lastChild->sibling = child;
				lastChild = child;				
			}				
		}
		while (FindNextFile(handle, &wfd) != 0);
	}
	
	parent->children = firstChild;
}

void checkIfNodesHasChildDirs(struct FileNode* node, bool deep) {
	if (!node) return;
	wchar_t path[MAX_PATH];
	
	struct FileNode* parent = node;
	while (parent) {
		parent->hasChildDirs = false;
		if (parent->type == TYPE_COMPUTER) {
			parent->hasChildDirs = true;
		}
		else {
		    getFileNodePath(parent, path);
			if (!isPathExists(path)) continue;
			
			WIN32_FIND_DATA wfd;
			wcscat_s(path, MAX_PATH, L"\\*");
			HANDLE handle = FindFirstFile(path, &wfd);

			if (handle != INVALID_HANDLE_VALUE) {
				do {
					if (wcscmp(wfd.cFileName, L".") == 0 || wcscmp(wfd.cFileName, L"..") == 0) continue;
					if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						parent->hasChildDirs = true;
						break;
					}				
				}
				while (FindNextFile(handle, &wfd) != 0);
			}
		}
		
		if (deep) checkIfNodesHasChildDirs(parent->children, deep);
		parent = parent->sibling;
	}
}

static void freeCurrPathFileNode() {
	struct FileNode* node = currPathFileNode;
	while (node) {
		struct FileNode* parent = node->parent;
		freeChildNodes(node);
		free(node);
		node = parent;
	}
	currPathFileNode = NULL;
}

void setCurrPathFileNode(struct FileNode* node) {
	struct FileNode* currNode = node;
	int count = 0;
	while (currNode) {
		count++;
		currNode = currNode->parent;
	}
	
	struct FileNode* nodes[count];
	int i = 0;
	currNode = node;
	while (currNode) {
		nodes[i++] = currNode;
		currNode = currNode->parent;
	}	
	
	for (int i = count-1; i >= 0; i--) {
		wchar_t* name = wcsdup(nodes[i]->name);
		struct FileNode* newNode = allocFileNode(name, nodes[i]->type);
		newNode->parent = currNode;
		currNode = newNode;
	}
	
	freeCurrPathFileNode();
	currPathFileNode = currNode;
}

void setCurrPathFromString(wchar_t* path) {
	if (!isPathExists(path)) return;
	wchar_t tmp[MAX_PATH];
	wcscpy_s(tmp, MAX_PATH, path);

	struct FileNode* currNode = allocFileNode(computerName, TYPE_COMPUTER);
	
	wchar_t* token = wcstok(tmp, L"\\");
	int i = 0;
	while (token != NULL) {
		wchar_t* name = wcsdup(token);
		enum FileType type = i++ == 0 ? TYPE_DRIVE : TYPE_DIR;
		struct FileNode* newNode = allocFileNode(name, type);
		newNode->parent = currNode;
		currNode = newNode;
		token = wcstok(NULL, L"\\");
	}
	
	freeCurrPathFileNode();
	currPathFileNode = currNode;
}

void initFileNodes() {
	struct FileNode* desktopNode = allocFileNode(desktopName, TYPE_DESKTOP);
	struct FileNode* documentsNode = allocFileNode(documentsName, TYPE_PERSONAL);
	struct FileNode* computerNode = allocFileNode(computerName, TYPE_COMPUTER);
	
	desktopNode->sibling = documentsNode;
	documentsNode->sibling = computerNode;
	buildChildNodes(computerNode, true);
	
	treeFileNode = desktopNode;
	
    SHGetFolderPath(NULL, CSIDL_PERSONAL, NULL, SHGFP_TYPE_CURRENT, personalPath);		
    SHGetFolderPath(NULL, CSIDL_DESKTOP, NULL, SHGFP_TYPE_CURRENT, desktopPath);
	
	currPathFileNode = NULL;
	checkIfNodesHasChildDirs(treeFileNode, true);
	setCurrPathFileNode(computerNode);
}

int getFileNodePath(struct FileNode* node, wchar_t* path) {
	struct FileNode* currNode = node;	
	wcscpy_s(path, MAX_PATH, L"");
	wchar_t tmp[MAX_PATH];
	int count = 0;
	
	while (currNode) {
		wchar_t* filename = NULL;
		
		switch (currNode->type) {
			case TYPE_DESKTOP:
				filename = desktopPath;
				break;
			case TYPE_PERSONAL:
				filename = personalPath;
				break;
			case TYPE_FILE:
			case TYPE_DIR:
			case TYPE_DRIVE:
				filename = currNode->name;				
				break;
		}
		
		if (filename && filename[0] != '\0') {
			if (count > 0) {
				swprintf_s(tmp, MAX_PATH, L"%ls\\%ls", filename, path);
				wcscpy_s(path, MAX_PATH, tmp);
			}
			else wcscpy_s(path, MAX_PATH, filename);
			count++;		
		}
		
		currNode = currNode->parent;
	}

	return count;
}