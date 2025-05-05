/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _ERROR_H
#define _ERROR_H

#define NO_ERROR 0
#define ERROR_LEXICAL_ANALYSIS 1
#define ERROR_SYNTAX_ANALYSIS 2
#define ERROR_SEMANTIC_DEFINITIONS 3
#define ERROR_SEMANTIC_ASSIGNMENT 4
#define ERROR_SEMANTIC_INCOMPATIBLE_TYPES 5
#define ERROR_SEMANTIC_TYPE_COMPABILITY 6
#define ERROR_SEMANTIC_OTHERS 7
#define ERROR_NOT_EXPECTED_NIL 8
#define ERROR_DIVIDING_BY_ZERO 9
#define ERROR_INTERN_TRANSLATOR 99

/**
 * funkci vytlaci errorovu hlasku na stderr
 */
void print_error_message( char * , int , char * , int );

#endif
