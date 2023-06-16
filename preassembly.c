#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "headers/nodes.h"
#include "headers/constants.h"
#include "headers/errors.h"

CodeNode* createLinkedListFromFile(FILE* file, Error* error);



int main () {
    FILE* file;
    CodeNode* cn;
    Error* error;

    file = fopen("test", "r");
    printf("Test");
    cn = createLinkedListFromFile(file, error);
    while (cn) {
        printf("%s\n", cn->code_row);
        cn = cn->next;
    }
    return 1;
}

/**
 * @brief Create a Linked List From File object
 * 
 * @param file 
 * @return CodeNode* 
 */
CodeNode* createLinkedListFromFile(FILE* file, Error* error) {
    char buffer[MAX_LINE_LENGTH + 1];
    CodeNode *head = NULL, *temp = NULL, *node = NULL;
    
    getLine(buffer, error, file);

    while(fgets(buffer, MAX_LINE_LENGTH + 1, file) != NULL) {
        /*Create a new node*/
        node = (CodeNode*)malloc(sizeof(CodeNode));
        if(node == NULL) {
            printf("Error allocating memory for new node.\n");
            return NULL;
        }
        
        /* Copy the string from buffer to the new node*/
        strncpy(node->code_row, buffer, MAX_LINE_LENGTH + 1);
        node->next = NULL;
        
        /* If this is the first node, it is the head of the list*/
        if(head == NULL) {
            head = node;
        } else {
            /* Otherwise, add the new node to the end of the list*/
            temp->next = node;
        }
        
        /* Move the temporary pointer to the new node*/
        temp = node;
    }
    
    return head;
}

/**
 * @brief frees all the memmory allocated to the linked list ( code rows)
 * 
 * @param head 
 */
void freeLinkedList(CodeNode* head) {
    CodeNode* tmp;

    while (head != NULL) {
        tmp = head;
        head = head->next;
        free(tmp);
    }
}

int getLine(char* line, Error* error, FILE* file) {
    char x; /*current symbol in the input stream*/
    int i = 0;
    clean_line(line);
    while ((x = fgetc(file)) != '\n' && x != EOF) {
        if (i == MAX_LINE_LENGTH) {
            *error = ERROR_MAXED_OUT_LINE_LENGTH;
            /*skipping to the next line*/
            while ((x = fgetc(file)) != '\n' && x != EOF) {
                continue;
            }
            return i;
        }
        /*substitution of whitespaces instead of tabs*/
        x = (x == '\t') ? ' ' : x;
        /*removing whitespaces at the beggining of the line*/
        if (i == 0 && x == ' ') {
            continue;
        }
        /*removing of duplications of whitespaces*/
        if ((i != 0) && line[i-1] == ' ' && (x == ' ')) {
            continue;
        }
        /*putting a char to the string*/
        line[i++] = x;
    }
    /*The case where the line is empty*/
    if (i == 0 && x == '\n') {
        line[0] = '\n';
        return 1;
    }
    return i;
}

void clean_line(char* line) {
    int i;
    for (i = 0; i < MAX_LINE_LENGTH; i++) {
        line[i] = '\0';
    }
}

MacroNode* scanCodeForMacros(CodeNode* code) {

}