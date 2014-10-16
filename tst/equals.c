#include "equals.h"
int main() {
    const char* binFile = "tst/bin/equals";
    const char* txtFile = "tst/equals.c";
    assertFilesEqual(binFile, binFile);
    assertFilesEqual(txtFile, txtFile);
    // TODO verify that directories are equal
    return 0;
}
