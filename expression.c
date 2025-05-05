/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021 - Spracovanie vyrazov
 * 
 * Autori ktori na tomto subore pracovali:
 * Jakub Kasem (xkasem02)   - Syntakticke kontroly
 * Adam Dzurilla (xdzuri00) - Semanticke kontroly
 *
 */

#ifndef _EXPRESSION_C
#define _EXPRESSION_C

#include <stdlib.h>
#include "expression.h"
#include "code_generator.h"
#include "./tests/test_utils.h"

#define PRECEDENT_TABLE_SIZE 17

/**
* Precedencna tabulka
* shift '>'
* reduce '>'
* ukoncenie zatvorky '='
* chyba '0' 
*/
char precedent_table[PRECEDENT_TABLE_SIZE][PRECEDENT_TABLE_SIZE] =
{
//    0    1    2    3    4     5   6    7    8     9   10   11   12   13   14   15   16
//    +    -    *    /    //   ..   (    )    <    <=   >    >=   ==   ~=   i    $    #     VSTUP/STACK
    {'>', '>', '<', '<', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<'}, // +      0
    {'>', '>', '<', '<', '<', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<'}, // -      1
    {'>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<'}, // *     2
    {'>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<'}, // /     3
    {'>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<'}, // //    4
    {'<', '<', '<', '<', '<', '<', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '<'}, // ..    5
    {'<', '<', '<', '<', '<', '<', '<', '=', '<', '<', '<', '<', '<', '<', '<', '0', '<'}, // (     6
    {'>', '>', '>', '>', '>', '>', '0', '>', '>', '>', '>', '>', '>', '>', '0', '>', '0'}, // )     7
    {'<', '<', '<', '<', '<', '<', '<', '>', '0', '0', '0', '0', '0', '0', '<', '>', '<'}, // <     8
    {'<', '<', '<', '<', '<', '<', '<', '>', '0', '0', '0', '0', '0', '0', '<', '>', '<'}, // <=    9
    {'<', '<', '<', '<', '<', '<', '<', '>', '0', '0', '0', '0', '0', '0', '<', '>', '<'}, // >     10
    {'<', '<', '<', '<', '<', '<', '<', '>', '0', '0', '0', '0', '0', '0', '<', '>', '<'}, // >=    11
    {'<', '<', '<', '<', '<', '<', '<', '>', '0', '0', '0', '0', '0', '0', '<', '>', '<'}, // ==    12
    {'<', '<', '<', '<', '<', '<', '<', '>', '0', '0', '0', '0', '0', '0', '<', '>', '<'}, // ~=    13
    {'>', '>', '>', '>', '>', '>', '2', '>', '>', '>', '>', '>', '>', '>', '0', '>', '0'}, // i     14
    {'<', '<', '<', '<', '<', '<', '<', '0', '<', '<', '<', '<', '<', '<', '<', '1', '<'}, // $     15
    {'>', '>', '>', '>', '>', '>', '<', '>', '>', '>', '>', '>', '>', '>', '<', '>', '0'}, // #     16
};

TToken previous_token_expression;

// Pomocne funkcie

/**
 * Funkcia skontroluje ci typ tokenu vyhovuje typu s ktorym sa pracuje, v pripade identifikatoru sa vrati hodnota true a jeho validita bude skontrolovana inde,
 * id je kontrolovane inde z dovodu ze zatial nevieme posudit ci ide o id premennej alebo funkcie
 */
bool is_token_okay(SPars_data *sdata, Data_type *current_working_type, int line)
{
    Token_type ctype = sdata->token->type;

    if(ctype == TOKEN_TYPE_IDENTIFIER)
    {
        TToken previous_token = *(sdata->token);
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        bool is_func_id = sdata->token->type == TOKEN_TYPE_LEFT_BRACKET;

        return_token(sdata->token);
        sdata->token->type = previous_token.type;
        sdata->token->value = previous_token.value;
        sdata->token->length = previous_token.length;

        TData data;

        // Overime ci funkcia bola uz deklarovana
        if(is_func_id)
        {
            if(!bst_search(sdata->global_table, sdata->token->value, &data))
            {
                // Nedeklarovana funkcia
                print_error_message("Nedeklarovany id funkcie", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                return false;
            }
        }
        else
        {
            if(!is_var_in_symt_list(&(sdata->list_of_symtables), sdata->token->value, &data))
            {
                // Nedeklarovana premenna
                print_error_message("Nedeklarovany identifikator", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                return false;
            }

            if(*current_working_type == TYPE_UNDEFINED)
            {
                *current_working_type = data.variable->type;
            }
            else if(data.variable->type != *current_working_type)
            {
                // Skontrolujeme ci sa da urobit konverzia
                if(!(*current_working_type == TYPE_NUMBER && data.variable->type == TYPE_INT))
                {
                    print_error_message("Nekompatibilita typov", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, __LINE__);
                    sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                    return false;
                }
            }
        }

        return true;
    }

    switch(*current_working_type)
    {
        case TYPE_UNDEFINED:
            if(ctype == TOKEN_TYPE_INT) *current_working_type = TYPE_INT;
            else if(ctype == TOKEN_TYPE_DOUBLE) *current_working_type = TYPE_NUMBER;
            else if(ctype == TOKEN_TYPE_STRING) *current_working_type = TYPE_STRING;
            else if(ctype == TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "nil")) *current_working_type = TYPE_NIL;
            else
            {
                print_error_message("Typ spracovaneho tokenu nie je kompatibilny", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                return false;
            }
            break;
        case TYPE_INT:
            if(ctype == TOKEN_TYPE_OPERATOR_DIVIDE || ctype == TOKEN_TYPE_OPERATOR_INT_DIVIDE)
            {
                TToken prev_tok = *(sdata->token);
                if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                if(sdata->token->type == TOKEN_TYPE_INT && atoi(sdata->token->value) == 0)
                {
                    // Chyba delenia nulou
                    print_error_message("Pokus o delenie nulou", ERROR_DIVIDING_BY_ZERO, __FILE__, __LINE__);
                    sdata->return_code = ERROR_DIVIDING_BY_ZERO;
                    return false;
                }
                else
                {
                    return_token(sdata->token);
                    sdata->token->type = prev_tok.type;
                    sdata->token->value = prev_tok.value;
                    sdata->token->length = prev_tok.length;
                }
            }

            if(ctype == TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "nil"))
            {
                print_error_message("Neocakavana hodnota nil\n", ERROR_NOT_EXPECTED_NIL, __FILE__, __LINE__);
                sdata->return_code = ERROR_NOT_EXPECTED_NIL;
                return false;
            }

            if(!(ctype == TOKEN_TYPE_INT || 
                ctype == TOKEN_TYPE_OPERATOR_PLUS ||
                ctype == TOKEN_TYPE_OPERATOR_MINUS ||
                ctype == TOKEN_TYPE_OPERATOR_MULTIPLY ||
                ctype == TOKEN_TYPE_OPERATOR_DIVIDE ||
                ctype == TOKEN_TYPE_OPERATOR_INT_DIVIDE ||
                ctype == TOKEN_TYPE_LEFT_BRACKET ||
                ctype == TOKEN_TYPE_RIGHT_BRACKET
                ))
            {
                if(sdata->state == STATE_IF || sdata->state == STATE_WHILE)
                {
                    if(ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_LESS &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL
                        )
                    {
                        print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                        sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                        return false;
                    }
                }
                else
                {
                    print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                    sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                    return false;
                }
            }
            break;
        case TYPE_NUMBER:
            if(ctype == TOKEN_TYPE_OPERATOR_DIVIDE)
            {
                TToken prev_tok = *(sdata->token);
                if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

                if(sdata->token->type == TOKEN_TYPE_INT && atoi(sdata->token->value) == 0)
                {
                    // Chyba delenia nulou
                    print_error_message("Pokus o delenie nulou", ERROR_DIVIDING_BY_ZERO, __FILE__, __LINE__);
                    sdata->return_code = ERROR_DIVIDING_BY_ZERO;
                    return false;
                }
                else
                {
                    return_token(sdata->token);
                    sdata->token->type = prev_tok.type;
                    sdata->token->value = prev_tok.value;
                    sdata->token->length = prev_tok.length;
                }
            }

            if(ctype == TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "nil"))
            {
                print_error_message("Neocakavana hodnota nil\n", ERROR_NOT_EXPECTED_NIL, __FILE__, __LINE__);
                sdata->return_code = ERROR_NOT_EXPECTED_NIL;
                return false;
            }

            if(!(ctype == TOKEN_TYPE_INT ||
                ctype == TOKEN_TYPE_DOUBLE ||
                ctype == TOKEN_TYPE_OPERATOR_PLUS ||
                ctype == TOKEN_TYPE_OPERATOR_MINUS ||
                ctype == TOKEN_TYPE_OPERATOR_MULTIPLY ||
                ctype == TOKEN_TYPE_OPERATOR_DIVIDE ||
                ctype == TOKEN_TYPE_LEFT_BRACKET ||
                ctype == TOKEN_TYPE_RIGHT_BRACKET ||
                ctype == TOKEN_TYPE_HASHTAG ||
                ctype == TOKEN_TYPE_STRING
                ))
            {
                if(sdata->state == STATE_IF || sdata->state == STATE_WHILE)
                {
                    if(ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_LESS &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL
                        )
                    {
                        print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                        sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                        return false;
                    }
                }
                else
                {
                    print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                    sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                    return false;
                }
            }
            else
            {
                if(ctype == TOKEN_TYPE_INT)
                {
                    sdata->token->type = TOKEN_TYPE_DOUBLE;
                }
            }
            break;
        case TYPE_STRING:
            if(ctype == TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "nil"))
            {
                print_error_message("Neocakavana hodnota nil\n", ERROR_NOT_EXPECTED_NIL, __FILE__, __LINE__);
                sdata->return_code = ERROR_NOT_EXPECTED_NIL;
                return false;
            }

            if(!(ctype == TOKEN_TYPE_STRING ||
                ctype == TOKEN_TYPE_CONCATENATION ||
                ctype == TOKEN_TYPE_LEFT_BRACKET ||
                ctype == TOKEN_TYPE_RIGHT_BRACKET
                ))
            {
                if(sdata->state == STATE_IF || sdata->state == STATE_WHILE)
                {
                    if(ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL &&
                        ctype != TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL
                        )
                    {
                        print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                        sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                        return false;
                    }
                }
                else
                {
                    print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                    sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                    return false;
                }
            }
            break;
        case TYPE_NIL:
            if(ctype != TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "nil"))
            {
                print_error_message("Nespravny typ operandu", ERROR_SEMANTIC_TYPE_COMPABILITY, __FILE__, line);
                sdata->return_code = ERROR_SEMANTIC_TYPE_COMPABILITY;
                return false;
            }
            break;
        default:
            break;
    }

    return true;
}

// Definicie funkcii

/**
 * Funkcia nacita a skontroluje dalsi token
 * Ak je v poriadku, do b priradi aktualny znak na vstupe
 */
bool next_expression(SPars_data *sdata, Table_index *tab_index, Data_type *current_working_type)
{
    previous_token_expression = *sdata->token;
    // Nacitaj dalsi token a skontroluj ho
    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

    // Do b priradi aktualny znak na vstupe
    switch(sdata->token->type)
    {
        // i
        case TOKEN_TYPE_IDENTIFIER:
        case TOKEN_TYPE_INT:
        case TOKEN_TYPE_DOUBLE:
        case TOKEN_TYPE_STRING:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_I;
            break;
        //ostatne symboly z tabulky
        case TOKEN_TYPE_OPERATOR_PLUS:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_PLUS;
            break;
        case TOKEN_TYPE_OPERATOR_MINUS:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_MINUS;
            break;
        case TOKEN_TYPE_OPERATOR_MULTIPLY:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_MUL;
            break;
        case TOKEN_TYPE_OPERATOR_DIVIDE:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_DIV;
            break;
        case TOKEN_TYPE_OPERATOR_INT_DIVIDE:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_DIV_INT;
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_GREATER;
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_LESS:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_LESS;
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_GREATER_EQUAL:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_GREATER_EQ;
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_LESS_EQUAL:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_LESS_EQ;
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_EQUAL:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_EQ;
            break;
        case TOKEN_TYPE_RELATIONAL_OPERATOR_NOT_EQUAL:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_NEQ;
            break;
        case TOKEN_TYPE_CONCATENATION:
            if(!is_token_okay(sdata, current_working_type, __LINE__)) return false;
            *tab_index = EXPR_CONC;
            break;
        case TOKEN_TYPE_LEFT_BRACKET:
            *tab_index = EXPR_LEFT_BRACKET;
            break;
        case TOKEN_TYPE_RIGHT_BRACKET:
            *tab_index = EXPR_RIGHT_BRACKET;
            break;
        case TOKEN_TYPE_HASHTAG:
            *tab_index = EXPR_LENGTH;
            break;
        //$
        default:
            *tab_index = EXPR_DOLLAR;
            break;
    }
    if(sdata->token->type == TOKEN_TYPE_KEYWORD && !strcmp(sdata->token->value, "nil"))
    {
        *tab_index = EXPR_I;
    }

    return true;
}

/**
 * Funkcia skontroluje, ci ide o validne pravidlo
 * O ake pravidlo sa jedna vrati cez *rule
 */
bool check_expr_rule(Stack *stack, int shift_index, Prec_rule *rule)
{
    Table_index *expr = (Table_index *)malloc((stack->topIndex)*(sizeof(Table_index)));
    if (expr == NULL)
    { 
        print_error_message("Chyba pri alokovani pamati v syntaktickej analyze", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        return false;
    }

    int expr_symbol_count = 0;
    //do *expr nacitaj y
    for (int i = 0; i < (stack->topIndex - shift_index); i++)
    {
        expr[i] = stack->array[stack->topIndex - i];
        expr_symbol_count++;
    }

    switch(expr[0])
    {
        //E -> i
        case EXPR_I:
            if (expr_symbol_count == 1)
            {
                *rule = RULE_OPERAND;
            }
            else
                *rule = RULE_ERROR;
            break;
        // E -> (E)
        case EXPR_RIGHT_BRACKET:
            if ((expr_symbol_count == 3) && (expr[2] == EXPR_LEFT_BRACKET) && (expr[1] == STACK_EXPRESSION))
                *rule = RULE_BRACKETS;
            else
                *rule = RULE_ERROR;
            break;
        // Ostatne pravidla
        case STACK_EXPRESSION:
            if ((expr_symbol_count == 2) && (expr[1] == EXPR_LENGTH))
                *rule = RULE_LENGTH;
            else if (expr_symbol_count == 3)
            {
                if (expr[2] == STACK_EXPRESSION)
                {
                    switch(expr[1])
                    {
                        case EXPR_PLUS:
                            *rule = RULE_PLUS;
                            break;
                        case EXPR_MINUS:
                            *rule = RULE_MINUS;
                            break;
                        case EXPR_MUL:
                            *rule = RULE_MUL;
                            break;
                        case EXPR_DIV:
                            *rule = RULE_DIV;
                            break;
                        case EXPR_DIV_INT:
                            *rule = RULE_DIV_INT;
                            break;
                        case EXPR_CONC:
                            *rule = RULE_CONC;
                            break;
                        case EXPR_LESS:
                            *rule = RULE_LESS;
                            break;
                        case EXPR_LESS_EQ:
                            *rule = RULE_LESS_EQ;
                            break;
                        case EXPR_GREATER:
                            *rule = RULE_GREATER;
                            break;
                        case EXPR_GREATER_EQ:
                            *rule = RULE_GREATER_EQ;
                            break;
                        case EXPR_EQ:
                            *rule = RULE_EQ;
                            break;
                        case EXPR_NEQ:
                            *rule = RULE_NEQ;
                            break;
                        default:
                            *rule = RULE_ERROR;
                            break;
                    }
                }
                else
                    *rule = RULE_ERROR;
            }
            else
                *rule = RULE_ERROR;
            break;
        // Chyba
        default:
            *rule = RULE_ERROR;
            break;
    }

    free(expr);
    if (*rule != RULE_ERROR)
        return true;
    else
        return false;
}

/**
 * Funkcia dostane znak a vrati poziciu na ktorej sa znak nachadza v tabulke
 */
int prec_table_index(Table_index i)
{
    switch(i)
    {
            case EXPR_PLUS:
                return 0;
                break;          
            case EXPR_MINUS:
                return 1;
                break;
            case EXPR_MUL:
                return 2;
                break;
            case EXPR_DIV:
                return 3;
                break;
            case EXPR_DIV_INT:
                return 4;
                break;
            case EXPR_CONC:
                return 5;
                break;
            case EXPR_LEFT_BRACKET:
                return 6;
                break;
            case EXPR_RIGHT_BRACKET:
                return 7;
                break;
            case EXPR_LESS:
                return 8;
                break;
            case EXPR_LESS_EQ:
                return 9;
                break;
            case EXPR_GREATER:
                return 10;
                break;
            case EXPR_GREATER_EQ:
                return 11;
                break;
            case EXPR_EQ:
                return 12;
                break;
            case EXPR_NEQ:
                return 13;
                break;
            case EXPR_I:
                return 14;
                break;
            case EXPR_DOLLAR:
            case STACK_END:
                return 15;
                break;
            case EXPR_LENGTH:
                return 16;
                break;
            default:
                break;
    }

    return -1;
}

Data_type set_dt_by_token(SPars_data *sdata)
{
    switch(sdata->token->type)
    {
        case TOKEN_TYPE_KEYWORD:
            if(!strcmp(sdata->token->value, "nil")) return TYPE_NIL;
            break;
        case TOKEN_TYPE_IDENTIFIER:
            return TYPE_UNDEFINED;
            break;
        case TOKEN_TYPE_INT:
            return TYPE_INT;
            break;
        case TOKEN_TYPE_DOUBLE:
            return TYPE_NUMBER;
            break;
        case TOKEN_TYPE_STRING:
            return TYPE_STRING;
            break;
        default:
            break;
    }

    return TYPE_UNDEFINED;
}

bool func_expression(SPars_data *sdata)
{
    //Inicializuj stack
    Stack *stack = (Stack *)malloc(sizeof(Stack));
    if(!stack_init(stack))
    {
        print_error_message("Chyba v alokacii stacku vo funkcii <func_expression>", ERROR_INTERN_TRANSLATOR, __FILE__, __LINE__);
        sdata->return_code = ERROR_INTERN_TRANSLATOR;
        return false;
    }

    // Vlozime na zasobnik $
    stack_push(stack, STACK_END);

    // a
    Table_index a;

    // b
    Table_index b;

    // SEMANTICKA KONTROLA
    Data_type current_working_type = set_dt_by_token(sdata);

    // Do b priradi aktualny token na vstupe a skontroluje ho
    if(!next_expression(sdata, &b, &current_working_type))
    {
        free(stack);
        return false;
    }

    // GENEROVANIE KODU
    Stack cod_gen_stack;
    stack_init(&cod_gen_stack);

    // Pomocne premenne
    int shift_index;

    Prec_rule rule;

    bool finish = false;
    while(!(finish))
    {
        // Do a prirad vrchol zasobnika
        stack_top_a(stack, &a);

        int pos_a = prec_table_index(a);
        int pos_b = prec_table_index(b);

        switch(precedent_table[pos_a][pos_b])
        {
            case '=':
                generate_push(*(sdata->token), &cod_gen_stack);
                stack_push(stack, b);
                if(!next_expression(sdata, &b, &current_working_type))
                {
                    free(stack);
                    return false;
                }
                break;
            case '<':
                generate_push(*(sdata->token), &cod_gen_stack);
                stack_change_top_a(stack);
                stack_push(stack, b);
                if(!next_expression(sdata, &b, &current_working_type))
                {
                    free(stack);
                    return false;
                }
                break;
            case '>':
                if (stack_top_y(stack, &shift_index))
                {
                    if (check_expr_rule(stack, shift_index, &rule))
                    {
                        stack_change_top_b(stack, rule, shift_index);
                    }
                    else
                    {
                        print_error_message("Neexistuje pravidlo pre prechod precedencnu gramatiku", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                        return false;
                    }
                }
                else
                {
                    print_error_message("Na zasobniku je nespravny vyraz", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                    sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                    return false;
                }
                break;
            case '0':
            case '1':
                //print_data_type(current_working_type);
                push_op_until_left(&cod_gen_stack);
                if(sdata->state == STATE_ASSIGNMENT_TO_ID || sdata->state == STATE_ID_INIT)
                {
                    if(sdata->active_variable->type != current_working_type)
                    {
                        print_error_message("Nekompatibilita typov v priradeni", ERROR_SEMANTIC_ASSIGNMENT, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SEMANTIC_ASSIGNMENT;
                        return false;
                    }
                    generate_pop(sdata->active_variable->name);
                }
                else if(sdata->state == STATE_FUNCTION_CALL)
                {
                    if(!strcmp(sdata->active_function->name, "write"))
                    {
                        build_function_write();
                    }
                    else
                    {
                        if(DLLTypesGetActiveType(sdata->active_function->params) != TYPE_UNDEFINED)
                        {
                            if(current_working_type != DLLTypesGetActiveType(sdata->active_function->params))
                            {
                                print_error_message("Nespravny typ parametrov pri volani funkcie", ERROR_SEMANTIC_INCOMPATIBLE_TYPES, __FILE__, __LINE__);
                                sdata->return_code = ERROR_SEMANTIC_INCOMPATIBLE_TYPES;
                                return false;
                            }
                        }
                    }
                }
                else if (sdata->state == STATE_IF)
                {
                    char if_pop_result_index[] = "$if_result";
                    char c[5];
                    sprintf(c, "%d", get_if_index());
                    strcat(if_pop_result_index, c);
                    generate_pop(if_pop_result_index);
                }
                return_token(sdata->token);
                finish = true;
                break;
            // ID ( <expression> ) - ID je funkcia
            case '2':
                // Rekurzivne volanie expressions ak jednym z argumentov funkcie je znovu volanie funkcie
                if(previous_token_expression.type == TOKEN_TYPE_IDENTIFIER)
                {
                    // SEMANTICKE KONTROLY
                    if(sdata->state == STATE_IF || sdata->state == STATE_WHILE || sdata->state == STATE_FUNCTION_DECLARATION)
                    {
                        print_error_message("Nespravny stav pri volani vyrazu", ERROR_SEMANTIC_OTHERS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SEMANTIC_OTHERS;
                        return false;
                    }

                    TData data;
                    if(!bst_search(sdata->global_table, previous_token_expression.value, &data))
                    {
                        print_error_message("Volanie nedeklarovanej funkcie v <func_expression>", ERROR_SEMANTIC_DEFINITIONS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SEMANTIC_DEFINITIONS;
                        return false;
                    }

                    TFunction *previous_active_func = sdata->active_function;

                    sdata->active_function = data.function;

                    Parser_State previous_state = sdata->state;

                    DLLTypesActiveFirst(sdata->active_function->return_types);
                    while(sdata->active_function->return_types->active != NULL)
                    {
                        Data_type id_t;
                        if(sdata->state == STATE_ASSIGNMENT_TO_ID)
                        {
                            TVariable *act_var = DLLVarGetActive(&(sdata->list_of_variables));
                            id_t = act_var->type;
                        }
                        else if(sdata->state == STATE_RETURN)
                        {
                            id_t = DLLTypesGetActiveType(previous_active_func->return_types);
                        }
                        else if(sdata->state == STATE_ID_INIT)
                        {
                            id_t = sdata->active_variable->type;
                        }

                        Data_type ret_t = DLLTypesGetActiveType(sdata->active_function->return_types);

                        if(id_t != ret_t)
                        {
                            print_error_message("Nekompatibilita typov v <func_expression>", ERROR_SEMANTIC_ASSIGNMENT, __FILE__, __LINE__);
                            sdata->return_code = ERROR_SEMANTIC_ASSIGNMENT;
                            return false;
                        }

                        if(sdata->state == STATE_ASSIGNMENT_TO_ID)
                            DLLVarActiveNext(&(sdata->list_of_variables));
                        else
                            DLLTypesActiveNext(previous_active_func->return_types);

                        DLLTypesActiveNext(sdata->active_function->return_types);
                    }

                    sdata->state = STATE_FUNCTION_CALL;

                    if(!strcmp(sdata->active_function->name, "readi"))
                    {
                        if(sdata->active_variable->type == TYPE_INT || sdata->active_variable->type == TYPE_NUMBER)
                            build_function_readi(sdata->active_variable->name);
                        else
                        {
                            print_error_message("Nekompatibilita v priradeni", ERROR_SEMANTIC_ASSIGNMENT, __FILE__, __LINE__);
                            sdata->return_code = ERROR_SEMANTIC_ASSIGNMENT;
                            return false;
                        }
                    }
                    else if(!strcmp(sdata->active_function->name, "readn"))
                    {
                        if(sdata->active_variable->type == TYPE_NUMBER)
                            build_function_readi(sdata->active_variable->name);
                        else
                        {
                            print_error_message("Nekompatibilita v priradeni", ERROR_SEMANTIC_ASSIGNMENT, __FILE__, __LINE__);
                            sdata->return_code = ERROR_SEMANTIC_ASSIGNMENT;
                            return false;
                        }
                    }
                    else if(!strcmp(sdata->active_function->name, "reads"))
                    {
                        if(sdata->active_variable->type == TYPE_STRING)
                            build_function_readi(sdata->active_variable->name);
                        else
                        {
                            print_error_message("Nekompatibilita v priradeni", ERROR_SEMANTIC_ASSIGNMENT, __FILE__, __LINE__);
                            sdata->return_code = ERROR_SEMANTIC_ASSIGNMENT;
                            return false;
                        }
                    }
                    else if(!strcmp(sdata->active_function->name, "tointeger"))
                    {
                        if(sdata->active_variable->type == TYPE_INT || sdata->active_variable->type == TYPE_NUMBER)
                            build_function_tointeger(sdata->active_variable->name, *(sdata->token));
                        else
                        {
                            print_error_message("Nekompatibilita v priradeni", ERROR_SEMANTIC_ASSIGNMENT, __FILE__, __LINE__);
                            sdata->return_code = ERROR_SEMANTIC_ASSIGNMENT;
                            return false;
                        }
                    }
                    else
                    {
                        DLLTypesActiveFirst(sdata->active_function->params);

                        // PROGRAM
                        // Rekurzivne volanie kedze expression je volanie funkcie
                        func_expression(sdata);
                    }

                    sdata->state = previous_state;

                    sdata->active_function = previous_active_func;

                    // SYNTAKTICKA KONTROLA
                    if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;
                    if(sdata->token->type != TOKEN_TYPE_RIGHT_BRACKET)
                    {
                        print_error_message("Token na vstupe nie je ocakavany RIGHT_BRACKET", ERROR_SYNTAX_ANALYSIS, __FILE__, __LINE__);
                        sdata->return_code = ERROR_SYNTAX_ANALYSIS;
                        return false;
                    }
                }
                else
                {
                    return_token(sdata->token);
                }
                finish = true;
                break;
        }
    }

    // Ak sme v stave kde mozeme nacitavat aj viac expressions po sebe skontrolujeme ci sme este neskoncili
    if((sdata->state == STATE_FUNCTION_CALL ||
        sdata->state == STATE_RETURN ||
        sdata->state == STATE_ASSIGNMENT_TO_ID) && 
        sdata->token->type == TOKEN_TYPE_COMMA)
    {
        if(!is_next_token_valid(sdata, __FILE__, __LINE__)) return false;

        switch(sdata->state)
        {
            case STATE_FUNCTION_CALL:
                if(sdata->active_function->matters_on_params_count) DLLTypesActiveNext(sdata->active_function->params);
                break;
            case STATE_RETURN:
                DLLTypesActiveNext(sdata->active_function->return_types);
                break;
            case STATE_ASSIGNMENT_TO_ID:
                DLLVarActiveNext(&(sdata->list_of_variables));
                break;
            default:
                break;
        }
        func_expression(sdata);
    }

    free(stack);

    return true;
}

#endif
