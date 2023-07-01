#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "libs.h"
#include <string.h>

void firstIteration(short* memory, CodeNode* code, LabelNode* labels) {
    CodeNode* temp_code;
    char* label_flag;


    temp_code = code;

    while(temp_code) {
        if(strstr(temp_code->code_row, ":")) {
            printf("%d\n", isLabel(temp_code->code_row, strstr(temp_code->code_row, ":")));
        }



        temp_code = temp_code->next;
    }
}


bool isLabel(char* word){
    bool flag = false;
    int i = 0;
    if (!isalpha(word[i]))
    {
        return flag;
    }
    
    for (; word[i] != '\0'; i++)
    {
        if (!isalpha(word[i]) && !isdigit(word[i]))
        {
            return flag;
        }
        
    }
    if (word[i-1] == ':')
    {
        flag = true;
    }

    return flag;
}