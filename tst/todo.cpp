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
#include <vector>
using std::vector;

// in actual use there will be at most one producer per directory but this helps protect correctness
void* put_path(void* arg) {
    const char* str = (const char*) arg;
    add_path(str);
    return NULL;
}
set<string> paths;
void init() {
    DIR* dir = opendir(".");
    if (dir == NULL) {
        perror(".");
        exit(errno);
    }
    struct dirent* ent;
    do {
        ent = readdir(dir);
        if (ent == NULL) {
            if (errno) {
                perror("readdir");
                exit(errno);
            }
        } else {
            string put(ent->d_name);
            paths.insert(put);
        }
    } while(ent);
    closedir(dir);
}
void trial() {
    vector<pthread_t> children;
    todo_init();
    for (auto it = paths.cbegin(); it != paths.cend(); it++) {
        size_t sz = it->length() + 1;
        char* copy = (char*) malloc(sz);
        strncpy(copy, it->c_str(), sz);
        pthread_t child;
        pthread_create(&child, NULL, put_path, copy);
        children.push_back(child);
    }
    for (auto it = children.cbegin(); it != children.cend(); it++) {
        pthread_join(*it, NULL);
    }
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
    todo_destroy();
}

int main() {
    init();
    for (size_t i = 0; i < 500; i++) {
        trial();
    }
    return 0;
}
