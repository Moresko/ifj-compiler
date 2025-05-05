/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _TEST_UTILS_H
#define _TEST_UTILS_H

#include <stdio.h>
#include "../scanner.h"
#include "../symtable.h"
#include "../parser.h"
#include "../stack.h"

/**
 * Funkcia vytlaci typ tokenu a jeho atribut(ak nejaky je) na stdout z scanner.h
 */
void print_token(TToken *token);

/**
 * Funkcia vytlaci prislusny Data_type z symtable.h
 */
void print_data_type(Data_type type);

/**
 * Funkcia vytlaci prislusny Parser_State z parser.h
 */
void print_state(Parser_State state);

/**
 * Funkcia vytlaci table index z stack.h
 */
void print_table_index(Table_index tab_in);

#endif
