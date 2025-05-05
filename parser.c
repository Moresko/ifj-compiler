/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Jakub Kasem (xkasem02)
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _PARSER_C
#define _PARSER_C

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "error.h"
#include "symtable.h"
#include "parser.h"
#include "expression.h"
#include "code_generator.h"
#include "./tests/test_utils.h"

// DEKLARACIE FUNKCII PRE SYNTAKTICKU ANALYZU
bool func_prol(SPars_data *sdata);
bool func_prog(SPars_data *sdata);
bool func_return_func_var(SPars_data *sdata);
bool func_params(SPars_data *sdata);
bool func_param_n(SPars_data *sdata);
bool func_statement(SPars_data *sdata);
bool func_assign_value(SPars_data *sdata);
bool func_id_n(SPars_data *sdata);
bool func_types(SPars_data *sdata);
bool func_type_n(SPars_data *sdata);
bool func_type(SPars_data *sdata, bool called_from_types);
bool func_assig_func(SPars_data *sdata);

// DEFINICIE POMOCNYCH FUNKCII
/**
 * Funkcia nacita token a overi ci to prebehlo v poriadku
 */
bool is_next_token_valid(SPars_data *sdata, char *file, int line){
    int result = get_next_token(sdata->token);
    if(result != 0){
        print_error_message("Chyba pri nacitani tokenu", result, file, line);
        sdata->return_code = result;
        return false;
    }

    return true;
}

bool insert_build_functions(SPars_data *sdata)
{
    TData symt_data;
    symt_data.function = create_function(true, "reads");
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_STRING)) return false;
    bst_insert(&(sdata->global_table), "reads", symt_data);

    symt_data.function = create_function(true, "readi");
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_INT)) return false;
    bst_insert(&(sdata->global_table), "readi", symt_data);

    symt_data.function = create_function(true, "readn");
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_NUMBER)) return false;
    bst_insert(&(sdata->global_table), "readn", symt_data);

    symt_data.function = create_function(true, "write");
    symt_data.function->matters_on_params_count = false;
    bst_insert(&(sdata->global_table), "write", symt_data);

    symt_data.function = create_function(true, "tointeger");
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_NUMBER)) return false;
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_INT)) return false;
    bst_insert(&(sdata->global_table), "tointeger", symt_data);

    symt_data.function = create_function(true, "substr");
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_STRING)) return false;
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_NUMBER)) return false;
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_NUMBER)) return false;
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_STRING)) return false;
    bst_insert(&(sdata->global_table), "substr", symt_data);

    symt_data.function = create_function(true, "ord");
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_STRING)) return false;
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_INT)) return false;
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_INT)) return false;
    bst_insert(&(sdata->global_table), "ord", symt_data);

    symt_data.function = create_function(true, "chr");
    if(!DLLTypesAddLast(symt_data.function->params, TYPE_INT)) return false;
    if(!DLLTypesAddLast(symt_data.function->return_types, TYPE_STRING)) return false;
    bst_insert(&(sdata->global_table), "chr", symt_data);

    return true;
}

// DEFINICIE FUNKCII SYNTAKTICKEJ ANALYZY

/**
 * Neterminal <prol>
 * 
 * Mozne postupnosti tokenov:
 * 1. REQUIRE "ifj21" <prog>     (1.pravidlo)
 * 
 */
bool func_prol(SPars_data *sdata)
{
    // Prikaz nacita dalsi token a skontroluje ho
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 1: Token musi byt KEYWORD_REQUIRE
    if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "require"))
    {
        // Token: KEYWORD_REQUIRE

        // Prikaz nacita dalsi token a skontroluje ho
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        // Zistujeme ci token je string "ifj21" (1.pravidlo)
        if(sdata->token->type == TOKEN_TYPE_STRING && !strcmp(sdata->token->value, "ifj21"))
        {
            // Token je string "ifj21"
            return func_prog(sdata);
        }
        // Chyba: Token nie je string "ifj21"
        else
        {
            print_error_message("<prol> token nie je string \"ifj121\"", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Chyba: Token nie je KEYWORD_REQUIRE
    else
    {
        print_error_message("<prol> token nie je KEYWORD_REQUIRE", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
        return false;
    }
}

/**
 * Neterminal <prog>
 * 
 * Mozne postupnosti tokenov:
 * 1. GLOBAL ID COLON FUNCTION ( <types> ) <return_func_var> <prog>             (2.pravidlo)
 * 2. FUNCTION ID ( <params> ) <return_func_var> <statement> END <prog>         (3.pravidlo)
 * 3. ID ( <expression> ) <prog>                                                (4.pravidlo)
 * 4. EOF                                                                       (5.pravidlo)
 * 
 */
bool func_prog(SPars_data *sdata)
{
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

    // Pravidlo 2: Token musi byt KEYWORD_GLOBAL
    if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "global"))
    {
        // Nastavenie stavu kvoli semantickej kontrole
        sdata->state = STATE_FUNCTION_DECLARATION;

        // Prikaz nacita a skontroluje dalsi token
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        // Token je ID, pokracuj
        if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
        {
            // SEMANTICKA KONTROLA
            // Skontrolujeme ci funkcia uz nebola deklarovana, ak nie vytvorime funkciu a vlozime ju do stromu
            TData data;

            if(bst_search(sdata->global_table, sdata->token->value, &data))
            {
                // Ak sme nasli funkciu s takymto menom v strome jedna sa o semanticku chybu (redeklaracia funkcie)
                sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                return false;
            }

            // Pridavanie funkcie do globalnej tabulky (tabulky funkcii)
            // Vytvori sa funkcia a boolean defined sa nastavi na false
            data.function = create_function(false, sdata->token->value);

            // Funkcia sa nastavi ako aktivna pre lahsiu pracu s nou
            sdata->active_function = data.function;

            // Vlozime kluc s nazvom funkcie do globalnej tabulky symbolov (tabulka funkcii)
            bst_insert(&(sdata->global_table), sdata->token->value, data);

            // Nastavenie ze sa idu nacitavat typy parametrov funkcie
            sdata->params_or_return_types = true;

            // SYNTAKTICKA KONTROLA
            //Prikaz nacita a skontroluje dalsi token
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

            // Token je COLON, pokracuj
            if (sdata->token->type == TOKEN_TYPE_COLON)
            {
                if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

                // Token je KEYWORD_FUNCTION
                if(sdata->token->type == TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "function"))
                {
                    //Prikaz nacita a skontroluje dalsi token
                    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                    // Token je LEFT_BRACKET, pokracuj
                    if (sdata->token->type == TOKEN_TYPE_LEFT_BRACKET)
                    {
                        // Token je <types>, pokracuj
                        if (func_types(sdata))
                        {
                            //Prikaz nacita a skontroluje dalsi token
                            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                            //Token je RBRACKET, pokracuj
                            if (sdata->token->type == TOKEN_TYPE_RIGHT_BRACKET)
                            {
                                //Token je <return_func_var>, pokracuj
                                if (func_return_func_var(sdata))
                                {
                                    return func_prog(sdata);
                                }
                                // Chyba: <return_func_var>
                                else
                                {
                                    return false;
                                }
                            }
                            // Chyba: Token nie je RBRACKET
                            else
                            {
                                print_error_message("<prog> Token nie je RIGHT_BRACKET pri pravidle 2", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                                return false;
                            }
                        }
                        // Chyba: <func_types>
                        else
                        {
                            return false;
                        }
                    }
                    // Chyba: Token nie je LEFT_BRACKET
                    else
                    {
                        print_error_message("<prog> Token nie je LEFT_BRACKET pri pravidle 2", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                        return false;
                    }
                }
                // Chyba: Token nie je KEYWORD_FUNCTION
                else
                {
                    print_error_message("<prog> Token nie je KEYWORD_FUNCTION pri pravidle 2", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                    sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                    return false;
                }
            }
            // Chyba: Token nie je COLON
            else
            {
                print_error_message("<prog> Token nie je COLON pri pravidle 2", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: Token nie je ID
        else
        {
            print_error_message("<prog> Token nie je ID pri pravidle 2", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }      
    }
    // Pravidlo 3: Token musi byt KEYWORD_FUNCTION
    else if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "function"))
    {
        // Nastavenie stavu kvoli semantickej kontrole
        sdata->state = STATE_FUNCTION_DEFINITION;

        // Prikaz nacita a skontroluje dalsi token
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        // Token je ID, pokracuj
        if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
        {
            // SEMANTICKA KONTROLA
            // Skontrolujeme ci funkcia uz nebola nahodou definovana
            TData data;

            // Zistime ci funkcia uz bola definovana alebo deklarovana
            if(bst_search(sdata->global_table, sdata->token->value, &data))
            {
                // Funkcia uz existuje

                // Nastavime funkciu ako aktivnu pre lahsiu pracu s nou
                sdata->active_function = data.function;

                // Zistime ci nejde o redefinicu funkcie
                if(sdata->active_function->defined)
                {
                    // Chyba: Redefinicia funkcie
                    print_error_message("<prog> Redefinicia funkcie pri pravidle 3", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                    sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                    return false;
                }

                // Posunieme aktivnost zoznamov typov na prve prvky
                DLLTypesActiveFirst(sdata->active_function->params);
                DLLTypesActiveFirst(sdata->active_function->return_types);
            }
            // Funkcia este nie je v globalnej tabulke symbolov (tabulke funkcii)
            else
            {
                // Vlozime funkciu do globalnej tabulky symbolov (tabulka funkcii)
                data.function = create_function(false, sdata->token->value);

                // Skontrolujeme ci vytvorenie funkcie bolo uspesne
                if(data.function == NULL)
                {
                    // Chyba: Neuspesna alokacia pamate
                    print_error_message("<prog> Chyba alokacie funkcie pri pravidle 3", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }
                sdata->active_function = data.function;
                bst_insert(&(sdata->global_table), sdata->token->value, data);

                // Nastavime ze najskor sa budu kontrolovat pripadne vkladat typy parametrov
                sdata->params_or_return_types = true;
            }

            // Generovanie zaciatku funkcie
            generate_function_start(sdata->token->value);

            // SYNTAKTICKA KONTROLA
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

            // Token je LEFT_BRACKET, pokracuj
            if (sdata->token->type == TOKEN_TYPE_LEFT_BRACKET)
            {
                // SEMANTICKA KONTROLA
                // Vytvorime a inicializujeme novu lokalnu tabulku symbolov (tabulka premennych), a vlozime ju nakoniec zoznamu tabuliek symbolov
                Sym_table local_table;
                bst_init_table(&local_table);

                // Skontrolujeme ci sa podarila alokacia tabulky symbolov
                if(!DLLSymtAddLast(&(sdata->list_of_symtables), local_table))
                {
                    // Chyba: Neuspesna alokacia
                    print_error_message("<prog> Neuspesna alokacia pri pravidle 3", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }

                // Nastavime aktivnost tabulky symbolov na poslednu tabulku
                DLLSymtActiveLast(&(sdata->list_of_symtables));

                // SYNTAKTICKA KONTROLA
                // Token je <params>, pokracuj
                if (func_params(sdata))
                {
                    // Prikaz nacita a skontroluje dalsi token
                    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                    //Token je RIGHT_BRACKET, pokracuj
                    if (sdata->token->type == TOKEN_TYPE_RIGHT_BRACKET)
                    {
                        // Token je <return_func_var>, pokracuj
                        if (func_return_func_var(sdata))
                        {
                            // Nastavime funkciu uz ako definovanu
                            sdata->active_function->defined = true;

                            //Token je <statement>, pokracuj
                            if (func_statement(sdata))
                            {
                                if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

                                // Token je END, pokracuj
                                if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "end"))
                                {
                                    // Vygenerovanie konca funkcie
                                    generate_function_end();
                                    return func_prog(sdata);
                                }
                                // Chyba: Token nie je END
                                else
                                {
                                    print_error_message("<prog> Token nie je END pri pravidle 3", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                                    sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                                    return false;
                                }
                            }
                            // Chyba: <statement>
                            else
                            {
                                return false;
                            }
                        }
                        // Chyba: <return_func_var>
                        else
                        {
                            return false;
                        }
                    }
                    // Chyba: Token nie je RIGHT_BRACKET
                    else
                    {
                        print_error_message("<prog> Token nie je RIGHT_BRACKET pri pravidle 3", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                        return false;
                    }
                }
                // Chyba: <func_params>
                else
                {
                    return false;
                }   
            }
            // Chyba: Token nie je LEFT_BRACKET
            else
            {
                print_error_message("<prog> Token nie je LEFT_BRACKET pri pravidle 3", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: Token nie je ID
        else
        {
            print_error_message("<prog> Token nie je ID pri pravidle 3", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Pravidlo 4: Token musi byt ID funkcie
    else if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
    {
        // Nastavenie stavu kvoli semantickej kontrole
        sdata->state = STATE_FUNCTION_CALL;

        // SEMANTICKA KONTROLA: Skontrolujeme ci sedia typy parametrov volanej funkcie main
        TData data;

        // Skontrolujeme ci funkcia je v globalnej tabulke symbolov
        if(bst_search(sdata->global_table, sdata->token->value, &data))
        {
            // Funkcia main bola najdena, nastavime ju ako aktivnu a aktivny prvok zoznamu parametrov nastavime na zaciatok
            sdata->active_function = data.function;
            DLLTypesActiveFirst(sdata->active_function->params);
            if(!DLLTypesIsEmpty(sdata->active_function->return_types))
            {
                print_error_message("Volanie funkcie ktora ma navratovu hodnotu v <prog>", ERROR_SEMANTIC_OTHERS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SEMANTIC_OTHERS;
                return false;
            }
        }
        // Chyba: Funkcia main nebola najdena
        else
        {
            print_error_message("<prog> Volanie nedefinovanej funkcie pri pravidle 4", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
            return false;
        }

        // Vygeneruje sa zavolanie funkcie
        generate_function_call(sdata->token->value, true);

        // SYNTAKTICKA KONTROLA
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   //Prikaz nacita a skontroluje dalsi token

        // Token je LEFT_BRACKET, pokracuj
        if (sdata->token->type == TOKEN_TYPE_LEFT_BRACKET)
        {
            // Token je <expression>, pokracuj
            if(func_expression(sdata))
            {
                if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   //Prikaz nacita a skontroluje dalsi token

                //Token je RIGHT_BRACKET, pokracuj
                if (sdata->token->type == TOKEN_TYPE_RIGHT_BRACKET)
                {
                    return func_prog(sdata);
                }
                // Chyba: Token nie je RIGHT_BRACKET
                else
                {
                    print_error_message("<prog> Token nie je RIGHT_BRACKET pri pravidle 4", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                    sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                    return false;
                }
            }
            // Chyba: <func_expression>
            else
            {
                return false;
            }
        }
        // Chyba: Token nie je LEFT_BRACKET
        else
        {
            print_error_message("<prog> Token nie je LEFT_BRACKET pri pravidle 4", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Pravidlo 5: Token musi byt EOF
    else if (sdata->token->type == TOKEN_TYPE_EOF)
    {
        return true;
    }
    // Chyba: Neexistuje prechod
    else
    {
        print_error_message("<prog> Neexistuje prechod pre token", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
        return false;
    }

    return false;
}

/**
 * Neterminal <return_func_var>
 * 
 * Mozne postupnosti tokenov:
 * 1. COLON <type> <type_n>     (6. pravidlo)
 * 2. ε                         (7. pravidlo)
 * 
 */
bool func_return_func_var(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 6: Token musi byt COLON
    if (sdata->token->type == TOKEN_TYPE_COLON)
    {
        // Prepnutie do nacitavania navratovych hodnot
        sdata->params_or_return_types = false;

        //Token je <type>, pokracuj
        if (func_type(sdata, false))
        {
            // Vrati sa uspesnost <type_n>
            return func_type_n(sdata);
        }
        //  Chyba: <func_type>
        else
        {
            return false;
        }
    }
    // Pravidlo 7: Epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }
}

/**
 * Neterminal <params>
 * 
 * Mozne postupnosti tokenov:
 * 1. ID COLON <type> <param_n>     (8.pravidlo)
 * 2. ε                             (9.pravidlo)
 */
bool func_params(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 8: Token musi byt ID
    if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
    {
        // SEMANTICKA KONTROLA
        // Sme v stave ked definujeme funkciu
        if(sdata->state == STATE_FUNCTION_DEFINITION)
        {
            TData data;

            // Skontrolujeme ci hodnota uz nie je v lokalnej tabulke na poslednej urovni
            if(bst_search(sdata->list_of_symtables.active->data, sdata->token->value, &data))
            {
                // Chyba: Redefinicia premennej
                print_error_message("<func_params> Redefinicia premennej pri pravidle 8", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                return false;
            }
            // Hodnota nie je v tabulke
            else
            {
                // Vlozime nazov prvku do tabulky
                data.variable = create_variable(TYPE_UNDEFINED, "", false, sdata->token->value);
                sdata->active_variable = data.variable;

                // Skontrolujeme ci vlozenie do lokalnej tabulky symbolov bolo uspesne
                if(!bst_insert(&(sdata->list_of_symtables.active->data), sdata->token->value, data))
                {
                    // Chyba: Neuspesna alokacia pamate
                    print_error_message("<func_params> Neuspesna alokacia pri pravidle 8", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }
            }
        }

        // SYNTAKTICKA KONTROLA
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

        // Token je COLON, pokracuj
        if (sdata->token->type == TOKEN_TYPE_COLON)
        {
            // Token je <type>, pokracuj
            if (func_type(sdata, false))
            {
                return func_param_n(sdata);
            }
            // Chyba: <func_type>
            else
            {
                return false;
            }
        }
        // Chyba: Token nie je COLON
        else
        {
            print_error_message("<func_params> Token nie je COLON pri pravidle 8", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Pravidlo 9: epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }
}

/**
 * Neterminal <param_n>
 * 
 * Mozne postupnosti tokenov:
 * 1. COMMA ID COLON <type> <param_n>   (10. pravidlo)
 * 2. ε                                 (11. pravidlo)
 * 
 */
bool func_param_n(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 10: Token musi byt COMMA
    if (sdata->token->type == TOKEN_TYPE_COMMA)
    {
        // Prikaz nacita a skontroluje dalsi token
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        //Token je ID, pokracuj
        if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
        {
            // SEMANTICKA KONTROLA
            TData data;

            // Skontrolujeme ci premenna s rovnakym nazvom uz nie je nahodou v tabulke
            if(bst_search(sdata->list_of_symtables.active->data, sdata->token->value, &data))
            {
                // Chyba: redefinicia premennej
                print_error_message("<func_param_n> Redefinicia premennej pri pravidle 10", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                return false;
            }
            else
            {
                // Premenna nie je v tabulke
                data.variable = create_variable(TYPE_UNDEFINED, "", false, sdata->token->value);
                sdata->active_variable = data.variable;

                // Kontrola ci vkladanie do lokalnej tabulky symbolov bolo uspesne
                if(!bst_insert(&(sdata->list_of_symtables.active->data), sdata->token->value, data))
                {
                    // Chyba: Neuspesna alokacia pamate
                    print_error_message("<func_param_n> Neuspesna alokacia pri pravidle 10", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }
            }

            // SYNTAKTICKA KONTROLA
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

            // Token je COLON, pokracuj
            if (sdata->token->type == TOKEN_TYPE_COLON)
            {
                //Token je <type>, pokracuj
                if (func_type(sdata, false))
                {
                    return func_param_n(sdata);
                }
                // Chyba: <func_type>
                else
                {
                    return false;
                }
            }
            // Chyba: Token nie je COLON
            else
            {
                print_error_message("<func_param_n> Token nie je COLON pri pravidle 10", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            } 
        }
        // Chyba: Token nie je ID
        else
        {
            print_error_message("<func_param_n> Token nie je ID pri pravidle 10", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Pravidlo 11: epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }
}

/**
 * Neterminal <statement>
 * 
 * Mozne postupnosti tokenov:
 * 1. ε                                                                     (12. pravidlo)
 * 2. LOCAL ID : <type> <assign_value> <statement>                          (13. pravidlo)
 * 3. RETURN <expression> <statement>                                       (14. pravidlo)
 * 4. ID <assig_func> <statement>                                           (15. pravidlo)
 * 5. IF <expression> THEN <statement> ELSE <statement> END <statement>     (16. pravidlo)
 * 6. WHILE <expression> DO <statement> END <statement>                     (17. pravidlo)
 */
bool func_statement(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 13: Token musi byt LOCAL
    if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "local"))
    {
        // Prikaz nacita a skontroluje dalsi token
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        //Token je ID, pokracuj
        if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
        {
            // SEMANTICKA KONTROLA
            // Nastavenie stavu kvoli semantickej kontrole
            sdata->state = STATE_ID_INIT;

            TData data;
            // Skontrolujeme ci premenna uz nebola definovana
            if(bst_search(sdata->list_of_symtables.active->data, sdata->token->value, &data))
            {
                // Chyba: redefinicia premennej
                print_error_message("<statement> Redefinicia premennej pri pravidle 13", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                return false;
            }
            // Premenna nie je v tabulke
            else
            {
                // Premenna nie je v tabulke, vytvorime premennu a vlozime ju do lokalnej tabulky symbolov
                data.variable = create_variable(TYPE_UNDEFINED, "", false, sdata->token->value);
                sdata->active_variable = data.variable;

                // Skontrolujeme ci vkladanie do tabulky symbolov bolo uspesne
                if(!bst_insert(&(sdata->list_of_symtables.active->data), sdata->token->value, data))
                {
                    // Chyba: Neuspesna alokacia pamate
                    print_error_message("<statement> Neuspesna alokacia pri pravidle 13", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }
            }

            // Vygenerujeme si inicializaciu premennej
            generate_local_var(sdata->token->value);

            // SYNTAKTICKA KONTROLA
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;   // Prikaz nacita a skontroluje dalsi token

            // Token je COLON, pokracuj
            if (sdata->token->type == TOKEN_TYPE_COLON)
            {
                //Token je <type>, pokracuj
                if (func_type(sdata, false))
                {
                    //Token je <assign_value>, pokracuj
                    if (func_assign_value(sdata))
                    {
                        return func_statement(sdata);
                    }
                    // Chyba: <func_assign_value>
                    else
                    {
                        return false;
                    }
                }
                // Chyba: <func_type>
                else
                {
                    return false;
                }
            }
            // Chyba: Token nie je COLON
            else
            {
                print_error_message("<statement> Token nie je COLON pri pravidle 13", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: Token nie je ID
        else
        {
            print_error_message("<statement> Token nie je ID pri pravidle 13", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Pravidlo 14: Token musi byt RETURN
    else if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "return"))
    {
        // Nastavenie stavu kvoli semantickej kontrole
        sdata->state = STATE_RETURN;

        DLLTypesActiveFirst(sdata->active_function->return_types);

        //Token je <expression>, pokracuj
        if (func_expression(sdata))
        {
            return func_statement(sdata); 
        }
        // Chyba: <func_expression>
        else
        {
            return false;
        }
    }
    // Pravidlo 15: Token musi byt ID
    else if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
    {
        // Vycistime zoznam premennych aby sme do neho mohli pridavat hodnoty
        DLLVarDispose(&(sdata->list_of_variables));

        // Token je <func_assig_func>
        if(func_assig_func(sdata))
        {
            return func_statement(sdata);
        }
        // Chyba: <func_assig_func>
        else
        {
            return false;
        }
    }
    // Pravidlo 16: Token musi byt IF
    else if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "if"))
    {
        // Nastavenie stavu kvoli semantickej kontrole
        sdata->state = STATE_IF;

        // Generovanie zaciatku prikazu if
        generate_if_start();

        // Token je <expression>
        if(func_expression(sdata))
        {
            // Prikaz nacita a skontroluje dalsi token
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

            //Token je THEN, pokracuj
            if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "then"))
            {
                // SEMANTICKA KONTROLA
                // Vytvorime novu lokalnu tabulku symbolov 
                Sym_table symt_if;
                bst_init_table(&symt_if);

                // Pridame ju na koniec zoznamu lokalnych tabuliek symbolov
                // Overime ci pridanie bolo uspesne 
                if(!DLLSymtAddLast(&(sdata->list_of_symtables), symt_if))
                {
                    // Chyba: Neuspesna alokacia pamate
                    print_error_message("<statement> Neuspesna alokacia pri pravidle 16", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }

                // Nastavi poslednu tabulku symbolov v zozname ako aktivnu
                DLLSymtActiveLast(&(sdata->list_of_symtables));

                // Vygeneruj skokove instrukcie
                generate_if_cond_check();

                // SYNTAKTICKA KONTROLA
                //Token je <statement>, pokracuj
                if (func_statement(sdata))
                {
                    // Koniec bloku: Odstranime poslednu tabulku symbolov
                    DLLSymtRemoveLast(&(sdata->list_of_symtables));
                    // Nastavime aktivnost na poslednu tabulku
                    DLLSymtActiveLast(&(sdata->list_of_symtables));

                    // Prikaz nacita a skontroluje dalsi token
                    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                    //Token je ELSE, pokracuj
                    if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "else"))
                    {
                        // SEMANTICKA KONTROLA
                        // Vytvorime novu lokalnu tabulku symbolov 
                        Sym_table symt_else;
                        bst_init_table(&symt_else);

                        // Pridame ju na koniec zoznamu lokalnych tabuliek symbolov
                        // Overime ci pridanie bolo uspesne 
                        if(!DLLSymtAddLast(&(sdata->list_of_symtables), symt_else))
                        {
                            // Chyba: Neuspesna alokacia pamate
                            print_error_message("<statement> Neuspesna alokacia pri pravidle 16", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                            sdata->return_code = ERROR_INTERN_TRANSLATOR;
                            return false;
                        }

                        // Nastavi poslednu tabulku symbolov v zozname ako aktivnu
                        DLLSymtActiveLast(&(sdata->list_of_symtables));

                        // Vygeneruje sa prechod na else cast pripadne skok na koniec podmieneneho prikazu
                        generate_if_true_end();
                        generate_if_false_part();

                        // Token je <statement>, pokracuj
                        if (func_statement(sdata))
                        {
                            // Koniec bloku: Odstranime poslednu tabulku symbolov
                            DLLSymtRemoveLast(&(sdata->list_of_symtables));
                            // Nastavime aktivnost na poslednu tabulku
                            DLLSymtActiveLast(&(sdata->list_of_symtables));

                            // Prikaz nacita a skontroluje dalsi token
                            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                            //Token je END, pokracuj
                            if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "end"))
                            {
                                // Vygeneruje label pre koniec podmieneneho prikazu
                                generate_if_end();
                                return func_statement(sdata);
                            }
                            // Chyba: Token nie je END
                            else
                            {
                                print_error_message("<statement> Token nie je END pri pravidle 16", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                                return false;
                            }
                        }
                        // Chyba: <func_statement>
                        else
                        {
                            return false;
                        }
                    }
                    // Chyba: Token nie je ELSE
                    else
                    {
                        print_error_message("<statement> Token nie je ELSE pri pravidle 16", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                        return false;
                    }
                }
                // Chyba: <func_statement>
                else
                {
                    return false;
                }
            }
            // Chyba: Token nie je THEN
            else
            {
                print_error_message("<statement> Token nie je THEN pri pravidle 16", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: <func_expression>
        else
        {
            return false;
        }
    }
    // Pravidlo 17: Token musi byt WHILE
    else if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "while"))
    {
        // Nastavime state pre semanticku analyzu
        sdata->state = STATE_WHILE;

        // Token je <expression>
        if(func_expression(sdata))
        {
            // Prikaz nacita a skontroluje dalsi token
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

            // Token je DO, pokracuj
            if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "do"))
            {
                // SEMANTICKA KONTROLA
                // Vytvorime novu lokalnu tabulku symbolov 
                Sym_table symt_while;
                bst_init_table(&symt_while);

                // Pridame ju na koniec zoznamu lokalnych tabuliek symbolov
                // Overime ci pridanie bolo uspesne 
                if(!DLLSymtAddLast(&(sdata->list_of_symtables), symt_while))
                {
                    // Chyba: Neuspesna alokacia pamate
                    print_error_message("<statement> Neuspesna alokacia pri pravidle 17", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }

                // Nastavi poslednu tabulku symbolov v zozname ako aktivnu
                DLLSymtActiveLast(&(sdata->list_of_symtables));

                // SYNTAKTICKA KONTROLA
                // Token je <statement>, pokracuj
                if (func_statement(sdata))
                {
                    // Koniec bloku: Odstranime poslednu tabulku symbolov
                    DLLSymtRemoveLast(&(sdata->list_of_symtables));
                    // Nastavime aktivnost na poslednu tabulku
                    DLLSymtActiveLast(&(sdata->list_of_symtables));

                    // Prikaz nacita a skontroluje dalsi token
                    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                    //Token je END, pokracuj
                    if ((sdata->token->type == TOKEN_TYPE_KEYWORD) && !strcmp(sdata->token->value, "end"))
                    {
                        return func_statement(sdata);
                    }
                    // Chyba: Token nie je END
                    else
                    {
                        print_error_message("<statement> Token nie je END pri pravidle 17", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                        return false;
                    }
                }
                // Chyba: <func_statement>
                else
                {
                    return false;
                }
            }
            // Chyba: Token nie je DO
            else
            {
                print_error_message("<statement> Token nie je DO pri pravidle 17", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: <func_expression>
        else
        {
            return false;
        }
    }
    // Pravidlo 12: epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }
}

/**
 * Neterminal <assign_value>
 * 
 * Mozne postupnosti tokenov:
 * 1. ε                     (18. pravidlo)
 * 2. = <expression>        (19. pravidlo)
 */
bool func_assign_value(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 19: Token musi byt ASSIGNMENT
    if (sdata->token->type == TOKEN_TYPE_ASSIGNMENT)
    {
        return func_expression(sdata);
    }
    // Pravidlo 18: epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }

    return false;
}

/**
 * Neterminal <id_n>
 * 
 * Mozne postupnosti tokenov:
 * 1. ε                     (20.pravidlo)
 * 2. COMMA ID <id_n>       (21. pravidlo)
 *
 */
bool func_id_n(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 21: Token musi byt COMMA
    if (sdata->token->type == TOKEN_TYPE_COMMA)
    {
        // Prikaz nacita a skontroluje dalsi token
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        // Token je ID, pokracuj
        if (sdata->token->type == TOKEN_TYPE_IDENTIFIER)
        {
            // SEMANTICKA KONTROLA
            if(sdata->state == STATE_ASSIGNMENT_TO_ID)
            {
                TData data;

                // Skontrolujeme ci premenna je v niektorej lokalnej tabulke symbolov
                if(!is_var_in_symt_list(&(sdata->list_of_symtables), sdata->token->value, &data))
                {
                    // Chyba: Nedeklarovana premenna
                    print_error_message("<func_id_n> Nedeklarovana premenna pri pravidle 21", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                    sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                    return false;
                }

                // Premenna bola najdena, nastavi sa ako aktivna
                sdata->active_variable = data.variable;

                // Prida sa nakoniec zoznamu typov struktury
                DLLVarAddLast(&(sdata->list_of_variables), data.variable, sdata->token->value);
            }

            // SYNTAKTICKA KONTROLA
            return func_id_n(sdata);
        }
        // Chyba: Token nie je ID
        else
        {
            print_error_message("<func_id_n> Token nie je ID pri pravidle 21", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
            return false;
        }
    }
    // Pravidlo 20: epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }
}

/**                     
 * Neterminal <types>
 * 
 * Mozne postupnosti tokenov:
 * 1. ε                 (22. pravidlo)
 * 2. <type> <type_n>   (23. pravidlo)
 * 
 */
bool func_types(SPars_data *sdata)
{
    // Pravidlo 23: Token musi byt <type>
    if (func_type(sdata, true))
    {
        return func_type_n(sdata);
    }
    // Pravidlo 22: epsilon
    else
    {
        // Funkcia vrati false teda v nej doslo k chybe, treba vratit token lexikalnemu analyzatoru
        return_token(sdata->token);
        return true;
    }
}

/**
 * Neterminal <type_n>
 * 
 * Mozne postupnosti tokenov:
 * 1. ε                         (24. pravidlo)
 * 2. COMMA <type> <type_n>     (25. pravidlo)
 * 
 */
bool func_type_n(SPars_data *sdata)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 25: Token musi byt COMMA
    if (sdata->token->type == TOKEN_TYPE_COMMA)
    {
        // Ak je token <type>, pokracuj
        if (func_type(sdata, false))
        {
            return func_type_n(sdata);      
        }
        // Chyba: <func_type>
        else
        {
            return false;
        }
    }
    // Pravidlo 24: epsilon
    else
    {
        return_token(sdata->token);
        return true;
    }
}

/**
 * Neterminal <type>
 * 
 * Mozne postupnosti tokenov:
 * 1. INTEGER           (26. pravidlo)
 * 2. STRING            (27. pravidlo)
 * 3. NUMBER            (28. pravidlo)
 * 4. NIL               (29. pravidlo)
 * 
 */
bool func_type(SPars_data *sdata, bool called_from_types)
{
    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 26, 27, 28, 29: Token musi byt KEYWORD_INTEGER, KEYWORD_STRING, KEYWORD_NUMBER alebo KEYWORD_NIL 
    if(
        sdata->token->type == TOKEN_TYPE_KEYWORD && (
            !strcmp(sdata->token->value, "integer") ||
            !strcmp(sdata->token->value, "string") ||
            !strcmp(sdata->token->value, "number") ||
            !strcmp(sdata->token->value, "nil")
            )
        )
    {
        // SEMANTICKA KONTROLA
        if(sdata->state == STATE_FUNCTION_DECLARATION ||
            sdata->state == STATE_FUNCTION_DEFINITION ||
            sdata->state == STATE_ID_INIT)
        {
            // Typ KEYWORDu
            Data_type type = TYPE_UNDEFINED;

            // Nastavime type
            if(!strcmp(sdata->token->value, "integer"))
                type = TYPE_INT;
            else if(!strcmp(sdata->token->value, "number"))
                type = TYPE_NUMBER;
            else if(!strcmp(sdata->token->value, "string"))
                type = TYPE_STRING;
            else if(!strcmp(sdata->token->value, "nil"))
                type = TYPE_NIL;

            if(sdata->state == STATE_ID_INIT)
            {
                // Nastavenie typu premennej
                sdata->active_variable->type = type;
            }
            else
            {
                // Nastavime list podla toho ci ideme nacitavat parametre alebo navratove hodnoty
                DLLTypes *list = (sdata->params_or_return_types) ? sdata->active_function->params : sdata->active_function->return_types;

                // Skontrolujeme ci nahodou nejde uz o definovanu funkciu
                if(sdata->state == STATE_FUNCTION_DEFINITION)
                {
                    if(sdata->active_function->defined)
                    {
                        // Ziskame typ premennej 
                        Data_type dtype = DLLTypesGetActiveType(list);

                        // Posunieme aktivnost na dalsi prvok v zozname
                        DLLTypesActiveNext(list);

                        // Nastavime typ atributu v lokalnej tabulke symbolov
                        if(sdata->params_or_return_types)
                            sdata->active_variable->type = dtype;

                        // Skontrolujeme kompatibilnost typov deklarovanej a definovanej funkcie
                        if(dtype != type)
                        {
                            // Chyba: Nekompabilita typov
                            sdata->return_code = ERROR_SEMANTIC_INCOMPATIBLE_TYPES;
                            return false;
                        }

                        return true;
                    }
                    else
                    {
                        sdata->active_variable->type = type;
                    }
                }

                // Vlozime typ nakoniec zoznamu typov a skontrolujeme uspesnost vlozenia
                if(!DLLTypesAddLast(list, type))
                {
                    // Chyba: Neuspesna alokacia pamate
                    sdata->return_code = ERROR_INTERN_TRANSLATOR;
                    return false;
                }
            }
        }

        return true;
    }
    // Chyba: Token nie je KEYWORD_INTEGER, KEYWORD_STRING, KEYWORD_NUMBER and KEYWORD_NIL
    else
    {
        if(!called_from_types)
        {
            print_error_message("<type> Token nie je KEYWORD_INTEGER, KEYWORD_STRING, KEYWORD_NUMBER ani KEYWORD_NIL pri pravidlach 26-29", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SYNTAX_ANALYSIS;
        }

        return false;
    }
}

/**
 * Neterminal <assig_func>
 * 
 * Mozne postupnosti tokenov:
 * 1. <id_n> = <expression>                 (30.pravidlo)
 * 2. ( <expression> )                      (31.pravidlo)
 * 
 */
bool func_assig_func(SPars_data *sdata){
    // Ulozime si predosly token pre semanticku kontrolu
    TToken previous_token = *(sdata->token);

    // Prikaz nacita a skontroluje dalsi token
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Pravidlo 31: Token musi byt LEFT_BRACKET
    if(sdata->token->type == TOKEN_TYPE_LEFT_BRACKET)
    {
        // SEMANTICKA KONTROLA
        // Nastavenie stavu kvoli semantickej kontrole
        sdata->state = STATE_FUNCTION_CALL;

        TData data;

        // Skontrolujeme ci funkcia je v globalnej tabulke prvkov
        if(!bst_search(sdata->global_table, previous_token.value, &data))
        {
            // Chyba: volanie nedeklarovanej funkcie
            print_error_message("<func_assig_func> Volanie nedeklarovanej funkcie pri pravidle 31", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
            return false;
        }

        // Zalohujeme si aktivnu funkciu
        TFunction *main_active = sdata->active_function;

        // Nastavi funkciu ako aktivnu pre lahsiu pracu pri citani typov parametrov, neskor sa musi nastavit naspat kvoli pripadnemu returnu definovanej funkcie
        sdata->active_function = data.function;

        // SYNTAKTICKA KONTROLA
        if(func_expression(sdata))
        {
            if(data.function->matters_on_params_count)
                generate_function_call(data.function->name, false);

            // Vratime aktivnost funkcie naspat povodnej funkcii aby sme vedeli typ navratovych hodnot
            sdata->active_function = main_active;

            // Prikaz nacita a skontroluje dalsi token
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

            // Token je RIGHT_BRACKET, pokracuj
            if(sdata->token->type == TOKEN_TYPE_RIGHT_BRACKET)
            {
                return true;
            }
            // Chyba: Token nie je RIGHT_BRACKET
            else
            {
                print_error_message("<func_assig_func> Token nie je RIGHT_BRACKET pri pravidle 31", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: <func_expression>
        else
        {
            return false;
        }
    }
    // Pravidlo 30: Token musi byt <id_n> alebo ERROR_SYNTAX_ANALYSIS
    else
    {
        // SEMANTICKA KONTROLA
        sdata->state = STATE_ASSIGNMENT_TO_ID;
        
        TData data;

        // Prehladame lokalne tabulky symbolov ci premenna bola deklarovana
        if(!is_var_in_symt_list(&(sdata->list_of_symtables), previous_token.value, &data))
        {
            // Chyba: Nedeklarovana premenna
            print_error_message("<func_assig_func> Volanie nedeklarovanej premennej pri pravidle 30", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
            sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
            return false;
        }

        // Nastavime premennu ako aktivnu pre lahsiu pracu s nou
        sdata->active_variable = data.variable;

        // Pridame previous premennu na koniec zoznamu premennych
        if(!DLLVarAddLast(&(sdata->list_of_variables), data.variable, previous_token.value))
        {
            print_error_message("<func_assig_func> Neuspesna alokacia pri pravidle 30", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
            sdata->return_code = ERROR_INTERN_TRANSLATOR;
            return false;
        }

        // SYNTAKTICKA KONTROLA
        return_token(sdata->token);

        // Pravidlo 30: Token musi byt <id_n>
        if(func_id_n(sdata))
        {
            // Prikaz nacita a skontroluje dalsi token
            if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

            // Token musi byt ASSIGNMENT
            if(sdata->token->type == TOKEN_TYPE_ASSIGNMENT)
            {
                DLLVarActiveFirst(&(sdata->list_of_variables));
                // Token musi byt <func_expression>
                if(func_expression(sdata))
                {
                    if(sdata->list_of_variables.active != NULL)
                    {
                        if(sdata->list_of_variables.active->nextItem != NULL)
                        {
                            print_error_message("Nie vsetkym identifikatorom boli priradene hodnoty", ERROR_SEMANTIC_INCOMPATIBLE_TYPES, __FILE__, __LINE__);
                            sdata->return_code = ERROR_SEMANTIC_INCOMPATIBLE_TYPES;
                            return false;    
                        }
                    }

                    // Nastavime aktivnost zoznamu na zaciatok
                    DLLVarActiveFirst(&(sdata->list_of_variables));

                    // Prechadzame zoznamom a nastavujeme premenne ako definovane
                    while(sdata->list_of_variables.active != NULL)
                    {
                        sdata->list_of_variables.active->data->defined = true;
                        DLLVarActiveNext(&(sdata->list_of_variables));
                    }

                    // Vraciame uspesnost pravidla 30
                    return true;
                }
                // Chyba: <func_expression>
                else
                {
                    return false;
                }
            }
            // Chyba: Token nie je ASSIGNMENT
            else
            {
                print_error_message("<func_assig_func> Token nie je ASSIGNMENT pri pravidle 30", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                return false;
            }
        }
        // Chyba: <func_id_n>
        else
        {
            return false;
        }
    }

    return false;
}

int analyse(){
    // Struktura pre syntakticku a semanticku kontrolu
    SPars_data *sdata = (SPars_data *)malloc(sizeof(SPars_data));

    // Skontrolujeme ci bola uspesna alokacia
    if(sdata == NULL)
    {
        print_error_message("<analyse> Neuspesna alokacia", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        return ERROR_INTERN_TRANSLATOR;
    }

    // Alokujeme token
    sdata->token = (TToken *)malloc(sizeof(TToken));

    // Skontrolujeme uspesnost alokacie
    if(sdata->token == NULL){
        // Chyba: Neuspesna alokacia pamate
        print_error_message("<analyse> Neuspesna alokacia", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        free(sdata);
        return ERROR_INTERN_TRANSLATOR;
    }

    // Inicializacia globalnej tabulky prvkov
    bst_init_table(&(sdata->global_table));

    // Overime uspesnost vlozenia vstavanych funkcii do globalnej tabulky prvkov
    if(!insert_build_functions(sdata))
    {
        print_error_message("Neuspesne vlozenie vstavanych funkcii do globalnej tabulky symbolov", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        free(sdata->token);
        free(sdata);
        return ERROR_INTERN_TRANSLATOR;
    }

    // Inicializacia zoznamu lokalnych tabuliek symbolov
    DLLSymtInit(&(sdata->list_of_symtables));

    // Nastavenie pociatocneho stavu
    sdata->state = STATE_NULL;

    // Inicializacia aktivnych prvkov struktury
    sdata->active_function = NULL;
    sdata->active_variable = NULL;

    // Return code bude primarne bez chyby
    sdata->return_code = NO_ERROR;

    // Inicializacia zoznamu typov struktury
    DLLVarInit(&(sdata->list_of_variables));

    // Generovanie kodu
    generator_start();

    // SPUSTENIE SYNTAKTICKEJ A SEMANTICKEJ ANALYZY
    func_prol(sdata);

    if(sdata->return_code == 0)
        print_to_output();

    // Ulozime si navratovu hodnotu pred uvolnenim pamate
    int return_code = sdata->return_code;

    // Korektne uvolnime alokovanu pamat
    bst_dispose(sdata->global_table, true);
    DLLVarDispose(&(sdata->list_of_variables));
    DLLSymtDispose(&(sdata->list_of_symtables));
    free(sdata->token);
    free(sdata);

    return return_code;
}

#endif
