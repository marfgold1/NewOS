#include "../header/shell.h"

bool checkIsFile(char* path, byte cdir) {
    if (
        cdir == FS_NODE_P_IDX_ROOT || // is a root or a directory
        node_fs_buffer.nodes[cdir].sector_entry_index == FS_NODE_S_IDX_FOLDER
    ){
        printStringColored("Not a file: ", COLOR_LIGHT_RED);
        printStringColored(path, COLOR_LIGHT_BLUE); endl; // is not a file
        return false;
    }
    return true;
}

bool checkIsDirectory(char* path, byte cdir) {
    if (
        cdir != FS_NODE_P_IDX_ROOT && // if not root and is a file
        node_fs_buffer.nodes[cdir].sector_entry_index != FS_NODE_S_IDX_FOLDER
    ){
        printStringColored("Not a directory: ", COLOR_LIGHT_RED);
        printStringColored(path, COLOR_LIGHT_BLUE); endl; // is not a directory
        return false;
    }
    return true;
}

bool checkIsExist(char* path, byte cdir) {
    if (cdir == IDX_NODE_UNDEF) {
        printStringColored("No such file or directory: ", COLOR_LIGHT_RED);
        printStringColored(path, COLOR_LIGHT_BLUE); endl; // no such file or directory
        return false;
    }
    return true;
}

void __applyPath(char* path, int dot_cnt, int len) {
    if (arg_cdir == IDX_NODE_UNDEF) { // if cdir already undef, last set to undef
        arg_ldir = IDX_NODE_UNDEF;
    } else if (dot_cnt == 2 && len == 2) { // go up
        if (arg_cdir != FS_NODE_P_IDX_ROOT) { // if not root, we can go up
            arg_ldir = arg_cdir;
            arg_cdir = node_fs_buffer.nodes[arg_cdir].parent_node_index;
        }
    } else if (len > 0 && !(len == 1 && name_temp[0] == '.')) { // at least len > 0 and not .
        name_temp[len] = nullt;
        arg_ldir = arg_cdir;
        arg_cdir = getNodeIdxFromParent(&node_fs_buffer, name_temp, arg_cdir);
    }
}

void parsePathArg(char* path) {
    byte dot_cnt = 0;
    int i = 0, len = 0;
    readNodeFs(&node_fs_buffer);
    arg_cdir = current_dir;
    arg_ldir = current_dir;
    if (path[i] == '/' || path[i] == '\\') {
        arg_cdir = FS_NODE_P_IDX_ROOT;
        arg_ldir = arg_cdir;
        i++;
    }
    while (path[i] != '\0') {
        if (path[i] == '.') {
            dot_cnt++;
            if (len < 13)
                name_temp[len++] = path[i];
        } else if (
            (path[i] == '/' || path[i] == '\\') // start to parse a new dir
        ) {
            __applyPath(path, dot_cnt, len);
            len = 0;
            dot_cnt = 0;
        } else {
            if (len < 13)
                name_temp[len++] = path[i];
        }
        i++;
    }
    if (len > 0) // remains.
        __applyPath(path, dot_cnt, len);
}

void parseArgs(char* input) {
    int i = 0, k = 0;
    bool strflag = false;
    arg_count = 0;
    while (input[i] != '\0') {
        if (arg_count > MAX_ARGS) // don't leak it
            return;
        if (input[i] == '"') {
            strflag = !strflag; // toggle string
        } else if (input[i] == ' ' && !strflag) {
            if (k > 0) { // ignore additional space
                args[arg_count++][k] = nullt; // insert null character
                k = 0;
            }
        } else {
            args[arg_count][k++] = input[i];
        }
        i++;
    }
    if (strflag)
        printChar('"'); // explicitly close the string
    if (k > 0 && args[arg_count][k-1] != nullt) { // insert null character
        args[arg_count++][k] = nullt;
    }
}