#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "libs.h"

static const Command commands[MAX_COMMAND_LENGTH] = {
    {"mov",  0x0, 2, {1, 1, 1}, {0, 1, 1}},
    {"cmp",  0x1, 2, {1, 1, 1}, {1, 1, 1}},
    {"add",  0x2, 2, {1, 1, 1}, {0, 1, 1}},
    {"sub",  0x3, 2, {1, 1, 1}, {0, 1, 1}},
    {"not",  0x4, 1, {0, 0, 0}, {0, 1, 1}},
    {"clr",  0x5, 1, {0, 0, 0}, {0, 1, 1}},
    {"lea",  0x6, 2, {0, 1, 0}, {0, 1, 1}},
    {"inc",  0x7, 1, {0, 0, 0}, {0, 1, 1}},
    {"dec",  0x8, 1, {0, 0, 0}, {0, 1, 1}},
    {"jmp",  0x9, 1, {0, 0, 0}, {0, 1, 1}},
    {"bne",  0xA, 1, {0, 0, 0}, {0, 1, 1}},
    {"red",  0xB, 1, {0, 0, 0}, {0, 1, 1}},
    {"prn",  0xC, 1, {0, 0, 0}, {1, 1, 1}},
    {"jsr",  0xD, 1, {0, 0, 0}, {0, 1, 1}},
    {"rts",  0xE, 0, {0, 0, 0}, {0, 0, 0}},
    {"stop", 0xF, 0, {0, 0, 0}, {0, 0, 0}}
};

static const char base64_chars[64] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

void firstIteration(short* memory, int* memory_idx, CodeNode* code, LabelNode** labels, int* DC, int* IC, bool* is_print, Error* error) {
    bool is_first_itteration_flag = true;
    bool stop_flag = false; /* gives information , whether the code already got to a line with "stop" command, or not*/
    CodeNode* temp_code;
    LabelNode* temp_label_node;
    bool label_flag = false;
    int data_memory_idx = DEFAULT_MEMORY_INDEX;
    int operand_num = 0;
    int i;
    int def_extern_mem = DEFAULT_EXTERN_MEMORY;
    int num_tokens = DEFAULT_VALUE;
    int token_idx = DEFAULT_VALUE;
    short binary_word;
    int L = DEFAULT_VALUE;
    int num_line = STARTING_LINE;
    short data_memory[MAX_MEMORY_SIZE];
    int char_index = 1;
    int token_len = 0;
    bool end_of_string_flag = false;
    char** tokens = allocateMemory(MAX_TOKENS * sizeof(char *), is_print, error);
    allocateMemoryTokens(tokens, is_print, error);

    if (*error == ERROR_MEMORY_ALLOCATION) return;

    *DC = *IC = DEFAULT_VALUE;
    cleanMemory(memory);
    cleanMemory(data_memory);
    temp_code = code;
    while(temp_code) {
        token_idx = DEFAULT_VALUE;
        end_of_string_flag = false;
        char_index = 1;
        label_flag = false;

        if (temp_code->code_row[FIRST_CHARACTER] == '\n' || temp_code->code_row[FIRST_CHARACTER] == '\0' || temp_code->code_row[FIRST_CHARACTER] == '\r' || temp_code->code_row[FIRST_CHARACTER] == ';') {
            temp_code = temp_code->next;
            num_line++;
            continue;
        }
        
        tokenizeInput(temp_code->code_row, tokens, &num_tokens, is_print, error);
        if (*error != NO_ERROR) {
            freeMemory(tokens, code, NULL, NULL);
            return;
        }

        if(isLabel(tokens[token_idx], true)) {
            label_flag = true;
            token_idx++;
        }

        switch (getDotType(tokens[token_idx], error)) {
            case DOT_DATA:
                if (label_flag) {
                    insertNewLabel(labels, removeColon(tokens[token_idx-1]), LABEL_TYPE_DATA, DC, is_print, error, is_first_itteration_flag);
                    if (*error == ERROR_MEMORY_ALLOCATION) {
                        handleError(error, num_line, is_print);
                        return;
                    }
                    if (*error == ERROR_DUPLICATE_LABEL)
                    {
                        handleError(error, num_line, is_print);
                        *error = NO_ERROR;
                        nextLine(&temp_code, &num_line);
                        continue;
                    }
                }
                if (checkDataLine(tokens, num_tokens, label_flag, error)) {
                    token_idx++;
                    for (i = token_idx; i < num_tokens; i += 2) {
                        pushToMemory(&data_memory_idx, data_memory, atoi(tokens[i]), error, num_line, is_print);
                        if (*error == ERROR_MAXED_OUT_MEMORY) return;
                        (*DC)++;
                    }
                }
                /*errr handaling*/
                if (*error != NO_ERROR) {
                    handleError(error, num_line, is_print);
                    *error = NO_ERROR;
                    nextLine(&temp_code, &num_line);
                    continue;
                }
                break;
            case DOT_STRING:
                if (label_flag) {
                    insertNewLabel(labels, removeColon(tokens[token_idx-1]), LABEL_TYPE_DATA, DC, is_print, error, is_first_itteration_flag);
                    if (*error == ERROR_MEMORY_ALLOCATION) {
                            handleError(error, num_line, is_print);
                            return;
                    }
                    if (*error == ERROR_DUPLICATE_LABEL)
                    {
                        handleError(error, num_line, is_print);
                        *error = NO_ERROR;
                        nextLine(&temp_code, &num_line);                        
                        continue;
                    }                    
                }
                if (checkDataLine(tokens, num_tokens, label_flag, error)) {
                    token_idx++;
                    for (; token_idx < num_tokens; token_idx++)
                    {
                        token_len = strlen(tokens[token_idx])+1;
                        for (; char_index < token_len; char_index++) {
                            if (tokens[token_idx][char_index] == '"'){
                                end_of_string_flag = true;
                                break;
                            }
                            else if (tokens[token_idx][char_index] == '\0')
                            {
                                tokens[token_idx][char_index] = ' ';
                            }
                            pushToMemory(&data_memory_idx, data_memory, tokens[token_idx][char_index], error, num_line, is_print);
                            if (*error == ERROR_MAXED_OUT_MEMORY) return;
                            (*DC)++;
                        }
                        if (end_of_string_flag) break;
                        char_index = 0;
                    }
                    pushToMemory(&data_memory_idx, data_memory, '\0', error, num_line, is_print);
                    if (*error == ERROR_MAXED_OUT_MEMORY) return;
                    (*DC)++;
                    /*
                    for (i = 1; i < (strlen(tokens[token_idx])-1); i++) {
                        pushToMemory(&data_memory_idx, data_memory, tokens[token_idx][i], error, num_line, is_print);
                        if (*error == ERROR_MAXED_OUT_MEMORY) return;
                        (*DC)++;
                    }
                    pushToMemory(&data_memory_idx, data_memory, '\0', error, num_line, is_print);
                    if (*error == ERROR_MAXED_OUT_MEMORY) return;
                    (*DC)++;
                    */


                }
                /*handlaing Error*/
                if (*error != NO_ERROR) {
                    handleError(error, num_line, is_print);
                    *error = NO_ERROR;
                    nextLine(&temp_code, &num_line);                    
                    continue;
                }
                break;
            case DOT_EXTERN:
                for (i = 1; i < num_tokens; i++) {
                    if (isLabel(tokens[i], false)) {
                        insertNewLabel(labels, tokens[i], LABEL_TYPE_EXTERNAL, &def_extern_mem, is_print, error, is_first_itteration_flag);
                        if (*error == ERROR_MEMORY_ALLOCATION) return;
                        if (*error == ERROR_DUPLICATE_LABEL)
                        {
                            /*handleError(error, num_line, is_print);*/
                            *error = NO_ERROR;
                            nextLine(&temp_code, &num_line);                            
                            continue;
                        }                        
                    }
                    else {
                        break;
                    }
                }
                break;
            case DOT_ENTRY:
                break;
            case DOT_OTHER:
                if (stop_flag) {
                    *error = ERROR_CODE_AFTER_STOP;
                    handleError(error, num_line, is_print);
                }
                if (label_flag) {
                    insertNewLabel(labels, removeColon(tokens[token_idx-1]), LABEL_TYPE_CODE, IC, is_print, error, is_first_itteration_flag);
                    if (*error == ERROR_MEMORY_ALLOCATION){
                        handleError(error, num_line, is_print);
                        return;
                    }
                    if (*error == ERROR_DUPLICATE_LABEL)
                    {
                        handleError(error, num_line, is_print);
                        *error = NO_ERROR;
                        nextLine(&temp_code, &num_line);                        
                        continue;
                    }                    
                }
                if (checkCommandLine(tokens, num_tokens, label_flag, *labels, error, is_first_itteration_flag, &stop_flag) != COMMAND_LINE_ERROR) {
                    
                    binary_word = createCommandBinaryWord(tokens, num_tokens, token_idx, error, is_first_itteration_flag, *labels);
                    pushToMemory(memory_idx, memory, binary_word, error, num_line, is_print);
                    if (*error == ERROR_MAXED_OUT_MEMORY) return;
                    L = checkCommandLine(tokens, num_tokens, label_flag, *labels, error, is_first_itteration_flag, &stop_flag);
                    if (num_tokens >= 4)
                    {
                        operand_num = 2;
                    }
                    else
                    {
                        operand_num = L-1;
                    }
                    
                    /* L-1 = operand_num*/
                    switch (operand_num)
                    {
                    case 0:
                        createOperandBinaryWord(L, *labels, true, OPERAND_TYPE_OTHER, OPERAND_TYPE_OTHER, (char*) NULL, (char*) NULL,
                        memory_idx, memory, error, num_line, is_print);
                        break;
                    case 1:
                        createOperandBinaryWord(L, *labels, true, checkOperand(tokens[token_idx + 1], *labels, error, is_first_itteration_flag),
                        OPERAND_TYPE_OTHER, tokens[token_idx + 1], (char*) NULL, memory_idx, memory, error, num_line, is_print);
                        break;
                    case 2:
                        createOperandBinaryWord(L, *labels, true, checkOperand(tokens[token_idx + 1], *labels, error, is_first_itteration_flag),
                        checkOperand(tokens[token_idx + 3], *labels, error, is_first_itteration_flag), tokens[token_idx + 1],
                        tokens[token_idx + 3], memory_idx, memory, error, num_line, is_print);
                        break;
                    }
                }
                /*handle error*/
                if (*error != NO_ERROR) {
                    handleError(error, num_line, is_print);
                    *error = NO_ERROR;
                    nextLine(&temp_code, &num_line);
                    continue;
                }
                *IC += L;
                L = DEFAULT_VALUE;
                break;
        }
        num_line++;
        temp_code = temp_code->next;
    }

    moveDataToMemory(data_memory, &data_memory_idx, memory, memory_idx, error);
    if (!stop_flag)
    {
        *error = ERROR_NO_STOP_COMMAND;
        handleError(error, num_line, is_print);
        *error = NO_ERROR;
    }
    if (*error == ERROR_MAXED_OUT_MEMORY || is_print == false) return;

    temp_label_node = *labels;
    while (temp_label_node) {
        switch (temp_label_node->label_type) {
            case LABEL_TYPE_DATA:
                temp_label_node->memory_adress += *IC;
            case LABEL_TYPE_CODE:
            case LABEL_TYPE_ENTRY:
                temp_label_node->memory_adress += DEFAULT_MEMORY_INDEX;
                break;
            case LABEL_TYPE_EXTERNAL:
            case LABEL_TYPE_NOT_FOUND:
                /* *error = ERROR_UNRECOGNIZED_LABEL; */
                break;
        }
        temp_label_node = temp_label_node->next;
    }
    freeMemory(tokens, NULL, NULL, NULL);
}

void secondIteration(short* memory, int* memory_idx, CodeNode* code, LabelNode* labels, int* DC, int* IC, Error* error, char* file_name, LabelNode* externals, bool* is_print) {
    CodeNode* temp_code;
    LabelNode* temp_label;
    bool stop_flag = false;
    bool is_first_itteration_flag = false; 
    int token_idx = DEFAULT_VALUE;
    bool label_flag = false;
    char** tokens = allocateMemory(MAX_TOKENS * sizeof(char *), is_print, error);
    int num_tokens = DEFAULT_VALUE;
    int L = DEFAULT_VALUE;
    int num_line = STARTING_LINE;
    int update_memory_idx = DEFAULT_MEMORY_INDEX;
    int check_counter;
    short curr_memory;

    allocateMemoryTokens(tokens, is_print, error);
    temp_code = code;
    *IC = DEFAULT_VALUE;
    while (temp_code) {
        token_idx = DEFAULT_VALUE;
        label_flag = false;
        if (temp_code->code_row[FIRST_CHARACTER] == ';') {
            temp_code = temp_code->next;
            num_line++;
            continue;
        }
        if (temp_code->code_row[FIRST_CHARACTER] == '\n' || temp_code->code_row[FIRST_CHARACTER] == '\0' || temp_code->code_row[FIRST_CHARACTER] == '\r') {
            temp_code = temp_code->next;
            num_line++;
            continue;
        }

        tokenizeInput(temp_code->code_row, tokens, &num_tokens, is_print, error);
        if (*error == ERROR_MEMORY_ALLOCATION){
            handleError(error, num_line, is_print);
            return;
        } 
        if(isLabel(tokens[token_idx], true)) {
            label_flag = true;
            token_idx++;
        }
        switch (getDotType(tokens[token_idx], error)) {
            case DOT_ENTRY:
                checkExternalEntryLine(tokens, num_tokens, error, &labels, LABEL_TYPE_ENTRY, is_first_itteration_flag);
                if (*error == ERROR_DUPLICATE_LABEL || *error == ERROR_NOT_ENOUGH_ARGUMENTS || *error == ERROR_INCORRECT_OPERAND_TYPE 
                    || *error == ERROR_WRONG_NUM_OF_COMMAS || *error == ERROR_UNRECOGNIZED_LABEL)
                {
                    handleError(error, num_line, is_print);
                    *error = NO_ERROR;
                    nextLine(&temp_code, &num_line);
                    continue;
                }
                
                updateEntryLabels(labels, tokens, num_tokens, token_idx);
                break;
            case DOT_OTHER:
                if (checkCommandLine(tokens, num_tokens, label_flag, labels, error, is_first_itteration_flag, &stop_flag) != COMMAND_LINE_ERROR) {
                    L = checkCommandLine(tokens, num_tokens, label_flag, labels, error, is_first_itteration_flag, &stop_flag);
                    check_counter = L;
                    curr_memory = memory[update_memory_idx];
                    /*Source*/
                    if ((curr_memory & 0x600) == 0x600) {
                        update_memory_idx++;
                        check_counter--;
                        if (memory[update_memory_idx] == 0xFFF) {
                            temp_label = labels;
                            while (temp_label) {
                                if (!strcmp(tokens[token_idx+1], temp_label->label_name)) {
                                    if (temp_label->label_type == LABEL_TYPE_EXTERNAL) {
                                        memory[update_memory_idx] = 0x001;
                                        insertNewLabel(&externals, temp_label->label_name, LABEL_TYPE_EXTERNAL, &update_memory_idx, is_print, error, is_first_itteration_flag);
                                        update_memory_idx++;
                                        check_counter--;
                                    } else {
                                        memory[update_memory_idx] = temp_label->memory_adress;
                                        memory[update_memory_idx] <<= 2;
                                        memory[update_memory_idx] += 2;
                                        update_memory_idx++;
                                        check_counter--;
                                    }
                                    break;
                                }
                                temp_label = temp_label->next;
                            }
                        }
                    } else {
                        if ((curr_memory & 0xE00) != 0x000) {
                            update_memory_idx++;
                            check_counter--;
                        }
                    }
                    if ((curr_memory & 0xE00) != 0x000) {
                        token_idx += 3;
                    } else {
                        token_idx++;
                    }
                    /*Destination*/
                    if ((curr_memory & 0x00C) == 0x00C) {
                        update_memory_idx++;
                        check_counter--;
                        if (memory[update_memory_idx] == 0xFFF) {
                            temp_label = labels;
                            while (temp_label) {
                                if (!strcmp(tokens[token_idx], temp_label->label_name)) {
                                    if (temp_label->label_type == LABEL_TYPE_EXTERNAL) {
                                        memory[update_memory_idx] = 0x001;
                                        insertNewLabel(&externals, temp_label->label_name, LABEL_TYPE_EXTERNAL, &update_memory_idx, is_print, error, is_first_itteration_flag);
                                        update_memory_idx++;
                                        check_counter--;
                                    } else {
                                        memory[update_memory_idx] = temp_label->memory_adress;
                                        memory[update_memory_idx] <<= 2;
                                        memory[update_memory_idx] += 2;
                                        update_memory_idx++;
                                        check_counter--;
                                    }
                                    break;
                                }
                                temp_label = temp_label->next;
                            }
                        }
                    }
                    update_memory_idx += check_counter;
                }
                if (*error != NO_ERROR) {
                    handleError(error, num_line, is_print);
                    *error = NO_ERROR;
                    nextLine(&temp_code, &num_line);
                    continue;
                }
                *IC += L;
                break;
            
            case DOT_EXTERN:
                checkExternalEntryLine(tokens, num_tokens, error, &labels, LABEL_TYPE_EXTERNAL, is_first_itteration_flag);
                if (*error == ERROR_DUPLICATE_LABEL || *error == ERROR_NOT_ENOUGH_ARGUMENTS || *error == ERROR_INCORRECT_OPERAND_TYPE
                    || *error == ERROR_WRONG_NUM_OF_COMMAS)
                {
                    handleError(error, num_line, is_print);
                    *error = NO_ERROR;
                    nextLine(&temp_code, &num_line);
                    continue;
                }
                break;
            case DOT_DATA:
            case DOT_STRING:
                break;
        }
        num_line++;
        temp_code = temp_code->next;
        L = DEFAULT_VALUE;
    }
    
    if (*error == ERROR_FILE_HANDLE){
        handleError(error, num_line, is_print);
        return;
    }

    if (!stop_flag)
    {
        *error = ERROR_NO_STOP_COMMAND;
        handleError(error, num_line, is_print);
        *error = NO_ERROR;
    }

    createOutputFiles(file_name, labels, memory, memory_idx, *IC, *DC, externals, is_print, error, num_line);
    freeMemory(tokens, code, NULL, labels);
    freeMemoryLabelNode(externals);
}

void createOperandBinaryWord(int L, LabelNode* labels, bool is_first_iteration, OperandType op_type_1, OperandType op_type_2, char* operand1, char* operand2, int* memory_idx, short* memory, Error* error, int num_line, bool* is_print) {
    short resulting_binary_word = 0x0;

    switch (L) {
        case 1:
            break;
        case 2:
            if (op_type_1 == OPERAND_TYPE_REGISTER && op_type_2 == OPERAND_TYPE_REGISTER) {
                resulting_binary_word += (short) atoi(operand1 + 2);
                resulting_binary_word <<= 5;
                resulting_binary_word += (short) atoi(operand2 + 2);
                resulting_binary_word <<= 2;
                pushToMemory(memory_idx, memory, resulting_binary_word, error, num_line, is_print);
                if (*error == ERROR_MAXED_OUT_MEMORY) return;
            } else {
                createBinaryWordByType(labels, op_type_1, operand1, memory, memory_idx, is_first_iteration, error, num_line, is_print);
            }
            break;
        case 3: 
            createBinaryWordByType(labels, op_type_1, operand1, memory, memory_idx, is_first_iteration, error, num_line, is_print);
            createBinaryWordByType(labels, op_type_2, operand2, memory, memory_idx, is_first_iteration, error, num_line, is_print);
            break;
    }
}

void createBinaryWordByType(LabelNode* labels, OperandType op_type, char* operand, short* memory, int* memory_idx, bool is_first_iteration, Error* error, int num_line, bool* is_print) {
    short resulting_binary_word = 0x0;
    LabelNode* temp_label_node;

    switch (op_type) {
        case OPERAND_TYPE_REGISTER:
            resulting_binary_word += (short) atoi(operand + 2);
            resulting_binary_word <<= 7;
            pushToMemory(memory_idx, memory, resulting_binary_word, error, num_line, is_print);
            if (*error == ERROR_MAXED_OUT_MEMORY) return;
            break;
        case OPERAND_TYPE_LABEL:
            if (!is_first_iteration) {
                temp_label_node = labels;
                while (temp_label_node) {
                    if (!strcmp(temp_label_node->label_name, operand)) {
                        resulting_binary_word += temp_label_node->memory_adress;
                        resulting_binary_word <<= 2;
                        pushToMemory(memory_idx, memory, resulting_binary_word, error, num_line, is_print);
                        if (*error == ERROR_MAXED_OUT_MEMORY) return;
                    }
                    temp_label_node = temp_label_node->next;
                }
            } else {
                pushToMemory(memory_idx, memory, 0xFFF, error, num_line, is_print);
                if (*error == ERROR_MAXED_OUT_MEMORY) return;
            }
            break;
        case OPERAND_TYPE_NUMBER:
            resulting_binary_word += (short) atoi(operand);
            resulting_binary_word <<= 2;
            pushToMemory(memory_idx, memory, resulting_binary_word, error, num_line, is_print);
            if (*error == ERROR_MAXED_OUT_MEMORY) return;
            break;
        case OPERAND_TYPE_OTHER:
            break;
    }
}

short createCommandBinaryWord(char** tokens, int num_tokens, int token_idx, Error* error, bool is_first_itteration, LabelNode* labelPtr) {
    short resulting_binary_word = DEFAULT_VALUE;
    short source_operand, destination_operand;
    int temp_idx = token_idx;
    int operand_amount;
    int opcode;

    operand_amount = getOperandAmount(tokens[temp_idx]);
    opcode = checkCommand(tokens[temp_idx++]);

    switch (operand_amount) {
        case 0:
            source_operand = destination_operand = 0x0;
            break;
        case 1:
            destination_operand = getAdressingMethodByOperandType(checkOperand(tokens[temp_idx++], labelPtr, error, is_first_itteration));;
            source_operand = 0x0;
            break;
        case 2:
            source_operand = getAdressingMethodByOperandType(checkOperand(tokens[temp_idx++], labelPtr, error, is_first_itteration));
            destination_operand = getAdressingMethodByOperandType(checkOperand(tokens[++temp_idx], labelPtr, error, is_first_itteration));
            break;
    }
    resulting_binary_word +=  (source_operand << 9);
    resulting_binary_word +=  (opcode << 5);
    resulting_binary_word +=  (destination_operand << 2);
    resulting_binary_word +=  0;
    return resulting_binary_word;
}

int getOperandsNumberByOpcode(short opcode) {
    int i;

    for (i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (commands[i].opcode == opcode) {
            return commands[i].number_of_operands;
        }
    }
    return DEFAULT_ERROR_VALUE;
}

void convertToBase64(short num, char* result) {
    result[0] = base64_chars[(num >> 6) & MASK64];
    result[1] = base64_chars[num & MASK64];
    result[2] = '\0';
}

int getAdressingMethodByOperandType(OperandType operand_type) {
    switch (operand_type) {
        case OPERAND_TYPE_NUMBER:   return 0x1; /*TO BE CHANGED TO CONSTANT*/
        case OPERAND_TYPE_LABEL:    return 0x3;
        case OPERAND_TYPE_REGISTER: return 0x5;
        case OPERAND_TYPE_OTHER:    return 0xFFF;
    }
    return DEFAULT_ERROR_VALUE;
}

int getOperandAmount(char* command) {
    int i;
    for(i = 0; i < NUMBER_OF_COMMANDS; i++) {
        if (!strcmp(command, commands[i].command)) {
            return commands[i].number_of_operands;
        }
    }
    return DEFAULT_ERROR_VALUE; /*CHANGE LATER*/
}

void incrementTokenCounter(int* token_idx, int num_tokens, int num_line, int step, bool* is_print, Error* error) {
    if (*token_idx + step >= num_tokens) {
        *error = ERROR_NOT_ENOUGH_ARGUMENTS;
        handleError(error, num_line, is_print);
    }
}

void updateEntryLabels(LabelNode* labels, char** tokens, int num_tokens, int token_idx) {
    LabelNode* temp_label;
    
    
    for (;token_idx < num_tokens; token_idx++) {
        if (isLabel(tokens[token_idx], false)) {
            temp_label = labels;
            while(temp_label) {
                if (!strcmp(temp_label->label_name, tokens[token_idx])) {
                    temp_label->label_type = LABEL_TYPE_ENTRY;
                }
                temp_label = temp_label->next;
            }
        }
    }
}

void createOutputFiles (char* file_name, LabelNode* labels, short* memory, int* memory_idx, int IC, int DC, LabelNode* externals, bool* is_print, Error* error, int num_line) {
    if (*is_print) {
        createFileWithLabelType(file_name, labels, LABEL_TYPE_ENTRY, is_print, error);
        createFileWithLabelType(file_name, externals, LABEL_TYPE_EXTERNAL, is_print, error);
        createFileWithMemoryDump(file_name, memory, memory_idx, IC, DC, error, num_line, is_print);
    }
}

void createFileWithMemoryDump(char* file_name, short* memory, int* memory_idx, int IC, int DC, Error* error,  int num_line, bool* is_print) {
    FILE *fptr;
    int i;
    char output_file_name[MAX_FILE_NAME_WITH_EXTENSION];
    char base64[3];

    cleanLine(output_file_name, MAX_FILE_NAME_WITH_EXTENSION);
    strcat(output_file_name, "output/");
    strcat(output_file_name, file_name);
    strcat(output_file_name, ".ob");

    fptr = fopen(output_file_name, "w");

    if (!fptr) {
        *error = ERROR_FILE_HANDLE;
        handleError(error, num_line, is_print);
        return;
    }
    fprintf(fptr, "%d %d\n", IC, DC);
    if (DC == 0) {
        (*memory_idx)--;
    }
    for (i = DEFAULT_MEMORY_INDEX; i < *memory_idx; i++) {
        convertToBase64(memory[i], base64);
        fprintf(fptr, "%s\n", base64);
    }
    fclose(fptr);
}

void createFileWithLabelType(char* file_name, LabelNode* labels, LabelType label_type, bool* is_print, Error* error) {
    FILE *fptr;
    bool is_entry = false;
    LabelNode* temp = labels;
    char output_file_name[MAX_FILE_NAME_WITH_EXTENSION];
    
    cleanLine(output_file_name, MAX_FILE_NAME_WITH_EXTENSION);

    strcat(output_file_name, "output/");
    strcat(output_file_name, file_name);
    switch (label_type) {
        case LABEL_TYPE_ENTRY:
            strcat(output_file_name, ".ent");
            while (temp) {
                if (temp->label_type == LABEL_TYPE_ENTRY) {
                    is_entry = true;
                    break;
                }
                temp = temp->next;
            }
            if (!is_entry) {
                return;
            }
            break;
        case LABEL_TYPE_EXTERNAL:
            strcat(output_file_name, ".ext");
            if (!labels) {
                return;
            }
            break;
        default:
            *error = ERROR_UNRECOGNIZED_LABEL;
            return;
    }
    fptr = fopen(output_file_name, "w");

    if (!fptr) {
        *error = ERROR_FILE_HANDLE;
        handleError(error, DEFAULT_LINE_NUMER, is_print);
        return;
    }

    /* write all of the labels into the corresponding output file type */
    while (labels) {
        if (labels->label_type == label_type) {
            fprintf(fptr, "%s %d\n", labels->label_name, labels->memory_adress);
        }
        labels = labels->next;
    }
    fclose(fptr);
}

void createCodeFileWithoutMacros(char* file_name, CodeNode* code, bool* is_print, Error* error) {
    char output_file_name[MAX_FILE_NAME_WITH_EXTENSION];
    FILE *fptr;
    
    cleanLine(output_file_name, MAX_FILE_NAME_WITH_EXTENSION);

    strcat(output_file_name, "output/");
    strcat(output_file_name, file_name);
    strcat(output_file_name, ".am");

    fptr = fopen(output_file_name, "w");

    if (!fptr) {
        *error = ERROR_FILE_HANDLE;
        handleError(error, DEFAULT_LINE_NUMER, is_print);
        return;
    }

    while (code) {
        fprintf(fptr, "%s\n", code->code_row);
        code = code->next;
    }
    fclose(fptr);
}

bool isLabel(char* word, bool colon){
    int i = 0;

    if (!isalpha(word[i++])) {
        return false;
    }
    for (; word[i] != '\0'; i++) {
        if (colon && word[i] == ':' && word[i+1] == '\0') {
            return true;
        }
        if (!isalpha(word[i]) && !isdigit(word[i])) {
            return false;
        }
    }
    return !colon;
}

DotType getDotType(char* word, Error* error){
    if (!strcmp(word, ".data")) return DOT_DATA;
    if (!strcmp(word, ".string")) return DOT_STRING;
    if (!strcmp(word, ".entry")) return DOT_ENTRY;
    if (!strcmp(word, ".extern")) return DOT_EXTERN;
    return DOT_OTHER;
}

LabelType getLabelType(char* label, LabelNode* LabelPtr, Error* error){
    while (LabelPtr) {
        if (!strcmp(label, LabelPtr->label_name)) {
            return LabelPtr->label_type;
        }
        LabelPtr = LabelPtr->next;
    }
    *error = ERROR_UNRECOGNIZED_LABEL;
    return LABEL_TYPE_NOT_FOUND;
}

bool isString(char** tokens, int num_tokens, bool label) {
    /*an idea for taking in  tokens , and then going through all of the tokens characters. basically just add
    an outside loop. */
    int char_index = 0;
    int string_index = label + 1;
    bool quote = false; /* a quote flag, that checks if found the 2nd (") character */
    int len = 0;

    /* checks the 1st character to be a quote */
    if (tokens[string_index][char_index++] != '"') {
        return false;
    }
    for (; string_index < num_tokens; string_index++)
    {
        len = strlen(tokens[string_index]);
        for (; char_index < len; char_index++) {
            if (quote) return false;

            if (tokens[string_index][char_index] == '"') quote = true;
        }
        char_index = 0;
    }

    /* returns false if 2nd quote isn't the last character */
    return quote;
}

bool isNumber(char* word){
    int i = 0;
    int len = strlen(word);

    /* Check for a minus sign at the beginning
     Skip it if exists */
    if (word[i] == '-') i++;  
    
    for ( ; i < len; i++) {
        if (!isdigit(word[i])) {
            return false; /* not a number*/
        }
    }
    return true;
}

bool checkDataLine(char** tokens, int num_tokens, bool label, Error* error){
    int token_index = 1;
    

    if (num_tokens < (2 + label)) {
        *error = ERROR_MISSING_DATA_ARGUMENT;
        return false;
    }
    

    /* checks .string line */
    if (getDotType(tokens[FIRST_WORD + label], error) == DOT_STRING) {
        /*
        if (num_tokens > (2 + label)) {
            *error = ERROR_EXTRANEOS_TEXT;
            return false;
        }
        */
        if (isString(tokens, num_tokens, label)) {
            return true;
        }
        
        /* the argument to .string is wrong */
        else
        {
            *error = ERROR_WRONG_ARGUMENT_FORMAT;
            return false;
        }
        
    }
    
    /* check .data line */
    if (getDotType(tokens[FIRST_WORD + label], error) == DOT_DATA) {
        if (num_tokens % 2 == (!label)) {
            *error = ERROR_WRONG_NUM_OF_COMMAS;
            return false;
        }
        token_index += label;

        /* loops throught all the arguments to .data */
        for (; token_index< num_tokens; token_index+=2) {
            if (!isNumber(tokens[token_index])) {
                *error = ERROR_WRONG_ARGUMENT_FORMAT;
                return false;
            }
        }

        /* loops throught all the places in the line , and checks if there is a comma
         where its needed to be */
        for (; token_index + 1 < num_tokens; token_index += 2) {
            if (strcmp(tokens[token_index], ",")) {
                *error = ERROR_MISSING_COMMA;
                return false;
            }
        }
        return true;
    }
    return false;
}


void pushToMemory(int* memory_idx, short* memory, short memoryField, Error* error, int num_line, bool *is_print) {
    if (*memory_idx >= MAX_MEMORY_SIZE) {
        *error = ERROR_MAXED_OUT_MEMORY;
        handleError(error, num_line, is_print);
        return;
    }
    memory[(*memory_idx)++] = memoryField;
}

void cleanMemory(short* memory) {
    int i;

    for (i = 0; i < MAX_MEMORY_SIZE; i++) {
        memory[i] = DEFAULT_ERROR_VALUE;
    }
}

void insertNewLabel(LabelNode** labels, char* label_name, LabelType label_type, int* memory_idx, bool* is_print, Error* error, bool is_first_itteration_flag) {
    LabelNode* temp_label;
    LabelNode* new_label;
    temp_label = *labels;

    if (isDuplicatedLabel(labels, label_name, label_type, error, is_first_itteration_flag))
    {
        return;
    }
    
    *error = NO_ERROR;
    /* if the head node isn't null */
    if (temp_label) {

        /* loop through linked list, untill getting to the end */
        while(temp_label && temp_label->next) {
            temp_label = temp_label->next;
        }
        /* allocating memory to the new node, in which the label name will be stored */
        new_label = (LabelNode*) allocateMemory(sizeof(LabelNode), is_print, error);
        new_label->label_name = allocateMemory(sizeof(label_name) * sizeof(char), is_print, error);

        /* add the label  into the linked list */        
        strcpy(new_label->label_name, label_name);
        new_label->label_type = label_type;
        new_label->memory_adress = *memory_idx;
        temp_label->next = new_label;
    } else {
        /* creates the first label node in the linked list */
        (*labels) = (LabelNode*) allocateMemory(sizeof(LabelNode), is_print, error);
        (*labels)->label_name = allocateMemory(sizeof(label_name) * sizeof(char), is_print, error);
        strcpy((*labels)->label_name, label_name);
        (*labels)->label_type = label_type;
        (*labels)->memory_adress = *memory_idx;
    }
}

char* removeColon(char* str) {
    str[strlen(str)-1] = '\0';
    return str;
}

short checkCommand(char* word){
    int i;

    for (i = 0; i < NUM_OF_COMMANDS; i++) {
        if (!strcmp((char*)commands[i].command, word)) {
            return commands[i].opcode;
        }
    }
    return DEFAULT_ERROR_VALUE;
}

int checkCommandLine(char** tokens, int num_tokens, bool label, LabelNode* LabelPtr, Error* error, bool is_first_iteration, bool* stop_flag) {
    short opcode = checkCommand(tokens[label]);

    int count = DEFAULT_VALUE;
    int operand_index = label+1; /*operand index*/
    int operand_result = DEFAULT_ERROR_VALUE;
    bool register_flag = false;
    bool source_flag = true; /* flag that looks after the operand if its a source or destination*/
    int L = 1;

    if (opcode == COMMAND_STOP)
    {
        *stop_flag = true;
    }

    /*ERROR unrecognized command name*/
    if (opcode == DEFAULT_ERROR_VALUE) {
        *error = ERROR_UNDEFINED_COMMAND; 
        return COMMAND_LINE_ERROR;
    }

    /* ERROR wrong amount of operands for the command */
    if ((num_tokens <= 2 + label) && ((num_tokens - label - 1) != commands[opcode].number_of_operands)) {
        *error = ERROR_WRONG_AMOUNT_OF_OPERANDS;
        return COMMAND_LINE_ERROR;
    }
    
    /*ERROR illegal comma*/
    if (!strcmp(tokens[num_tokens-1], ",")) {
        *error = ERROR_ILLEGAL_COMMA;
        return COMMAND_LINE_ERROR;
    }
    
    /* checks the number of operands for command that require 2 operands*/
    if ((num_tokens > 2 + label) && (num_tokens - label - 2) != commands[opcode].number_of_operands) {
        *error = ERROR_WRONG_AMOUNT_OF_OPERANDS;
        return COMMAND_LINE_ERROR;
    }
    
    /*  loop that goes through arguments if there are any, and checks them. Otherwise skip the loop   */
    for (; count < commands[opcode].number_of_operands; count++) {
        if (!source_flag) {
            operand_result = checkOperand(tokens[operand_index + count + 1], LabelPtr, error, is_first_iteration);
        }
        else {           
            operand_result = checkOperand(tokens[operand_index + count], LabelPtr, error, is_first_iteration);
        }
        

        switch (operand_result) {
            case OPERAND_TYPE_LABEL:
                L++;

                /* checks commands that require less then 2 operands, for correctness of arguments type */
                if (commands[opcode].number_of_operands < 2) {
                    if (!commands[opcode].destinationAddresingMethod[ADDRESING_LABEL]) {
                        *error = ERROR_INCORRECT_OPERAND_TYPE;
                        return COMMAND_LINE_ERROR;
                    }
                    break;
                }

                /* checks the commands that require 2 operands, for correctness of arguments type*/
                if ((source_flag && !commands[opcode].sourceAddresingMethod[ADDRESING_LABEL]) || (!source_flag && !commands[opcode].destinationAddresingMethod[ADDRESING_LABEL])) {
                    *error = ERROR_INCORRECT_OPERAND_TYPE;
                    return COMMAND_LINE_ERROR; 
                }
                break;
            case OPERAND_TYPE_REGISTER:

                /* checks commands that require less then 2 operands, for correctness of arguments type*/
                if (commands[opcode].number_of_operands < 2) {
                    if (!commands[opcode].destinationAddresingMethod[ADDRESING_REGISTER]) {
                        *error = ERROR_INCORRECT_OPERAND_TYPE;
                        return COMMAND_LINE_ERROR;
                    }
                    else {
                        L++;
                        break;
                    }
                }
                /* checks the commands that require 2 operands, for correctness of arguments type*/
                if ((source_flag && !commands[opcode].sourceAddresingMethod[ADDRESING_REGISTER]) || (!source_flag && !commands[opcode].destinationAddresingMethod[ADDRESING_REGISTER])) {
                    *error = ERROR_INCORRECT_OPERAND_TYPE;
                    return COMMAND_LINE_ERROR; 
                }

                /* No matter the amount of registers, give only 1 memmory word for it 
                    (gives only +1 to L, even if there are 2 registers as arguments for a command)
                */
                if (!register_flag) {
                    L++;
                    register_flag = true;
                }
                break;
            case OPERAND_TYPE_NUMBER:


                /*checks commands that require less then 2 operands, for correctness of arguments type*/
                if (commands[opcode].number_of_operands < 2) {
                    if (!commands[opcode].destinationAddresingMethod[ADDRESING_NUMBER]) {
                        *error = ERROR_INCORRECT_OPERAND_TYPE;
                        return COMMAND_LINE_ERROR;
                    }
                    else {
                        L++;
                        break;
                    }
                }
                
                /* checks the commands that require 2 operands, for correctness of arguments type*/
                if ((source_flag && !commands[opcode].sourceAddresingMethod[ADDRESING_NUMBER]) || (!source_flag && !commands[opcode].destinationAddresingMethod[ADDRESING_NUMBER])) {
                    *error = ERROR_INCORRECT_OPERAND_TYPE;
                    return COMMAND_LINE_ERROR; 
                }
                L++;
                break;
            case OPERAND_TYPE_OTHER:
                *error = ERROR_ILLEGAL_OPERAND_TYPE;
                *error = NO_ERROR;
                return COMMAND_LINE_ERROR;
            }
        if (source_flag) source_flag = false;

        operand_result = DEFAULT_ERROR_VALUE;
    }
    return L;
}

OperandType checkOperand(char* operand, LabelNode* LabelPtr, Error* error, bool is_first_iteration){    
    const char* registers[] = {"@r0", "@r1", "@r2", "@r3", "@r4", "@r5", "@r6", "@r7"};
    int i;

    /* Check if the operand is one of the registers */
    for (i = 0; i < NUM_OF_REGISTERS; i++) {
        if (!strcmp(registers[i], operand)) {
            return OPERAND_TYPE_REGISTER;
        }
    }

    /* Check if the operand is a number */
    if (isNumber(operand)) {
        return OPERAND_TYPE_NUMBER;
    }

    /* Check if the operand is a legal label, depending on the iteration */
    if (is_first_iteration) {
        if (isLabel(operand, false)) {
            return OPERAND_TYPE_LABEL;
        }
    } else if (getLabelType(operand, LabelPtr, error) != LABEL_TYPE_NOT_FOUND) {
        return OPERAND_TYPE_LABEL;
    }

    /* Handle error case */
    return OPERAND_TYPE_OTHER;
}

void moveDataToMemory(short* data_memory, int* data_memory_idx, short* memory, int* memory_idx, Error* error){
    /* Set the initial index for data_memory */
    *data_memory_idx = DEFAULT_MEMORY_INDEX;

    /* Loop until either data_memory or memory is maxed out */
    while (*data_memory_idx < MAX_MEMORY_SIZE && *memory_idx < MAX_MEMORY_SIZE) {

        /* Copy data from data_memory to memory */
        memory[*memory_idx] = data_memory[*data_memory_idx];
        (*memory_idx)++;
        (*data_memory_idx)++;

        /* Check for termination conditions */
        if (*data_memory_idx == MAX_MEMORY_SIZE || data_memory[*data_memory_idx] == DEFAULT_ERROR_VALUE) {
            break;
        }
    }

    /* Check if memory limits were reached */
    if (*data_memory_idx >= MAX_MEMORY_SIZE || *memory_idx >= MAX_MEMORY_SIZE) {
        *error = ERROR_MAXED_OUT_MEMORY;
        return;
    }
}

void nextLine(CodeNode** temp_code, int* num_line){
    /* Move to the next CodeNode and increment the line number */
    *temp_code =  (*temp_code)->next;
    (*num_line)++;
}

bool checkExternalEntryLine(char** tokens, int num_tokens, Error* error, LabelNode** labels, LabelType label_type, bool is_first_itteration){
    int operand_index = FIRST_ARGUMENT;
    bool entryLine = false;
    if (label_type == LABEL_TYPE_ENTRY)
    {
        entryLine = true;
    }
    

    /* Check if there are enough tokens */
    if (num_tokens < 2)
    {
        *error = ERROR_NOT_ENOUGH_ARGUMENTS;
        return false;
    }

    /* Check if the number of tokens is even */
    if (num_tokens % 2 != EVEN) {
        *error = ERROR_WRONG_NUM_OF_COMMAS;
        return false;
    }

    /* Check for missing commas */
    for (; operand_index + FIRST_ARGUMENT < num_tokens; operand_index += 2) {
        if (strcmp(tokens[operand_index+1], ",")) {
            *error = ERROR_MISSING_COMMA;
            return false;
        }
    }
    operand_index = FIRST_ARGUMENT;

    /* Check each operand for correctness */
    for (; operand_index < num_tokens; operand_index += 2)
    {
        if (!isLabel(tokens[operand_index], false))
        {
            *error = ERROR_INCORRECT_OPERAND_TYPE;
            return false;
        }

        if (entryLine && getLabelType(tokens[operand_index], *labels, error) == LABEL_TYPE_NOT_FOUND)
        {
             *error = ERROR_UNRECOGNIZED_LABEL;
             return false;
        }


    }
    operand_index = FIRST_ARGUMENT;

    /* Check for duplicated labels */
    return !(isDuplicatedLabel(labels, tokens[operand_index], label_type, error, is_first_itteration));
}

bool isDuplicatedLabel(LabelNode** labels, char* label_name, LabelType label_type, Error* error, bool is_first_itteration){
    LabelType argument_label_type = LABEL_TYPE_NOT_FOUND;
    argument_label_type =  getLabelType(label_name, *labels, error); /* gets the label type from the labels linked list */

    /*checks if entry label exists in other label types*/
    if ((label_type == LABEL_TYPE_ENTRY) && (argument_label_type == LABEL_TYPE_EXTERNAL) &&  argument_label_type != LABEL_TYPE_NOT_FOUND )
    {
        *error = ERROR_DUPLICATE_LABEL;
        return true;
    }

    /*checks if external label exists in other label types*/
    if ((label_type == LABEL_TYPE_EXTERNAL) && (argument_label_type != LABEL_TYPE_EXTERNAL) &&  argument_label_type != LABEL_TYPE_NOT_FOUND && is_first_itteration)
    {
        *error = ERROR_DUPLICATE_LABEL;
        return true;
    }
    

    /*checks if there are duplicate of labels in label types of CODE and DATA among themselfs*/
    else {
        switch (argument_label_type)
        {
        case LABEL_TYPE_CODE:
        case LABEL_TYPE_DATA:
            if (label_type == LABEL_TYPE_CODE || label_type == LABEL_TYPE_DATA)
            {
                *error = ERROR_DUPLICATE_LABEL;
                return true;
            }        
        default:
            return false;
            break;
        }
    }
}