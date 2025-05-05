/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021 - Lexikalna analyza
 *
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _SCANNER_C
#define _SCANNER_C

#include "scanner.h"
#include "error.h"
#include <stdlib.h>
#include <string.h>

#define INPUT_MAX_LENGTH 64
#define KEYWORD_MAX_LENGTH 8

// Stavy su koncove ale je z nich este prechod
#define STATE_START 700

#define STATE_KEYWORD_ID 701

#define STATE_INT 702
#define STATE_DOUBLE 703
#define STATE_DOUBLE_WITH_EXPONENT 704
#define STATE_ASSIGNMENT 705

#define STATE_OP_MINUS 706
#define STATE_OP_DIVIDE 707

#define STATE_RO_GREATER 708
#define STATE_RO_LESS 709

/**
 * Stavy ktore su koncove a neda sa z nich posunut dalej, sluzi k priradeniu v fsm
 * STATE_OP_PLUS 725
 * STATE_OP_MULTIPLY 726
 * STATE_OP_INT_DIVIDE 727
 * STATE_RO_GREATER_EQUAL 728
 * STATE_RO_LESS_EQUAL 729
 * STATE_RO_EQUAL 730
 * STATE_RO_NOT_EQUAL 731
 * STATE_HASHTAG 732
 * STATE_LEFT_BRACKET 733
 * STATE_RIGHT_BRACKET 734
 * STATE_COMMA 735
 * STATE_COLON 736
 * STATE_EOF 738
 * STATE_CONCATENATION 739
 * STATE_STRING 740
 */

// Pomocne stavy, nie su koncove
#define STATE_POTENTIAL_NOT_EQUAL 750
#define STATE_CONCATENATION_FIRST_DOT 751

#define STATE_DOUBLE_WITHOUT_DECIMAL_PART 752
#define STATE_DOUBLE_E_WITHOUT_EXPONENTIAL_PART 753
#define STATE_DOUBLE_E_WITH_OPERATOR_WITHOUT_EXP_PART 754

#define STATE_COMMENT_BEGIN 755
#define STATE_COMMENT_NORMAL_LOADING 756
#define STATE_COMMENT_POTENTIAL_BLOCK_BEGIN 757
#define STATE_COMMENT_BLOCK_COMMENT_LOADING 758
#define STATE_COMMENT_POTENTIAL_BLOCK_END 759

#define STATE_STRING_LOADING 760
#define STATE_STRING_BACKSLASH 761
#define STATE_STRING_BACKSLASH_NULL 762
#define STATE_STRING_BACKSLASH_DOUBLE_NULL 763
#define STATE_STRING_BACKSLASH_ONE 764
#define STATE_STRING_BACKSLASH_TWO 765
#define STATE_STRING_BACKSLASH_TWO_FIVE 766
#define STATE_STRING_BACKSLASH_LAST_DIGIT 767

FILE *file;

// Nastavenie suboru s ktorym bude scanner pracovat
void set_file(FILE *f){
    file = f;
}

/**
 * Funkcia urci ci string je keyword
 * ak ano, nastavi atribut tokenu
 */
bool is_keyword(char *str){
    if(strcmp(str, "do") == 0) return true;
    else if(strcmp(str, "global") == 0) return true;
    else if(strcmp(str, "number") == 0) return true;
    else if(strcmp(str, "else") == 0) return true;
    else if(strcmp(str, "if") == 0) return true;
    else if(strcmp(str, "require") == 0) return true;
    else if(strcmp(str, "end") == 0) return true;
    else if(strcmp(str, "integer") == 0) return true;
    else if(strcmp(str, "return") == 0) return true;
    else if(strcmp(str, "function") == 0) return true;
    else if(strcmp(str, "local") == 0) return true;
    else if(strcmp(str, "string") == 0) return true;
    else if(strcmp(str, "nil") == 0) return true;
    else if(strcmp(str, "then") == 0) return true;
    else if(strcmp(str, "while") == 0) return true;
    else return false;

    return true;
}

/**
 * Chyba v lexikalnej analyze, korektne uvolni alokovanu pamat
 * pre input a vrati chybovy kod
 */
int return_failure(int failure_code, char *input, TToken *token){
    free(input);
    token->type = TOKEN_TYPE_EMPTY;

    return failure_code;
}

/**
 * Funkcia nastavi token ktory bude funkcia vraciat pomocou ukazatela
 * a dalej uvolni alokovanu pamat
 */
void set_return_token(char *input, int input_index, TToken *token, Token_type type){
    // Nastavi na koniec inputu nulovy znak
    input[input_index] = '\0';

    // Nastavenie typu tokenu
    token->type = type;

    // Deklaracia pomocnych premennych
    int input_length = 0;
    int i = 0;

    // Alokujeme si iba potrebny priestor pre hodnotu tokenu
    while(input[i++] != '\0') input_length++;
    token->value = (char *)malloc(sizeof(char)*(++input_length));
    strcpy(token->value, input);

    // Uvolnenie alokovanej pamate
    free(input);
}

/**
 * Funkcia dostane ukazatel na token ktory nesmie byt NULL
 */
int get_next_token(TToken *token){
    // Chyba v pripade ze token je NULL
    if(token == NULL)
    {
        print_error_message("Chyba alokacie tokenu", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        return ERROR_INTERN_TRANSLATOR;
    }

    // Alokacia pamate pre input
    char *input = (char *) malloc(sizeof(char)*INPUT_MAX_LENGTH);

    // Kontrola uspesnosti alokacie
    if(input == NULL)
    {
        print_error_message("Chyba alokacie input retazca", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        return ERROR_INTERN_TRANSLATOR;
    }
    
    // Index na ktory ukladame znak v retazci input
    int input_index = 0;

    // Premenna stavu
    int state = STATE_START;

    // Premenna sluzi pre prevod sekvencie \ddd na znak pri retazcoch string
    int char_value = 0;

    do{
        // Nacitanie dalsieho znaku zo suboru
        char symbol = getc(file);

        switch(state){
            // Pociatocny stav
            case STATE_START:
                // Preskakovanie bielych znakov
                if(symbol == ' ' || symbol == '\t' || symbol == '\n') state = STATE_START;
                // Prechod do stavu keyword alebo identifikator
                else if((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol == '_'))
                {
                    state = STATE_KEYWORD_ID;
                    input[input_index++] = symbol;
                }
                // Prechod do stavu nacitavania cislicoveho literalu
                else if(symbol >= '0' && symbol <= '9')
                {
                    state = STATE_INT;
                    input[input_index++] = symbol;
                }
                // Prechod do stavu nacitavania stringu
                else if(symbol == '"') state = STATE_STRING_LOADING;
                // Token: operator plus
                else if(symbol == '+')
                {
                    token->length = 1;
                    token->type = TOKEN_TYPE_OPERATOR_PLUS;
                    free(input);
                    return NO_ERROR;
                }
                // Prechod do stavu minus alebo komentar
                else if(symbol == '-') state = STATE_OP_MINUS;
                // Token: operator krat
                else if(symbol == '*')
                {
                    token->length = 1;
                    token->type = TOKEN_TYPE_OPERATOR_MULTIPLY;
                    free(input);
                    return NO_ERROR;
                }
                // Prechod do stavu delenia alebo celociselneho delenia
                else if(symbol == '/') state = STATE_OP_DIVIDE;
                // Prechod do stavu relacny operator vacsi alebo vacsi rovny
                else if(symbol == '>') state = STATE_RO_GREATER;
                // Prechod do stavu relacny operator mensi alebo mensi rovny
                else if(symbol == '<') state = STATE_RO_LESS;
                // Prechod do stavu priradenia alebo relacneho operatoru rovna sa
                else if(symbol == '=') state = STATE_ASSIGNMENT;
                // Prechod do stavu potencionalny relacny operator nerovna sa
                else if(symbol == '~') state = STATE_POTENTIAL_NOT_EQUAL;
                // Prechod do stavu potencionalna konkatenacia
                else if(symbol == '.') state = STATE_CONCATENATION_FIRST_DOT;
                // Token: Hashtag
                else if(symbol == '#'){
                    token->length = 1;
                    token->type = TOKEN_TYPE_HASHTAG;
                    free(input);
                    return NO_ERROR;
                }
                // Token: LEFT_BRACKET
                else if(symbol == '('){
                    token->length = 1;
                    token->type = TOKEN_TYPE_LEFT_BRACKET;
                    free(input);
                    return NO_ERROR;
                }
                // Token: RIGHT_BRACKET
                else if(symbol == ')'){
                    token->length = 1;
                    token->type = TOKEN_TYPE_RIGHT_BRACKET;
                    free(input);
                    return NO_ERROR;
                }
                // Token: COMMA
                else if(symbol == ','){
                    token->length = 1;
                    token->type = TOKEN_TYPE_COMMA;
                    free(input);
                    return NO_ERROR;
                }
                // Token: COLON
                else if(symbol == ':'){
                    token->length = 1;
                    token->type = TOKEN_TYPE_COLON;
                    free(input);
                    return NO_ERROR;
                }
                // Token: EOF
                else if(symbol == -1){
                    token->length = 1;
                    token->type = TOKEN_TYPE_EOF;
                    free(input);
                    return NO_ERROR;
                }
                // Chyba: Neocakavany znak
                else 
                {
                    print_error_message("Neznamy znak na vstupe", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;

            // Stav delenia, urcujeme ci ide o normalne alebo celociselne delenie
            case STATE_OP_DIVIDE:
                // Skontrolujeme ci ide o celociselne delenie
                if(symbol == '/')
                {
                    // Ide o celociselne delenie
                    token->length = 2;
                    token->type = TOKEN_TYPE_OPERATOR_INT_DIVIDE;
                }
                // Nejde o celociselne delenie
                else
                {
                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    token->length = 1;
                    token->type = TOKEN_TYPE_OPERATOR_DIVIDE;
                }

                free(input);
                return NO_ERROR;
                break;
            // Stav minus, urcujeme ci ide o minus alebo komentar
            case STATE_OP_MINUS:
                // Skontrolujeme ci ide o komentar
                if(symbol == '-') state = STATE_COMMENT_BEGIN;
                else
                {
                    // Nejde o komentar
                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    token->length = 1;
                    token->type = TOKEN_TYPE_OPERATOR_MINUS;

                    free(input);
                    return NO_ERROR;
                }
                break;

            // Stav vacsi, urcujeme ci ide o relacny operator vacsi alebo vacsi rovny
            case STATE_RO_GREATER:
                // Skontrolujeme ci ide o relacny operator vacsi rovny
                if(symbol == '=')
                {
                    token->length = 2;
                    token->type = TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL;
                }
                else
                {
                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);
                    token->length = 1;
                    token->type = TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER;
                }

                free(input);
                return NO_ERROR;
                break;
            // Stav mensi, urcujeme ci ide o relacny operator mensi alebo mensi rovny
            case STATE_RO_LESS:
                // Skontrolujeme ci ide o relacny operator mensi rovny
                if(symbol == '=')
                {
                    token->length = 2;
                    token->type = TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL;
                }
                else
                {
                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    token->length = 1;
                    token->type = TOKEN_TYPE_RELATIONAL_OPERATOR_LESS;
                }

                free(input);
                return NO_ERROR;
                break;

            // Stav rovna sa, urcujeme ci ide o priradenie sa alebo relacny operator rovnosti
            case STATE_ASSIGNMENT:
                // Skontrolujeme ci sa jedna o relacny operator rovny
                if(symbol == '=')
                {
                    token->length = 2;
                    token->type = TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL;
                }
                else
                {
                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    token->length = 1;
                    token->type = TOKEN_TYPE_ASSIGNMENT;
                }

                free(input);
                return NO_ERROR;
                break;
            // Stav ~, urcujeme ci ide o chybu alebo relacny operator nerovnosti
            case STATE_POTENTIAL_NOT_EQUAL:
                if(symbol == '=')
                {
                    token->length = 2;
                    token->type = TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL;

                    free(input);
                    return NO_ERROR;
                }
                // Chyba: Neocakavany znak na vstupe
                else
                {
                    print_error_message("Chyba pri nacitani relacneho operatoru nie rovny", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
            // Stav pot, urcujeme ci ide o chybu alebo o relacny operator konkatenacie
            case STATE_CONCATENATION_FIRST_DOT:
                // Skontrolujeme ci sa jedna o operaciu konkatenacie
                if(symbol == '.')
                {
                    token->length = 2;
                    token->type = TOKEN_TYPE_CONCATENATION;

                    free(input);
                    return NO_ERROR;
                }
                // Chyba: Neocakavany znak na vstupe
                else
                {
                    print_error_message("Chyba pri nacitani konkatenacie", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;

            // Stav keyword alebo identifikator
            case STATE_KEYWORD_ID:
                if(!((symbol >= 'a' && symbol <= 'z') || (symbol >= 'A' && symbol <= 'Z') || (symbol >= '0' && symbol <= '9') || symbol == '_'))
                {
                    // Z daneho stavu uz nie je prechod pre symbol ktory prisiel
                    token->length = input_index;

                    if(is_keyword(input))
                        set_return_token(input, input_index, token, TOKEN_TYPE_KEYWORD);
                    else
                        set_return_token(input, input_index, token, TOKEN_TYPE_IDENTIFIER);

                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    return NO_ERROR;
                }
                // Nacitavame znaky dalej a pridame znak do input retazca
                else
                {
                    input[input_index++] = symbol;
                }
                break;

            // Stav nacitania cislicovych literalov
            case STATE_INT:
                // Prechod do stavu nacitavania decimalneho cisla
                if(symbol == '.') state = STATE_DOUBLE_WITHOUT_DECIMAL_PART;
                // Prechod do stavu nacitavania decimalneho cisla zadaneho pomocou exponentu
                else if(symbol == 'e' || symbol == 'E') state = STATE_DOUBLE_E_WITHOUT_EXPONENTIAL_PART;
                // Znak na vstupe nie je cislo
                else if(symbol < '0' || symbol > '9')
                {
                    token->length = input_index;

                    set_return_token(input, input_index, token, TOKEN_TYPE_INT);

                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    return NO_ERROR;
                }

                // Pokracovanie v nacitavani integeru
                input[input_index++] = symbol;
                break;
            case STATE_DOUBLE_WITHOUT_DECIMAL_PART:
                // Chyba: znak nie je cislo
                if(symbol < '0' || symbol > '9') return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                // Prechod do stavu nacitavania desatinneho cisla
                else state = STATE_DOUBLE;

                // Pridanie znaku na input
                input[input_index++] = symbol;
                break;
            case STATE_DOUBLE:
                // Nacitanie cisla s exponentom
                if(symbol == 'e' || symbol == 'E') state = STATE_DOUBLE_E_WITHOUT_EXPONENTIAL_PART;
                // Ak znak nie je cislo nepatri uz tokenu
                else if(symbol < '0' || symbol > '9'){
                    token->length = input_index;
                    set_return_token(input, input_index, token, TOKEN_TYPE_DOUBLE);

                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    return NO_ERROR;
                }

                input[input_index++] = symbol;
                break;
            case STATE_DOUBLE_E_WITHOUT_EXPONENTIAL_PART:
                // Decimalne cislo s exponentom a operatorom
                if(symbol == '+' || symbol == '-') state = STATE_DOUBLE_E_WITH_OPERATOR_WITHOUT_EXP_PART;
                // Ak je to cislo prejde do stavu decimalneho cisla s exponentom
                else if(symbol >= '0' && symbol <= '9') state = STATE_DOUBLE_WITH_EXPONENT;
                // Inak sa jedna o chybu
                else
                { 
                    print_error_message("Chyba pri nacitani decimalneho cisla konciaceho na e, E", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }

                input[input_index++] = symbol;
                break;
            case STATE_DOUBLE_WITH_EXPONENT:
                if(symbol < '0' || symbol > '9')
                {
                    token->length = input_index;
                    set_return_token(input, input_index, token, TOKEN_TYPE_DOUBLE);

                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);

                    return NO_ERROR;
                }

                input[input_index++] = symbol;
                break;
            case STATE_DOUBLE_E_WITH_OPERATOR_WITHOUT_EXP_PART:
                // Cislom prejdeme do nacitania decimalneho cisla s exponentom
                if(symbol >= '0' && symbol <= '9') state = STATE_DOUBLE_WITH_EXPONENT;
                // Inak sa jedna o chybu
                else 
                {
                    print_error_message("Chyba pri nacitani decimalneho cisla ukonceneho operatorom plus alebo minus", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }

                input[input_index++] = symbol;
                break;

            // Stav nacitavania string retazca
            case STATE_STRING_LOADING:
                // Znak pre ukoncenie retazca
                if(symbol == '\"')
                {
                    token->length = input_index;
                    set_return_token(input, input_index, token, TOKEN_TYPE_STRING);

                    return NO_ERROR;
                }

                // Prechod do stavu backslashu
                else if(symbol == '\\') state = STATE_STRING_BACKSLASH;
                // Znak je mimo povolenych rozmedzi
                else if(symbol < 32 || symbol > 255)
                {
                    print_error_message("Znak mimo povoleneho rozsahu v nacitani stringu", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                // Inak pridame znak do inputu
                else input[input_index++] = symbol;

                break;

            // Specialne stavy nacitavania string retazca
            case STATE_STRING_BACKSLASH:
                // Specialne znaky
                if(symbol == 'n' || symbol == 't' || symbol == '\"' || symbol == '\\'){
                    state = STATE_STRING_LOADING;
                    if(symbol == 'n') input[input_index++] = '\n';
                    else if(symbol == 't') input[input_index++] = '\t';
                    else if(symbol == '\"') input[input_index++] = '\"';
                    else input[input_index++] = '\\';
                }
                // Prechod do stavu nacitania /ddd
                else if(symbol == '0'){
                    state = STATE_STRING_BACKSLASH_NULL;
                    char_value = 0;
                }
                // Prechod do stavu nacitania /ddd
                else if(symbol == '1'){
                    state = STATE_STRING_BACKSLASH_ONE;
                    char_value = 100;
                }
                // Prechod do stavu nacitania /ddd
                else if(symbol == '2'){
                    state = STATE_STRING_BACKSLASH_TWO;
                    char_value = 200;
                }
                // Iny znak detekuje chybu
                else
                {
                    print_error_message("Neznamy znak na vstupe pri nacitany string backslash", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__); 
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }

                break;
            case STATE_STRING_BACKSLASH_NULL:
                if(symbol == '0') state = STATE_STRING_BACKSLASH_DOUBLE_NULL;
                else if(symbol >= '1' && symbol <= '9')
                {
                    state = STATE_STRING_BACKSLASH_LAST_DIGIT;
                    char_value = (symbol - '1' + 1)*10;
                }
                else 
                {
                    print_error_message("Znak na vstupe nie je cislica v backslash null", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }

                break;
            case STATE_STRING_BACKSLASH_DOUBLE_NULL:
                if(symbol >= '1' && symbol <= '9') state = STATE_STRING_LOADING;
                else
                {
                    print_error_message("Znak na vstupe nie je cislica v rozmedzi 1-9 v stave backslash double null", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__); 
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }

                // Prevod na symbolu na int hodnotu
                char_value = (symbol - '1' + 1);

                input[input_index++] = char_value;

                break;
            case STATE_STRING_BACKSLASH_ONE:
                if(symbol >= '0' && symbol <= '9') state = STATE_STRING_BACKSLASH_LAST_DIGIT;
                else
                {
                    print_error_message("Znak na vstupe nie je cislica v stave backslash one", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__); 
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                char_value = char_value + (symbol - '1' + 1)*10;
                break;
            case STATE_STRING_BACKSLASH_TWO:
                if(symbol == '5') state = STATE_STRING_BACKSLASH_TWO_FIVE;
                else if(symbol >= '0' && symbol <= '4') state = STATE_STRING_BACKSLASH_LAST_DIGIT;
                else
                { 
                    print_error_message("Znak na vstupe nie je cislica v rozmedzi 0-5 v stave backslash two", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }

                // Prevod na symbolu na int hodnotu
                char_value = char_value + (symbol - '1' + 1)*10;

                break;
            case STATE_STRING_BACKSLASH_TWO_FIVE:
                if(symbol >= '0' && symbol <= '5'){
                    state = STATE_STRING_LOADING;

                    // Prevod na symbolu na int hodnotu
                    char_value = char_value + (symbol - '1' + 1);

                    input[input_index++] = char_value;
                }
                else
                {   
                    print_error_message("Znak na vstupe nie je cislica v rozmedzi 0-5 v stave backslash two five", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
            case STATE_STRING_BACKSLASH_LAST_DIGIT:
                if(symbol >= '0' && symbol <= '9'){
                    state = STATE_STRING_LOADING;

                    // Prevod na symbolu na int hodnotu
                    char_value = char_value + (symbol - '1' + 1);

                    input[input_index++] = char_value;
                }
                else 
                {   
                    print_error_message("Znak na vstupe nie je cislica v stave backslash last digit", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;

            // Stavy komentarov
            case STATE_COMMENT_BEGIN:
                if(symbol == '\n') state = STATE_START;
                else if(symbol == '[') state = STATE_COMMENT_POTENTIAL_BLOCK_BEGIN;
                else if(symbol >= 32 && symbol <= 255) state = STATE_COMMENT_NORMAL_LOADING;
                else 
                {
                    print_error_message("Neznamy znak na vstupe pri nacitani komentaru", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
            case STATE_COMMENT_NORMAL_LOADING:
                // Ukoncenie nacitania komentaru
                if(symbol == '\n') state = STATE_START;
                // Neocakavany EOF
                else if(symbol == -1){
                    state = STATE_START;

                    // Vratime znak znovu naspat aby ho mohol dalsi token nacitat
                    ungetc(symbol, file);
                }
                // Chyba: Neocakavany znak na vstupe
                else if(symbol < 32 || symbol > 255)
                {
                    print_error_message("Neocakavany znak na vstupe v nacitani normalneho komentaru", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
            case STATE_COMMENT_POTENTIAL_BLOCK_BEGIN:
                if(symbol == '[') state = STATE_COMMENT_BLOCK_COMMENT_LOADING;
                else if(symbol >= 32 && symbol <= 255) state = STATE_COMMENT_NORMAL_LOADING;
                else
                {
                    print_error_message("Neocakavany znak pri nacitani potencionalneho blokoveho komentaru", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
            case STATE_COMMENT_BLOCK_COMMENT_LOADING:
                if(symbol == ']') state = STATE_COMMENT_POTENTIAL_BLOCK_END;
                else if((symbol < 32 || symbol > 255) && symbol != '\n')
                {
                    print_error_message("Neocakavany znak na vstupe pri nacitani blokoveho komentaru", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
            case STATE_COMMENT_POTENTIAL_BLOCK_END:
                if(symbol == ']')  state = STATE_START;
                else if((symbol >= 32 && symbol <= 255) || symbol == '\n') state = STATE_COMMENT_BLOCK_COMMENT_LOADING;
                else
                {
                    print_error_message("Neocakavany znak na vstupe pri nacitani potencionalneho konca blokoveho komentaru", ERROR_LEXICAL_ANALYSIS, __FILE__, __LINE__);
                    return return_failure(ERROR_LEXICAL_ANALYSIS, input, token);
                }
                break;
        }
    }while(true);
}

/**
 * Funkcia vrati scanneru posledny token pomocou typu tokenu pripadne jeho atributu
 */
void return_token(TToken *token){
    // Funkcia vrati hodnotu aby ho mohol lexikalny analyzator znovu nacitat
    switch(token->type){
        case TOKEN_TYPE_EMPTY:
            break;
        case TOKEN_TYPE_KEYWORD:
        case TOKEN_TYPE_IDENTIFIER:
        case TOKEN_TYPE_INT:
        case TOKEN_TYPE_DOUBLE:
            for(int i = token->length; i > 0; i--) ungetc(token->value[i-1], file);
            break;
        case TOKEN_TYPE_STRING:
            ungetc('\"', file);
            for(int i = token->length; i > 0; i--) ungetc(token->value[i-1], file);
            ungetc('\"', file);
            break;

        case TOKEN_TYPE_OPERATOR_PLUS:
            ungetc('+', file);
            break;
        case TOKEN_TYPE_OPERATOR_MINUS:
            ungetc('-', file);
            break;
        case TOKEN_TYPE_OPERATOR_MULTIPLY:
            ungetc('*', file);
            break;
        case TOKEN_TYPE_OPERATOR_DIVIDE:
            ungetc('/', file);
            break;
        case TOKEN_TYPE_OPERATOR_INT_DIVIDE:
            ungetc('/', file);
            ungetc('/', file);
            break;

        case TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER:
            ungetc('>', file);
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_LESS:
            ungetc('<', file);
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL:
            ungetc('=', file);
            ungetc('>', file);
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL:
            ungetc('=', file);
            ungetc('<', file);
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL:
            ungetc('=', file);
            ungetc('=', file);
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL:
            ungetc('=', file);
            ungetc('~', file);
            break;

        case TOKEN_TYPE_CONCATENATION:
            ungetc('.', file);
            ungetc('.', file);
            break;
        case TOKEN_TYPE_ASSIGNMENT:
            ungetc('=', file);
            break;
        case TOKEN_TYPE_LEFT_BRACKET:
            ungetc('(', file);
            break;
        case TOKEN_TYPE_RIGHT_BRACKET:
            ungetc(')', file);
            break;
        case TOKEN_TYPE_COMMA:
            ungetc(',', file);
            break;
        case TOKEN_TYPE_COLON:
            ungetc(':', file);
            break;
        case TOKEN_TYPE_HASHTAG:
            ungetc('#', file);
            break;

        case TOKEN_TYPE_EOF:
            ungetc(-1, file);
            break;
    }
}

#endif
