/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 * Jakub Kasem (xkasem02)
 *
 */


#ifndef _CODE_GENERATOR_C
#define _CODE_GENERATOR_C

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "code_generator.h"
#include "parser.h"
#include "symtable.h"
#include "./tests/test_utils.h"

#define MAX_CHARS 150

char output[3000];
char function_definitions[1500];
char main_body[1500];

// Globalne premenne
static int if_index = 0;
static int param_index = 0;
static int help_var_index = 0;

void add_to_output(char *sentence)
{
    strcat(output, sentence);
}

void add_to_func_definitions(char *sentence)
{
    strcat(function_definitions, sentence);
}

void add_to_main_body(char *sentence)
{
    strcat(main_body, sentence);
}

// ------------------------------------------------------------------------
// VSTAVANE FUNKCIE
// reads() : string
void build_function_reads(char *id)
{
    add_to_func_definitions("READ LF@");
    add_to_func_definitions(id);
    add_to_func_definitions(" string\n");
}

// readi() : integer
void build_function_readi(char *id)
{
    add_to_func_definitions("READ LF@");
    add_to_func_definitions(id);
    add_to_func_definitions(" int\n");
}

// readn() : number
void build_function_readn(char *id)
{
    add_to_func_definitions("READ LF@");
    add_to_func_definitions(id);
    add_to_func_definitions(" float\n");
}

void build_function_write()
{
    char c[5];
    sprintf(c, "%d", help_var_index++);
    add_to_func_definitions("DEFVAR LF@$help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");

    add_to_func_definitions("POPS LF@$help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");

    add_to_func_definitions("WRITE LF@$help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void build_function_tointeger(char *id, TToken token)
{
    add_to_func_definitions("PUSHS ");
    generate_term_value(token);
    add_to_func_definitions("\n");
    add_to_func_definitions("FLOAT2INTS");
    add_to_func_definitions("POPS LF@");
    add_to_func_definitions(id);
    add_to_func_definitions("\n");
}

void build_function_substr()
{

}

void build_function_ord()
{

}

void build_function_chr()
{

}

// ------------------------------------------------------------------------
// FUNKCIE NA GENEROVANIE KODU
void generate_file_header()
{
    add_to_output("# Vysledny preklad jazyka ifj21 do medzijazyka ifjcode21\n");
    add_to_output(".IFJcode21\n");
    add_to_output("JUMP $$main\n");

    add_to_main_body("\n# Hlavne telo programu\n");
    add_to_main_body("LABEL $$main\n");
}

void generator_start()
{
    generate_file_header();
}

void generate_function_start(char *func_id)
{
    add_to_output("\n# Zaciatok funkcie "); 
    add_to_output(func_id);
    add_to_output("\n");

    add_to_output("LABEL $"); 
    add_to_output(func_id);
    add_to_output("\n");

    add_to_output("PUSHFRAME\n");
}

void generate_function_retval()
{
    static int retval_index = 0;
    add_to_func_definitions("DEFVAR LF@$retval");
    char c[5];
    sprintf(c, "%d", retval_index);
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void generate_function_end()
{
    add_to_func_definitions("POPFRAME\n");
    add_to_func_definitions("RETURN\n");
}

void generate_function_call(char *func_id, bool to_main_body)
{
    if(to_main_body)
    {
        add_to_main_body("CREATEFRAME\n");
        add_to_main_body("CALL $"); 
        add_to_main_body(func_id);
        add_to_main_body("\n");
    }
    else
    {
        add_to_func_definitions("CALL $"); 
        add_to_func_definitions(func_id);
        add_to_func_definitions("\n");
    }
}

void generate_funcition_pass_param(TToken token)
{
    char c[5];
    sprintf(c, "%d", param_index++);
    add_to_func_definitions("CREATEFRAME\n");
    add_to_func_definitions("DEFVAR TF@$param");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
    add_to_func_definitions("MOVE TF@$param");
    add_to_func_definitions(c);
    add_to_func_definitions(" ");
    generate_term_value(token);
    add_to_func_definitions("\n");
}

void generate_local_var(char *id)
{
    add_to_func_definitions("DEFVAR LF@");
    add_to_func_definitions(id);
    add_to_func_definitions("\n");
}

void generate_assign_retval(TToken token) // ???????????????????????????????????????????????????????????
{
    add_to_func_definitions("MOVE LF@$retval");
    char c[5];
    sprintf(c, "%d", if_index);
    add_to_func_definitions(c);
    add_to_func_definitions(" ");
    generate_term_value(token);
    add_to_func_definitions("\n");
}

void generate_assign_var(char *id, TToken token)
{
    add_to_func_definitions("MOVE LF@");
    add_to_func_definitions(id);
    add_to_func_definitions(" ");
    generate_term_value(token);
    add_to_func_definitions("\n");
}

void generate_assign_from_func(char *id)
{
    static int retval_index = 0;
    add_to_func_definitions("MOVE LF@$");
    add_to_func_definitions(id);
    add_to_func_definitions(" TF@$retval");
    char c[5];
    sprintf(c, "%d", retval_index);
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void generate_assign_var_value() // ???????????????????????????????????????????
{
    add_to_func_definitions("int@5\n");
}

void generate_if_start()
{
    char c[5];
    sprintf(c, "%d", if_index);
    add_to_func_definitions("DEFVAR LF@$if_result");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void generate_if_statement(TToken token)
{
    char c[5];
    sprintf(c, "%d", if_index);
    add_to_func_definitions("MOVE LF@$if_statement");
    add_to_func_definitions(c);
    add_to_func_definitions(" ");
    generate_term_value(token);
    add_to_func_definitions("\n");

    add_to_func_definitions("EQ LF@$if_result");
    add_to_func_definitions(c);
    add_to_func_definitions(" LF$if_statement");
    add_to_func_definitions(c);
    add_to_func_definitions(" nil@nil\n");
    add_to_func_definitions("NOT LF@if_result");
    add_to_func_definitions(c);
    add_to_func_definitions(" LF@if_result");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void generate_if_cond_check()
{
    char c[5];
    sprintf(c, "%d", if_index);

    add_to_func_definitions("JUMPIFNEQ $if_false");
    add_to_func_definitions(c);
    add_to_func_definitions(" LF@$if_result");
    add_to_func_definitions(c);
    add_to_func_definitions(" bool@true\n");
}

void generate_if_true_end()
{
    char c[5];
    sprintf(c, "%d", if_index);
    add_to_func_definitions("JUMP $if_end");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void generate_if_false_part()
{
    char c[5];
    sprintf(c, "%d", if_index);
    add_to_func_definitions("LABEL $if_false");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
}

void generate_if_end()
{
    char c[5];
    sprintf(c, "%d", if_index++);
    add_to_func_definitions("LABEL $if_end");
    add_to_func_definitions(c);
    add_to_func_definitions("\n"); 
}

void generate_while_head()
{
    
}

void generate_while_start()
{
    
}

void generate_while_end()
{
    
}

void generate_term_value(TToken token)
{
    char str[250];

    switch(token.type)
    {
        case TOKEN_TYPE_INT:
            sprintf(str, "%d", atoi(token.value));
            add_to_func_definitions("int@"); 
            add_to_func_definitions(str);
            break;
        case TOKEN_TYPE_DOUBLE:
            add_to_func_definitions("float@");
            sprintf(str, "%a", atof(token.value));
            add_to_func_definitions(str);
            break;
        case TOKEN_TYPE_STRING:
            strcpy(str, token.value);
            add_to_func_definitions("string@");
            char help_str[5];
            for(int i = 0; token.value[i] != '\0'; i++)
            {
                char c = token.value[i];
                if(c == '\\' || c == '#' || c <= 32)
                {
                    add_to_func_definitions("\\");
                    sprintf(help_str, "%03d", c);
                    add_to_func_definitions(help_str);
                }
                else
                {
                    sprintf(help_str, "%c", c);
                    add_to_func_definitions(help_str);
                }
            }
            break;
        case TOKEN_TYPE_IDENTIFIER:
            add_to_func_definitions("LF@"); 
            add_to_func_definitions(token.value);
            break;
        case TOKEN_TYPE_KEYWORD:
            if(!strcmp(token.value, "nil"))
                add_to_func_definitions("nil@nil");
            break;
        default:
            break;
    }
}

int rule_priority(Table_index rule)
{
    switch(rule)
    {
        case EXPR_LENGTH:
            return 5;
            break;
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_DIV_INT:
            return 4;
            break;
        case EXPR_PLUS:
        case EXPR_MINUS:
            return 3;
            break;
        case EXPR_CONC:
            return 2;
            break;
        case EXPR_LESS:
        case EXPR_LESS_EQ:
        case EXPR_GREATER:
        case EXPR_GREATER_EQ:
        case EXPR_NEQ:
        case EXPR_EQ:
            return 1;
            break;
        default:
            break;
    }

    return 0;
}

void push_op_until_left(Stack *stack)
{
    Table_index top;
    top = stack_top(stack);
    while(!stack_is_empty(stack) && top != EXPR_LEFT_BRACKET)
    {
        top = stack_top(stack);
        generate_stack_operations(top);
        stack_pop(stack);
    }

    if(!stack_is_empty(stack)) stack_pop(stack);
}

void do_stack_operation(Stack *stack, Table_index rule)
{
    int priority_t = 0, priority_new = 0;

    priority_new = rule_priority(rule);

    if(stack_is_empty(stack))
        stack_push(stack, rule);
    else
    {
        Table_index top;
        top = stack_top(stack);

        priority_t = rule_priority(top);

        if(top == EXPR_LEFT_BRACKET || priority_t < priority_new)
            stack_push(stack, rule);
        else
        {
            generate_stack_operations(top);
            stack_pop(stack);
            do_stack_operation(stack, rule);
        }
    }
}

void generate_push(TToken token, Stack *stack)
{
    if((token.type == TOKEN_TYPE_KEYWORD && !strcmp(token.value, "nil")) ||
        token.type == TOKEN_TYPE_IDENTIFIER ||
        token.type == TOKEN_TYPE_INT ||
        token.type == TOKEN_TYPE_DOUBLE ||
        token.type == TOKEN_TYPE_STRING)
    {
        // Ak je id tak pushujeme na zasobnik
        add_to_func_definitions("PUSHS ");
        generate_term_value(token);
        add_to_func_definitions("\n");
    }
    else if(token.type == TOKEN_TYPE_LEFT_BRACKET)
    {
        stack_push(stack, EXPR_LEFT_BRACKET);
    }
    else if(token.type == TOKEN_TYPE_RIGHT_BRACKET)
    {
        push_op_until_left(stack);
    }
    else if(token.type == TOKEN_TYPE_OPERATOR_PLUS) do_stack_operation(stack, EXPR_PLUS);
    else if(token.type == TOKEN_TYPE_OPERATOR_MINUS) do_stack_operation(stack, EXPR_MINUS);
    else if(token.type == TOKEN_TYPE_OPERATOR_MULTIPLY) do_stack_operation(stack, EXPR_MUL);
    else if(token.type == TOKEN_TYPE_OPERATOR_DIVIDE) do_stack_operation(stack, EXPR_DIV);
    else if(token.type == TOKEN_TYPE_OPERATOR_INT_DIVIDE) do_stack_operation(stack, EXPR_DIV_INT);
    else if(token.type == TOKEN_TYPE_CONCATENATION) do_stack_operation(stack, EXPR_CONC);
    else if(token.type == TOKEN_TYPE_HASHTAG) do_stack_operation(stack, EXPR_LENGTH);

    else if(token.type == TOKEN_TYPE_RELATIONAL_OPERATOR_LESS) do_stack_operation(stack, EXPR_LESS);
    else if(token.type == TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL) do_stack_operation(stack, EXPR_LESS_EQ);
    else if(token.type == TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER) do_stack_operation(stack, EXPR_GREATER);
    else if(token.type == TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL) do_stack_operation(stack, EXPR_GREATER_EQ);
    else if(token.type == TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL) do_stack_operation(stack, EXPR_EQ);
    else if(token.type == TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL) do_stack_operation(stack, EXPR_NEQ);
}

void generate_pop(char *id)
{
    add_to_func_definitions("POPS LF@");
    add_to_func_definitions(id);
    add_to_func_definitions("\n");
}

void generate_concat()
{
    static bool was_used = false;

    if (!was_used)
    {
        add_to_func_definitions("DEFVAR LF@$concat_help_var0\n");
        add_to_func_definitions("DEFVAR LF@$concat_help_var1\n");
        was_used = true;
    }


    add_to_func_definitions("POPS LF@$concat_help_var1\n");
    add_to_func_definitions("POPS LF@$concat_help_var0\n");
    add_to_func_definitions("CONCAT LF@$concat_help_var0 LF@$concat_help_var0 LF@$concat_help_var1\n");
    add_to_func_definitions("PUSHS LF@$concat_help_var0\n");
}

void generate_length()
{
    static int length_call_counter = 0;

    char c[5];
    sprintf(c, "%d", length_call_counter++);
    add_to_func_definitions("DEFVAR LF@$length_help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
    sprintf(c, "%d", length_call_counter++);
    add_to_func_definitions("DEFVAR LF@$length_help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
    sprintf(c, "%d", length_call_counter - 1);
    add_to_func_definitions("POPS LF@$length_help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
    sprintf(c, "%d", length_call_counter - 2);
    add_to_func_definitions("STRLEN LF@$length_help_var");
    add_to_func_definitions(c);
    sprintf(c, "%d", length_call_counter - 1);
    add_to_func_definitions(" LF@$length_help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");
    sprintf(c, "%d", length_call_counter - 2);
    add_to_func_definitions("PUSHS LF@$length_help_var");
    add_to_func_definitions(c);
    add_to_func_definitions("\n");   
}

void generate_stack_operations(Table_index rule)
{
    switch(rule)
    {
        case EXPR_PLUS:
            add_to_func_definitions("ADDS\n");
            break;
        case EXPR_MINUS:
            add_to_func_definitions("SUBS\n");
            break;
        case EXPR_MUL:
            add_to_func_definitions("MULS\n");
            break;
        case EXPR_DIV:
            add_to_func_definitions("DIVS\n");
            break;
        case EXPR_DIV_INT:
            add_to_func_definitions("IDIVS\n");
            break;
        case EXPR_CONC:
            generate_concat();
            break;
        case EXPR_LESS:
            add_to_func_definitions("LTS\n");
            break;
        case EXPR_LESS_EQ:  //probably not working
            add_to_func_definitions("POPS GF\n");
            break;
        case EXPR_GREATER:
            add_to_func_definitions("GTS\n");
            break;
        case EXPR_GREATER_EQ: //not working
            break;
        case EXPR_EQ:
            add_to_func_definitions("EQS\n");
            break;
        case EXPR_NEQ:
            add_to_func_definitions("EQS\n");
            add_to_func_definitions("NOTS\n");
            break;
        case EXPR_LENGTH:
            generate_length(); 
            break;
        default:
            break;
    }
}

// ------------------------------------------------------------------------
// PRINT NA OUTPUT
void print_to_output()
{
    strcat(output, function_definitions);
    strcat(output, main_body);
    printf("%s", output);
}

// ------------------------------------------------------------------------
// POMOCNA FUNKCIA
int get_if_index()
{
    return if_index;
}

#endif
