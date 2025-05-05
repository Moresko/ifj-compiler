/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _COMPILER_C
#define _COMPILER_C

#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "scanner.h"
#include "error.h"

#define TEST_FILE "./tests/t_kubo_temp"//"./tests/t_np1"

int main(){
    FILE *source_file;

    //source_file = stdin;
    source_file = fopen(TEST_FILE, "r");
    if (source_file == NULL){
        print_error_message("Neuspesne nacitanie zo standardneho vstupu", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        return ERROR_INTERN_TRANSLATOR;
    }

    set_file(source_file);

    int return_code = analyse();

    printf("\n\nTESTOVANY SUBOR: %s\n", TEST_FILE);

    fclose(source_file);

	return return_code;
}

#endif
