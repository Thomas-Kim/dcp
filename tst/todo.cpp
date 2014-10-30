#include "todo.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include <set>
using std::set;
#include <string>
using std::string;

int main() {
    set<string> paths;
    DIR* dir = opendir(".");
    if (dir == NULL) {
        perror(".");
        return errno;
    }
    todo_init();
    struct dirent* ent;
    do {
        ent = readdir(dir);
        if (ent == NULL) {
            if (errno) {
                perror("readdir");
                return errno;
            }
        } else {
            string put(ent->d_name);
            paths.insert(put);
            size_t sz = put.length() + 1;
            char* copy = (char*) malloc(sz);
            strncpy(copy, ent->d_name, sz);
            add_path(copy);
        }
    } while(ent);
    set<string> popped;
    const char* pop;
    while (1) {
        struct stat info;
        pop = get_next(&info);
        if (pop) {
            popped.insert(string(pop));
            free((char*)pop);
        } else {
            break;
        }
    }
    assert(popped == paths);
}
