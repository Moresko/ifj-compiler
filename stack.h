/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 *
 * Autori ktori na tomto subore pracovali:
 *
 */

#ifndef _STACK_H
#define _STACK_H

#include <stdbool.h>

#define STACK_SIZE 151

typedef enum
{
    EXPR_PLUS,          // + 
    EXPR_MINUS,         // -
    EXPR_MUL,           // *
    EXPR_DIV,           // /
    EXPR_DIV_INT,       // //
    EXPR_CONC,          // ..
    EXPR_LEFT_BRACKET,  // (
    EXPR_RIGHT_BRACKET, // )
    EXPR_LESS,          // <
    EXPR_LESS_EQ,       // <=
    EXPR_GREATER,       // >
    EXPR_GREATER_EQ,    // >=
    EXPR_EQ,            // ==
    EXPR_NEQ,           // ~=
    EXPR_I,             // i
    EXPR_DOLLAR,        // $
    EXPR_LENGTH,        // #

    STACK_END,          // $
    STACK_SHIFT,        // <
    STACK_EXPRESSION,   // E

    STACK_VOID,         //nic
}Table_index;

typedef enum
{
    RULE_PLUS,       // E -> E + E
    RULE_MINUS,      // E -> E - E
    RULE_MUL,        // E -> E * E
    RULE_DIV,        // E -> E / E
    RULE_DIV_INT,    // E -> E // E
    RULE_CONC,       // E -> E .. E
    RULE_BRACKETS,   // E -> (E)
    RULE_LESS,       // E -> E < E
    RULE_LESS_EQ,    // E -> E <= E
    RULE_GREATER,    // E -> E > E
    RULE_GREATER_EQ, // E -> E >= E
    RULE_EQ,         // E -> E == E
    RULE_NEQ,        // E -> E ~= E
    RULE_LENGTH,     // E -> #E
    RULE_OPERAND,    // E -> i
    RULE_ERROR,      // Neexistuje pravidlo
}Prec_rule;

// ADT zasobnik implementovany v statickom poli
typedef struct {
    Table_index array[STACK_SIZE];  // Pole pre ulozenie premennych
    int topIndex;   // Index vrcholu
} Stack;

//Deklaracie funkcii
bool stack_init(Stack *stack);

bool stack_is_empty(const Stack *stack);

void stack_top_a(const Stack *stack, Table_index *symbol);

bool stack_top_y(const Stack *stack, int *shift_index);

Table_index stack_top(Stack *stack);

void stack_change_top_a(Stack *stack);

void stack_change_top_b(Stack *stack, Prec_rule rule, int shift_index);

void stack_pop(Stack *stack);

void stack_push(Stack *stack, Table_index symbol);

#endif