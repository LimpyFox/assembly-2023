#ifndef CONSTANTS_H
#define CONSTANTS_H

#define MAX_LINE_LENGTH 80
#define MAX_TOKEN_LENGTH 80
#define MAX_TOKENS 80
#define NUMBER_OF_COMMANDS 16
#define NUM_OF_REGISTERS 8
#define MAX_FILE_NAME 30
#define MAX_FILE_NAME_WITH_EXTENSION 34

#define MEMORY_INDEX 100

#define COMMAND_STOP 0xF

#define DEFAULT_VALUE 0 

#define FIRST_CHARACTER 0

#define FIRST_WORD 0
#define SECOND_WORD 1
#define FIRST_ARGUMENT 1

#define EVEN 0

#define MASK64 0x3F
#define DEFAULT_ERROR_VALUE -1
#define DEFAULT_MEMORY_VALUE -1

#define STARTING_LINE 1

#define ADDRESING_NUMBER 0
#define ADDRESING_LABEL 1 
#define ADDRESING_REGISTER 2

#define COMMAND_LINE_ERROR 0 /*Change this later*/

#define DEFAULT_LINE_NUMER -1

typedef enum error {
    NO_ERROR, /*IN USE*/
    ERROR_ILLEGAL_MACRO_NAME, /*IN USE*/
    ERROR_DUPLICATED_MACRO_DEFINITION, /*IN USE*/
    ERROR_MISSING_VARIABLE, /*IN USE*/
    ERROR_UNDEFINED_VARIABLE, /*IN USE*/ 
    ERROR_UNDEFINED_COMMAND,/*IN USE*/
    ERROR_MISSING_COMMA, /*IN USE*/ 
    ERROR_ILLEGAL_COMMA, /*IN USE*/
    ERROR_EXTRANEOS_TEXT, /*IN USE*/
    ERROR_MAXED_OUT_LINE_LENGTH, /*IN USE ?? */
    ERROR_MEMORY_ALLOCATION, /*IN USE*/
    ERROR_WRONG_ARGUMENT_FORMAT, /*IN USE*/
    ERROR_FILE_HANDLE, /*IN USE*/
    ERROR_ILLEGAL_LABEL_NAME, /*IN USE*/
    ERROR_ILLEGAL_OPERAND_TYPE, /*IN USE*/
    ERROR_WRONG_AMOUNT_OF_OPERANDS, /*IN USE*/
    ERROR_INCORRECT_OPERAND_TYPE,  /*IN USE*/
    ERROR_MISSING_DATA_ARGUMENT, /*IN USE*/
    ERROR_WRONG_NUM_OF_COMMAS, /*IN USE*/
    ERROR_UNRECOGNIZED_LABEL, /*IN USE*/
    ERROR_MAXED_OUT_MEMORY, /*IN USE*/
    ERROR_CODE_AFTER_STOP, /*IN USE*/
    ERROR_NOT_ENOUGH_ARGUMENTS, /*IN USE*/
    ERROR_NO_STOP_COMMAND, /*IN USE*/
    ERROR_DUPLICATE_LABEL /*IN USE*/
} Error;

typedef enum DotType{
    DOT_OTHER,
    DOT_DATA,
    DOT_STRING, 
    DOT_ENTRY, 
    DOT_EXTERN
} DotType;

#define DEFAULT_EXTERN_MEMORY -1

#define MAX_MEMORY_SIZE 1024
/**
 * @brief a boolean type , that can hold either true or false;
 * 
 */
typedef enum bool{
    false = 0,
    true = 1
}bool;

typedef enum LabelType {
    LABEL_TYPE_NOT_FOUND,
    LABEL_TYPE_CODE,
    LABEL_TYPE_DATA,
    LABEL_TYPE_EXTERNAL,
    LABEL_TYPE_ENTRY
} LabelType;

typedef enum OperandType{
    OPERAND_TYPE_OTHER,
    OPERAND_TYPE_LABEL,
    OPERAND_TYPE_REGISTER,
    OPERAND_TYPE_NUMBER
} OperandType;

#endif