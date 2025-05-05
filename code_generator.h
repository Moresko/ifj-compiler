/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 * Jakub Kasem (xkasem02)
 *
 */

#ifndef _CODE_GENERATOR_H
#define _CODE_GENERATOR_H

#include <stdbool.h>
#include "scanner.h"
#include "stack.h"

//void build_function_write(TToken token);
void build_function_write();

void build_function_reads(char *id);

void build_function_readi(char *id);

void build_function_readn(char *id);

void build_function_tointeger(char *id, TToken token);

void generator_start();

void generate_file_header();

void generate_function_start(char *func_id);

void generate_function_retval();

void generate_function_end();

void generate_function_call(char *func_id, bool to_main_body);

void generate_local_var(char *id);

void generate_assign_retval(TToken token);

void generate_assign_var(char *id, TToken token);

void generate_if_start();

void generate_if_statement();

void generate_if_cond_check();

void generate_if_true_part();

void generate_if_true_end();

void generate_if_false_part();

void generate_if_end();

void generate_while_head();

void generate_while_start();

void generate_while_end();

void generate_term_value(TToken token);

void generate_push(TToken token, Stack *stack);

void generate_pop(char *id);

void generate_concat();

void generate_length();

void push_op_until_left(Stack *stack);

void generate_stack_operations(Table_index rule);

void print_to_output();

int get_if_index();

#endif
