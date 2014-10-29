#include "todo.h"

#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

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
    struct dirent* ent;
    do {
        ent = readdir(dir);
        if (ent == NULL) {
            if (errno) {
                perror("readdir");
                return errno;
            }
        } else {
            paths.insert(string(ent->d_name));
            add_path(ent->d_name);
        }
    } while(ent);
    set<string> popped;
    const char* pop;
    do {
        struct stat info;
        pop = get_next(&info);
        if (pop) {
            popped.insert(string(pop));
        }
    } while (pop);
    assert(popped == paths);
}
