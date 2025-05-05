/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _ERROR_C
#define _ERROR_C

#include <stdio.h>
#include "error.h"

/**
 * Funkcia vytlaci chybovu hlasku na standardny error vystup
 * Sprava by mala byt specificka kde presne k chybe doslo,
 * typ chyby sa vytlaci podla chyboveho kodu
 */
void print_error_message(char *message, int error_code, char *file, int line)
{
    switch(error_code)
    {
        case NO_ERROR:
            break;
        case ERROR_LEXICAL_ANALYSIS:
            fprintf(stderr, "Chyba: Lexikalna analyza\n");
            break;
        case ERROR_SYNTAX_ANALYSIS:
            fprintf(stderr, "Chyba: Syntakticka analyza\n");
            break;
        case ERROR_SEMANTIC_DEFINITIONS:
            fprintf(stderr, "Chyba: Semanticka analyza - nedefinovana/nedeklarovana funkcia/premenna, pokus o redefiniciu\n");
            break;
        case ERROR_SEMANTIC_ASSIGNMENT:
            fprintf(stderr, "Chyba: Semanticka analyza - chyba v priradeni (typova nekompatibilita)\n");
            break;
        case ERROR_SEMANTIC_INCOMPATIBLE_TYPES:
            fprintf(stderr, "Chyba: Semanticka analyza - zly pocet parametrov prip navratovych hodnot\n");
            break;
        case ERROR_SEMANTIC_TYPE_COMPABILITY:
            fprintf(stderr, "Chyba: Semanticka analyza - nekompatibilita v relacnych operaciach\n");
            break;
        case ERROR_SEMANTIC_OTHERS:
            fprintf(stderr, "Chyba: Semanticka analyza kod: %d\n", ERROR_SEMANTIC_OTHERS);
            break;
        case ERROR_NOT_EXPECTED_NIL:
            fprintf(stderr, "Chyba: Neocakavana hodnota NIL\n");
            break;
        case ERROR_DIVIDING_BY_ZERO:
            fprintf(stderr, "Chyba: Delenie nulou\n");
            break;
        case ERROR_INTERN_TRANSLATOR:
            fprintf(stderr, "Chyba: Interna chyba prekladaca\n");
            break;
    }
    fprintf(stderr, "%s\n", message);
    fprintf(stderr, "Chyba nastala v subore %s, na riadku: %d\n", file, line);
}

#endif
