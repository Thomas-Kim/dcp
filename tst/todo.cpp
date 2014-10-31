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
void trial() {
    set<string> paths;
    DIR* dir = opendir(".");
    if (dir == NULL) {
        perror(".");
        exit(errno);
    }
    todo_init();
    struct dirent* ent;
    vector<pthread_t> children;
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
            pthread_t child;
            size_t sz = put.length() + 1;
            char* copy = (char*) malloc(sz);
            strncpy(copy, ent->d_name, sz);
            pthread_create(&child, NULL, put_path, copy);
            children.push_back(child);
        }
    } while(ent);
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
    closedir(dir);
    todo_destroy();
}

int main() {
    for (size_t i = 0; i < 500; i++) {
        trial();
    }
    return 0;
}
