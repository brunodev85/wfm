#ifndef FILE_NODE_H
#define FILE_NODE_H

enum FileType {
    TYPE_DIR,
    TYPE_FILE,
    TYPE_DRIVE,
    TYPE_DESKTOP,
    TYPE_PERSONAL,
    TYPE_COMPUTER
};

struct FileNode {
    wchar_t* name;
    enum FileType type;
    struct FileNode* parent;
    struct FileNode* sibling;
    struct FileNode* children;
    bool hasChildDirs;
};

void initFileNodes();
bool isPathExists(wchar_t* path);
wchar_t* getDesktopPath();
void setCurrPathFileNode(struct FileNode* node);
void setCurrPathFromString(wchar_t* path);
int getChildNodeCount(struct FileNode* parent);
int getFileNodePath(struct FileNode* node, wchar_t* path);
void buildChildNodes(struct FileNode* parent, bool onlyDirs);
void checkIfNodesHasChildDirs(struct FileNode* node, bool deep);
void freeChildNodes(struct FileNode* parent);

#endif