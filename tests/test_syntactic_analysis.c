/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021 - Testovanie syntaktickej a semantickej
 *
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _TEST_SYNT_ANALYSIS
#define _TEST_SYNT_ANALYSIS

#include <stdlib.h>
#include "../parser.h"
#include "../scanner.h"
#include "../error.h"
#include "test_utils.h"

#define TEST_FILE "./tests/t_np1"

int main(){
    FILE *source_file;

    source_file = fopen(TEST_FILE, "r");
    if(source_file == NULL){
        print_error_message("Nepodarilo sa otvorit subor", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        return ERROR_INTERN_TRANSLATOR;
    }

    set_file(source_file);

    // Hlavne telo testovacieho programu
    int return_code = analyse();

    printf("\n");
    if(return_code == 0){
        printf("Vsetko dobre *fico voice*\n");
    }else{
        printf("Test zlyhal, chybovy kod: %d\n", return_code);
    }

    printf("Testovany subor: %s\n", TEST_FILE);

    fclose(source_file);

    return 0;
}

#endif