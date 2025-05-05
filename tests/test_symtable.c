/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 *
 */

#ifndef _TEST_SYMTABLE_C
#define _TEST_SYMTABLE_C

#include <stdio.h>
#include <stdlib.h>
#include "../symtable.h"

void print_type(Data_type type)
{
    switch(type)
    {
        case TYPE_UNDEFINED:
            printf("UNDEFINED");
            break;
        case TYPE_INT:
            printf("INT");
            break;
        case TYPE_NUMBER:
            printf("DOUBLE");
            break;
        case TYPE_NUMBER_EXPONENT:
            printf("DOUBLE W EXPONENT");
            break;
        case TYPE_STRING:
            printf("STRING");
            break;
        case TYPE_BOOLEAN:
            printf("BOOLEAN");
            break;
        case TYPE_NIL:
            printf("NIL");
            break;
    }
}

void print_table_item(bst_item_t *tree, bool is_function)
{
    printf("Key: %s\n", tree->key);
    if(is_function)
    {
        TFunction *func = tree->data.function;
        printf("Je to funkcia ");
        if(func->defined)
            printf("(definovana)\n");
        else
            printf("(nedefinovana)\n");

        printf("Prvky params: ");
        DLLTypesActiveFirst(func->params);
        while(func->params->active != NULL)
        {
            print_type(DLLTypesGetActiveType(func->params));
            DLLTypesActiveNext(func->params);
            printf(" ");
        }
        printf("(%d)\n", DLLTypesLength(func->params));

        printf("Prvky return_types: ");
        DLLTypesActiveFirst(func->return_types);
        while(func->return_types->active != NULL)
        {
            print_type(DLLTypesGetActiveType(func->return_types));
            DLLTypesActiveNext(func->return_types);
            printf(" ");
        }
        printf("(%d)\n", DLLTypesLength(func->return_types));
    }
    else
    {
        TVariable *var = tree->data.variable;
        printf("Je to premenna ");
        if(var->defined)
        {
            printf("(definovana)\n");
            printf("Hodnota premennej je: ");
            switch(var->type){
                case TYPE_UNDEFINED:
                case TYPE_BOOLEAN:
                case TYPE_NIL:
                    printf("undef || bool || nil\n");
                    break;
                case TYPE_INT:
                    printf("%d\n", var->value.integer);
                    break;
                case TYPE_NUMBER:
                    printf("%f\n", var->value.decimal);
                    break;
                case TYPE_NUMBER_EXPONENT:
                    printf("%s\n", var->value.string);
                    break;
                case TYPE_STRING:
                    printf("%s\n", var->value.string);
                    break;
            }
        }
        else
            printf("(nedefinovana)\n");
    }

    printf("\n");
}

// Vypisanie symtable pomocou preorder
void preorder(bst_item_t *tree, bool is_function)
{
    if(tree != NULL)
    {
        print_table_item(tree, is_function);

        if(tree->left != NULL) printf("Lavy syn od %s: \n", tree->key);
        preorder(tree->left, is_function);

        if(tree->right != NULL) printf("Pravy syn od %s: \n", tree->key);
        preorder(tree->right, is_function);
    }
}

void test_function_tree()
{
    TData data;

    Sym_table global_table;
    bst_init_table(&global_table);

    TFunction *func = create_function(false);

    TFunction *func_boo = create_function(false);

    data.function = func;
    bst_insert(&global_table, "funkc", data);
    data.function = func_boo;
    bst_insert(&global_table, "boo", data);
    data.function = create_function(true);
    bst_insert(&global_table, "opes", data);

    if(bst_search(global_table, "boo", &data)){
        DLLTypesAddLast(data.function->params, TYPE_INT);
        DLLTypesAddLast(data.function->params, TYPE_NUMBER);
    }

    if(bst_search(global_table, "ako", &data)){
        printf("Nasiel som nieco co som najst nemal\n");
    }

    if(bst_search(global_table, "opes", &data))
    {
        DLLTypesAddLast(data.function->params, TYPE_INT);
        DLLTypesAddLast(data.function->return_types, TYPE_BOOLEAN);
    }

    preorder(global_table, true);

    bst_dispose(global_table, true);
}

void test_variables_tree()
{
    TData data;

    Sym_table local_table;
    bst_init_table(&local_table);

    Data_value datav;
    datav.integer = 5;
    TVariable *variable = create_variable(TYPE_INT, datav, true);
    data.variable = variable;
    bst_insert(&local_table, "hodnota5", data);

    preorder(local_table, false);

    bst_dispose(local_table, false);
}

void test_types_list()
{
    DLLTypes *list = (DLLTypes *)malloc(sizeof(DLLTypes));

    DLLTypesInit(list);

    // Vytlacit by sa malo ze list je prazdny
    if(DLLTypesIsEmpty(list)) printf("List je prazdny\n");
    else printf("CHYBA %d\n", __LINE__);

    DLLTypesAddLast(list, TYPE_INT);
    DLLTypesAddLast(list, TYPE_STRING);
    DLLTypesAddLast(list, TYPE_BOOLEAN);
    DLLTypesAddLast(list, TYPE_BOOLEAN);
    DLLTypesAddLast(list, TYPE_NIL);

    if(DLLTypesIsEmpty(list)) printf("Chyba %d\n", __LINE__);
    else printf("List nie je prazdny\n");

    DLLTypesActiveFirst(list);

    int length = DLLTypesLength(list);
    printf("Dlzka zoznamu je: %d\n", length);

    for(int i = 0; i < length; i++)
    {
        print_type(DLLTypesGetActiveType(list));
        printf(" ");
        DLLTypesActiveNext(list);
    }
    printf("\n");

    DLLTypesDispose(list);

    free(list);
}

void test_symtables_list()
{
    Sym_table table1, table2, table3, table4;
    bst_init_table(&table1);
    bst_init_table(&table2);
    bst_init_table(&table3);
    bst_init_table(&table4);

    TData data;
    Data_value dvalue;

    data.variable = create_variable(TYPE_INT, dvalue, false);
    bst_insert(&table1, "a - tab1 ", data);

    data.variable = create_variable(TYPE_BOOLEAN, dvalue, false);
    bst_insert(&table2, "b - tab2", data);

    dvalue.string = "ahoj";
    data.variable = create_variable(TYPE_STRING, dvalue, true);
    bst_insert(&table4, "a - tab4", data);

    data.variable = create_variable(TYPE_NIL, dvalue, false);
    bst_insert(&table3, "b - tab3", data);

    dvalue.integer = 5;
    data.variable = create_variable(TYPE_INT, dvalue, true);
    bst_insert(&table2, "c - tab2", data);

    DLLSymt *list = (DLLSymt *)malloc(sizeof(DLLSymt));
    DLLSymtInit(list);

    DLLSymtAddLast(list, table1);
    DLLSymtAddLast(list, table2);
    DLLSymtAddLast(list, table3);
    DLLSymtAddLast(list, table4);

    DLLSymtActiveLast(list);

    /*if(!DLLSymtIsEmpty(list))
        printf("Nie je prazdny\n");
    else
        printf("CHYBA\n");*/

    while(!DLLSymtIsFirstActive(list))
    {
        preorder(list->active->data, false);
        DLLSymtActivePrevious(list);
    }
    preorder(list->active->data, false);

    DLLSymtDispose(list);

    free(list);
}

void test_variables_list()
{
    DLLVariables *list = (DLLVariables *)malloc(sizeof(DLLVariables));
    DLLVarInit(list);

    Sym_table local_table;
    bst_init_table(&local_table);

    TData data;
    Data_value dvalue;
    data.variable = create_variable(TYPE_STRING, dvalue, false);
    bst_insert(&local_table, "g", data);

    data.variable = create_variable(TYPE_INT, dvalue, false);
    bst_insert(&local_table, "c", data);

    data.variable = create_variable(TYPE_NUMBER, dvalue, false);
    bst_insert(&local_table, "x", data);

    dvalue.integer = 15;
    data.variable = create_variable(TYPE_INT, dvalue, true);
    bst_insert(&local_table, "b", data);

    if(!bst_search(local_table, "c", &data)) printf("CHYBA %d\n", __LINE__);

    DLLVarAddLast(list, data.variable);

    if(!bst_search(local_table, "x", &data)) printf("CHYBA %d\n", __LINE__);

    DLLVarAddLast(list, data.variable);

    if(!bst_search(local_table, "b", &data)) printf("CHYBA %d\n", __LINE__);

    DLLVarAddLast(list, data.variable);

    DLLVarActiveFirst(list);
    while(list->active != NULL)
    {
        TVariable *var = DLLVarGetActive(list);
        if(var == NULL)
        {
            printf("Chyba\n");
        }

        print_type(var->type);
        printf("\n");
        if(var->defined)
        {
            printf("Premenna je definovana\n");
            printf("Hodnota: ");
            if(var->type == TYPE_INT) printf("%d\n", var->value.integer);
            else if(var->type == TYPE_STRING) printf("%s\n", var->value.string);
            else if(var->type == TYPE_NUMBER) printf("%g\n", var->value.decimal);
            else if(var->type == TYPE_NUMBER_EXPONENT) printf("%s\n", var->value.string);
            else if(var->type == TYPE_NIL) printf("NIL\n");
        }
        else
            printf("Premenna nie je definovana\n\n");

        DLLVarActiveNext(list);
    }

    DLLVarDispose(list);
    bst_dispose(local_table, false);

    free(list);
}

int main()
{
    //test_function_tree();

    //test_variables_tree();

    //test_types_list();

    //test_symtables_list();

    test_variables_list();

    return 0;
}

#endif
