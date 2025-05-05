/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _TEST_UTILS_C
#define _TEST_UTILS_C

#include "test_utils.h"

/**
 * Funkcia vypise informacie k tokenu, jeho typ a pripadne atribut
 */
void print_token(TToken *token){
    printf("Token - ");
    switch(token->type){
        case TOKEN_TYPE_EMPTY:
            printf("Prazdny token\n");
            printf("Tento token nema atribut");
            break;
        case TOKEN_TYPE_KEYWORD:
            printf("KEYWORD\n");
            printf("Atribut - %s", token->value);
            break;
        case TOKEN_TYPE_IDENTIFIER:
            printf("IDENTIFIER\n");
            printf("Atribut - %s", token->value);
            break;
        case TOKEN_TYPE_INT:
            printf("INT\n");
            printf("Atribut - %s", token->value);
            break;
        case TOKEN_TYPE_DOUBLE:
            printf("DOUBLE\n");
            printf("Atribut - %s", token->value);
            break;
        case TOKEN_TYPE_STRING:
            printf("STRING\n");
            printf("Atribut - %s", token->value);
            break;
        case TOKEN_TYPE_OPERATOR_PLUS:
            printf("OPERATOR_PLUS");
            break;
        case TOKEN_TYPE_OPERATOR_MINUS:
            printf("OPERATOR_MINUS");
            break;
        case TOKEN_TYPE_OPERATOR_MULTIPLY:
            printf("OPERATOR_MULTIPLY");
            break;
        case TOKEN_TYPE_OPERATOR_DIVIDE:
            printf("OPERATOR_DIVIDE");
            break;
        case TOKEN_TYPE_OPERATOR_INT_DIVIDE:
            printf("OPERATOR_INT_DIVIDE");
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER:
            printf("RELATIONAL_OPERATOR_GREATER");
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_LESS:
            printf("RELATIONAL_OPERATOR_LESS");
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL:
            printf("RELATIONAL_OPERATOR_GREATER_EQUAL");
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL:
            printf("RELATIONAL_OPERATOR_LESS_EQUAL");
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL:
            printf("RELATIONAL_OPERATOR_EQUAL");
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL:
            printf("RELATIONAL_OPERATOR_NOT_EQUAL");
            break;
        case TOKEN_TYPE_CONCATENATION:
            printf("CONCATENATION");
            break;
        case TOKEN_TYPE_ASSIGNMENT:
            printf("ASSIGNMENT");
            break;
        case TOKEN_TYPE_LEFT_BRACKET:
            printf("LEFT_BRACKET");
            break;
        case TOKEN_TYPE_RIGHT_BRACKET:
            printf("RIGHT_BRACKET");
            break;
        case TOKEN_TYPE_COMMA:
            printf("COMMA");
            break;
        case TOKEN_TYPE_COLON:
            printf("COLON");
            break;
        case TOKEN_TYPE_EOF:
            printf("EOF");
            break;
        case TOKEN_TYPE_HASHTAG:
            printf("HASHTAG");
            break;
    }
    printf("\n");
    printf("Dlzka tokenu je: %d\n\n", token->length);

    return;
}

/**
 * Funkcia vytlaci prislusny data_type
 */
void print_data_type(Data_type type)
{
    switch(type)
    {
        case TYPE_UNDEFINED:
            printf("TYPE_UNDEFINED");
            break;
        case TYPE_INT:
            printf("TYPE_INT");
            break;
        case TYPE_NUMBER:
            printf("TYPE_NUMBER");
            break;
        case TYPE_STRING:
            printf("TYPE_STRING");
            break;
        case TYPE_BOOLEAN:
            printf("TYPE BOOLEAN");
            break;
        case TYPE_NIL:
            printf("TYPE_NIL");
            break;
    }

    printf("\n");
}

/**
 * Funkcia vytlaci prisulsny state na vystup
 */
void print_state(Parser_State state)
{
    switch(state)
    {
        case STATE_NULL:
            printf("STATE_NULL");
            break;
        case STATE_FUNCTION_DECLARATION:
            printf("STATE_FUNCTION_DECLARATION");
            break;
        case STATE_FUNCTION_DEFINITION:
            printf("STATE_FUNCTION_DEFINITION");
            break;
        case STATE_FUNCTION_CALL:
            printf("STATE_FUNCTION_CALL");
            break;
        case STATE_ID_INIT:
            printf("STATE_ID_INIT");
            break;
        case STATE_RETURN:
            printf("STATE_RETURN");
            break;
        case STATE_ASSIGNMENT_TO_ID:
            printf("STATE_ASSIGNMENT_TO_ID");
            break;
        case STATE_IF:
            printf("STATE_IF");
            break;
        case STATE_WHILE:
            printf("STATE_WHILE");
            break;
    }

    printf("\n");
}

/**
 * Vytlaci Table_index
 */
void print_table_index(Table_index tab_in)
{
    switch(tab_in)
    {
        case EXPR_PLUS:
            printf("EXPR_PLUS\n");
            break;
        case EXPR_MINUS:
            printf("EXPR_MINUS\n");
            break;
        case EXPR_MUL:
            printf("EXPR_MUL\n");
            break;
        case EXPR_DIV:
            printf("EXPR_DIV\n");
            break;
        case EXPR_DIV_INT:
            printf("EXPR_DIV_INT\n");
            break;
        case EXPR_CONC:
            printf("EXPR_CONC\n");
            break;
        case EXPR_LEFT_BRACKET:
            printf("EXPR_LEFT_BRACKET\n");
            break;
        case EXPR_RIGHT_BRACKET:
            printf("EXPR_RIGHT_BRACKET\n");
            break;
        case EXPR_LESS:
            printf("EXPR_LESS\n");
            break;
        case EXPR_LESS_EQ:
            printf("EXPR_LESS_EQ\n");
            break;
        case EXPR_GREATER:
            printf("EXPR_GREATER\n");
            break;
        case EXPR_GREATER_EQ:
            printf("EXPR_GREATER_EQ\n");
            break;
        case EXPR_EQ:
            printf("EXPR_EQ\n");
            break;
        case EXPR_NEQ:
            printf("EXPR_NEQ\n");
            break;
        case EXPR_I:
            printf("EXPR_I\n");
            break;
        case EXPR_DOLLAR:
            printf("EXPR_DOLLAR\n");
            break;
        case EXPR_LENGTH:
            printf("EXPR_LENGTH\n");
            break;

        case STACK_END:
            printf("STACK_END\n");
            break;
        case STACK_SHIFT:
            printf("STACK_SHIFT\n");
            break;
        case STACK_EXPRESSION:
            printf("STACK_EXPRESSION\n");
            break;

        case STACK_VOID:
            printf("STACK_VOID\n");
            break;
    }
}

#endif
