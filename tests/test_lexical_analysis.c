/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021 - Testovanie lexikalnej analyzy
 *
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _TEST_LEX_ANALYSIS
#define _TEST_LEX_ANALYSIS

#include "../scanner.h"
#include "../error.h"
#include "test_utils.h"
#include <stdlib.h>

#define TEST_FILE "./tests/test2" // Subor na testovanie, program vypise postupne vsetky lexemy z tohto suboru

int print_all_tokens_test(){
    TToken *token;
    token = (TToken *)malloc(sizeof(TToken));
    if(token == NULL) return ERROR_INTERN_TRANSLATOR;

    int i = 0;
    while(token->type != TOKEN_TYPE_EOF){
        int return_code = get_next_token(token);
        if(return_code != 0) return return_code;

        // Vypisovanie tokenu
        printf("Token cislo %d: \n", ++i);
        print_token(token);

        // Test pre funkciu return token
        /*return_token(token);
        return_code = get_next_token(token);
        if(return_code != 0){
            printf("chybova situacia %d\n", return_code);
            return 1;
        }

        printf("Znovu ten isty token: \n");
        print_token(token);*/
    }

    free(token);

    return 0;
}

/**
 * Funkcia sa pokusi alokovat miesto pre token a otvorit subor ktory obsahuje zdrojovy kod ktory bude otestovany,
 * dalej sa nastavi tento subor pomocou funkcie set_file ktora je definovana v "scanner.h".
 * Postupne prechadzame token po tokene az kym nenarazime na token typu EOF ktory detekuje koniec testovacieho suboru.
 * Kazdy token postupne vypiseme na stdout pre lepsie nachadzanie pripadnej chyby
 */
int main(){
    FILE *source_file;

    source_file = fopen(TEST_FILE, "r");
    if(source_file == NULL){
        printf("Nepodarilo sa nacitat testovaci subor\n");
        return ERROR_INTERN_TRANSLATOR;
    }

    // Nastavi subor s ktorym ma pracovat scanner
    set_file(source_file);

    int return_code = print_all_tokens_test();
    if(return_code != 0)
        printf("Nastala chybova situacia: %d\n", return_code);
    else
        printf("TEST BOL USPESNY\n");

    printf("Testovany subor: %s\n", TEST_FILE);

    fclose(source_file);

    return 0;
}

#endif
