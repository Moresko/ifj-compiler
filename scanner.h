/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021 - Lexikalna analyza
 *
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _SCANNER_H
#define _SCANNER_H

#include <stdio.h>
#include <stdbool.h>

typedef enum{
	TOKEN_TYPE_EMPTY,

	TOKEN_TYPE_KEYWORD,
	TOKEN_TYPE_IDENTIFIER,
	TOKEN_TYPE_INT,
	TOKEN_TYPE_DOUBLE,
	TOKEN_TYPE_STRING,

	TOKEN_TYPE_OPERATOR_PLUS,
	TOKEN_TYPE_OPERATOR_MINUS,
	TOKEN_TYPE_OPERATOR_MULTIPLY,
	TOKEN_TYPE_OPERATOR_DIVIDE,
	TOKEN_TYPE_OPERATOR_INT_DIVIDE,

	TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER,
	TOKEN_TYPE_RELATIONAL_OPERATOR_LESS,
	TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL,
	TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL,
	TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL,
	TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL,

	TOKEN_TYPE_CONCATENATION,
	TOKEN_TYPE_ASSIGNMENT,
	TOKEN_TYPE_LEFT_BRACKET,
	TOKEN_TYPE_RIGHT_BRACKET,
	TOKEN_TYPE_COMMA,
	TOKEN_TYPE_COLON,
	TOKEN_TYPE_HASHTAG,

	TOKEN_TYPE_EOF,
} Token_type;

typedef struct{
	Token_type type;
	char *value;
	int length;
} TToken;

/**
 * Funkcia vracia navratovy kod podla toho ci doslo k chybe alebo nie
 * funkcia prepise hodnoty tokenu, teda ukazovatel na token sa stane po navrate funkcie vyslednym tokenom
 */
int get_next_token(TToken *token);

void set_file(FILE *f);

void return_token(TToken *token);

#endif
