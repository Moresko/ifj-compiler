/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _PARSER_H
#define _PARSER_H

#include <stdbool.h>
#include "scanner.h"
#include "symtable.h"

typedef enum{
    STATE_NULL,
    STATE_FUNCTION_DECLARATION,
    STATE_FUNCTION_DEFINITION,
    STATE_FUNCTION_CALL,
    STATE_ID_INIT,
    STATE_RETURN,
    STATE_ASSIGNMENT_TO_ID,
    STATE_IF,
    STATE_WHILE,
} Parser_State;

typedef struct{
    Sym_table global_table;
    DLLSymt list_of_symtables;
    DLLVariables list_of_variables;

    TFunction *active_function;
    TVariable *active_variable;


    TToken *token;
    Parser_State state;

    bool params_or_return_types;
    int return_code;
} SPars_data;

int analyse();
bool is_next_token_valid(SPars_data *sdata, char*file, int line);

#endif
