#include "equals.h"

#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    const char* binFile = "tst/bin/equals";
    const char* txtFile = "tst/equals.c";
    assertFilesEqual(binFile, binFile);
    assertFilesEqual(txtFile, txtFile);
    const char* dirOne = "tst/benchmarks";
    assertDirsEqual(dirOne, dirOne);
    const char* dirTwo = "tst/tmp0";
    const char* dirThree = "tst/tmp1";
    mode_t mode = 0755;
    if (mkdir(dirTwo, mode)) {
        perror(dirTwo);
    }
    if (mkdir(dirThree, mode)) {
        perror(dirThree);
    }
    assertDirsEqual(dirTwo, dirThree);
    if (rmdir(dirTwo)) {
        perror(dirTwo);
    }
    if (rmdir(dirThree)) {
        perror(dirThree);
    }
    return 0;
}
