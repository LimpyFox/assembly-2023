#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "libs.h"

int main (int argc, char** argv) {
    CodeNode* code;
    Error error = NO_ERROR;
    LabelNode* labels = NULL;
    short memory[MAX_MEMORY_SIZE];
    int DC, IC;

    code = preproccessor(argv[1], &error);

    if (error != NO_ERROR) {
        return 0;
    }
    firstIteration(memory, code, &labels, &DC, &IC, &error);
    if (error != NO_ERROR) {
        return 0;
    }
    secondIteration(memory, code, labels, &DC, &IC, &error, argv[1]);
    if (error != NO_ERROR) {
        return 0;
    }

    return 1;
}