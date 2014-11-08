#include "directory.h"
#include <sched.h>

#include <set>
using std::set;
#include <string>
using std::string;

set<string> in;
set<string> out;
volatile int finished = 0;

extern "C"
void add_path(const char* path) {
    out.insert(string(path));
}

extern "C"
void finish() {
    finished = 1;
}

int main() {
    directory(".");
    while (!finished) {
        sched_yield();
    }
    return 0;
}
