/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 * Martin Mores (xmores02)
 *
 */

#ifndef _SYMTABLE_C
#define _SYMTABLE_C

#include <stdlib.h>
#include <stdio.h>      // Iba testovanie
#include "symtable.h"
#include "scanner.h"

// --------------------------------------------------------------------------------------------------------------
// POMOCNE FUNKCIE
/**
 * Pomocna funkcia na porovnavanie dvoch retazcov
 * 
 * Funkcia vracia hodnoty:
 * 1 - v pripade ze str1 je mensi ako str2
 * 2 - v pripade ze str2 je mensi ako str1
 * 0 - v pripade ze retazce su rovnake
 */
int compare_string(char *str1, char *str2){
    int i = 0;

    while(str1[i] != '\0' && str2[i] != '\0'){
        if(str1[i] < str2[i]) return 1;
        else if(str1[i] > str2[i]) return 2;
        else i++;
    }

    if (str1[i] != '\0') return 2;
    else if(str2[i] != '\0') return 1;

    return 0;
}

// --------------------------------------------------------------------------------------------------------------
// Funkcie k tabulke symbolov

/*
 * Inicializacia stromu
 * 
 * Uzivatel musi zaistit ze inicializacia sa nebude volat nad inicializovanym stromom, 
 * v opacnom pripade moze dojst k prepisu ukazatela na strom 
 * hodnotou NULL a dany strom nebude mozne uvolnit (memory leak)
 */
void bst_init_table(bst_item_t **tree){
    (*tree) = NULL;
}

/*
 * Vlozenie noveho uzla do stromu
 *
 * Ak uz uzol s danym lkucom v strome existuje je jeho datova zlozka aktualizovana
 * v opacnom pripade je vytvoreny novy uzol a je vlozeny do stromu
 */
bool bst_insert(bst_item_t **tree, char *key, TData data){
   static bst_item_t *p_root = NULL;  // ukazatel na rodica uzla

  if((*tree) == NULL){
    // uzol neexistuje
    bst_item_t *new_root = (bst_item_t*)malloc(sizeof(bst_item_t));
    if(new_root == NULL) return false;
    new_root->left = NULL;
    new_root->right = NULL;
    new_root->key = key;
    new_root->data = data;

    if(p_root == NULL){
        // otec neexituje, takze strom bol prazdny
        (*tree) = new_root;
    }else{
        // otec existuje tak napojime uzol na otca
        if(compare_string(p_root->key, key) == 2)
          p_root->left = new_root;
        else
          p_root->right = new_root;
    }

    p_root = NULL; // nastavime otca na NULL pre dalsie pouzitie funkcie
  }else if(compare_string((*tree)->key, key) == 0){
    // uzol s klucom existuje, takze aktualizujeme jeho datovu zlozku
    (*tree)->data = data;
    p_root = NULL;
  }else{
    // pokracujeme podla kluca bud v lavom alebo pravom podstrome
    p_root = (*tree);
    if(compare_string((*tree)->key, key) == 2)
      bst_insert(&(*tree)->left, key, data);
    else
      bst_insert(&(*tree)->right, key, data);
  }

  return true;
}

/*
 * Hladanie uzla v strome
 *
 * Funkcia vracia pravdivostnu hodnotu ci uzol s danym klucom je v strome, 
 * v pripade hodnoty true je datova zlozka vratena prostrednictvom ukazatela
 */
bool bst_search(bst_item_t *tree, char *key, TData *data){
    if(tree == NULL)
      return false;
    else if (compare_string(key, tree->key) == 1) 
      // hladame v lavom podstrome
      return bst_search(tree->left, key, data);
    else if (compare_string(key, tree->key) == 2)
      // hladame v pravom podstrome
      return bst_search(tree->right, key, data);
    else {
      // nasli sme uzol s danym klucom
      *data = tree->data;
      return true;
    }
}

/*
 * Vymazanie celeho stromu
 *
 * Funkcia korektne uvolni vsetku pamet alokovanu pre dany strom
 * strom sa bude nachadzat v stave ako po alokacii
 */
void bst_dispose(bst_item_t *tree, bool is_function){
    if(tree != NULL){
        bst_dispose(tree->left, is_function);
        bst_dispose(tree->right, is_function);

        if(is_function)
            removeFunction(tree->data.function);
        else
            removeVariable(tree->data.variable);

        free(tree);
        tree = NULL;
    }
}

/**
 * Funkcia alokuje miesto a inicializuje pociatocne hodnoty funkcie a vrati ukazatel na nu
 * V pripade chybnej alokacie vracia NULL
 */
TFunction *create_function( bool defined , char *name )
{
    // Alokacia pamate pre premennu funkcie
    TFunction *element;
    element = (TFunction *)malloc(sizeof(TFunction));
    if(element == NULL) return NULL;

    // Alokacia pamate pre zoznam typov parametrov
    element->defined = defined;
    element->params = (DLLTypes *)malloc(sizeof(DLLTypes));
    if(element->params == NULL){
        free(element);
        return NULL;
    }

    // Alokacia pamate pre zoznam typov navratovych hodnot
    element->return_types = (DLLTypes *)malloc(sizeof(DLLTypes));
    if(element->return_types == NULL){
        free(element->params);
        free(element);
        return NULL;
    }

    element->name = name;
    // Inicializacia zoznamov v pripade uspesnych alokacii
    DLLTypesInit(element->params);
    DLLTypesInit(element->return_types);

    element->matters_on_params_count = true;

    return element;
}

/**
 * Funkcia alokuje miesto a inicializuje pociatocne hodnoty premennej a vrati ukazatel na nu
 * V pripade chybnej alokacie vracia NULL
 */
TVariable *create_variable( Data_type dtype , char *dvalue , bool defined , char *name)
{
    TVariable *element;
    element = (TVariable *)malloc(sizeof(TVariable));
    if(element == NULL) return NULL;

    element->is_value_nil = false;
    element->type = dtype;
    element->name = name;
    element->value = dvalue;
    element->defined = defined;

    return element;
}

/**
 * Funkcia korektne uvolni vsetok alokovany priestor pre funkciu
 */
void removeFunction( TFunction *function )
{
    DLLTypesDispose(function->params);
    DLLTypesDispose(function->return_types);

    free(function->params);
    free(function->return_types);
    free(function);
    function = NULL;
}

/**
 * Funkcia korektne uvolni vsetok alokovany priestor pre premennu
 */
void removeVariable( TVariable *variable )
{
    free(variable);
    variable = NULL;
}

// --------------------------------------------------------------------------------------------------------------
// Funkcie pre pracu so zoznamom typov

/**
 * Funkcia inicializuje pociatocne hodnoty zoznamu
 */
void DLLTypesInit( DLLTypes *list )
{
    list->first = NULL;
    list->active = NULL;
}

/**
 * Funkcia korektne uvolni vsetko alokovane miesto daneho zoznamu a uvedie zoznam do stavu ako po inicializacii
 */
void DLLTypesDispose( DLLTypes *list )
{
    if(!DLLTypesIsEmpty(list)){
        while(list->first != NULL){
            DLLTypesElement element;
            element = list->first;
            list->first = element->nextItem;
            free(element);
        }

        list->first = NULL;
        list->active = NULL;
    }
}

/**
 * Funkcia prida element nakoniec listu
 * funkcia vracia uspesnost vlozenia
 */
bool DLLTypesAddLast( DLLTypes *list, Data_type data)
{
    DLLTypesElement element;
    element = (DLLTypesElement)malloc(sizeof(struct list_type_element));

    if(element == NULL) return false;

    element->data = data;
    element->nextItem = NULL;

    // Zistime ci zoznam je prazdny
    if(list->first == NULL)
    {
        element->previousItem = NULL;
        list->first = element;
    }
    else    // Zoznam nie je prazdny
    {
        // Nastavime posledny prvok zoznamu
        DLLTypesElement lastElement = list->first;
        while(lastElement->nextItem != NULL) lastElement = lastElement->nextItem;

        // Prepojime prvky medzi sebou
        element->previousItem = lastElement;
        lastElement->nextItem = element;
    }

    return true;
}

/**
 * Funkcia nastavi ako aktivny prvy prvok zoznamu
 */
void DLLTypesActiveFirst( DLLTypes *list )
{
    list->active = list->first;
}

/**
 * Funkcia posunie aktivny prvok v zozname
 */
void DLLTypesActiveNext( DLLTypes *list )
{
    if(list->active != NULL) list->active = list->active->nextItem;
}

/**
 * Funkcia vrati data aktivnej polozky
 */
Data_type DLLTypesGetActiveType( DLLTypes *list )
{
    if(list->active == NULL) return TYPE_UNDEFINED;

    return list->active->data;
}

bool DLLTypesIsEmpty( DLLTypes * list )
{
    return list->first == NULL;
}

/**
 * Funkcia vracia celociselnu hodnotu poctu prvkov v zozname, funkcia ponecha povodny ukazatel na aktivnu polozku
 */
int DLLTypesLength( DLLTypes *list )
{
    // Overime ci zoznam nie je prazdny
    if(DLLTypesIsEmpty(list)) return 0;

    // Zalohujeme si pointer na aktivnu polozku
    DLLTypesElement original_active = list->active;

    int length;

    // Nastavime aktivnost na zaciatok zoznamu
    DLLTypesActiveFirst(list);

    // Cyklus cez zoznam
    for(length = 1; list->active->nextItem != NULL; length++)
    {
        // Posuvame ukazatel az kym nie sme na poslednej polozke
        DLLTypesActiveNext(list);
    }

    // Vratime aktivnost zoznamu do pociatocneho stavu
    list->active = original_active;

    // Vraciame dlzku zoznamu
    return length;
}

// --------------------------------------------------------------------------------------------------------------
// Funkcie k dvojito viazanemu zoznamu symtables

/**
 * Inicializacia zoznamu symtablov
 */
void DLLSymtInit( DLLSymt *list )
{
    list->last = NULL;
    list->active = NULL;
}

/**
 * Funkcia odstrani vsetky prvky zo stromu a nastavi strom na pociatocne hodnoty
 */
void DLLSymtDispose( DLLSymt *list )
{
    while(list->last != NULL)
    {
        DLLSymtRemoveLast(list);
    }

    list = NULL;
}

/**
 * Funkcia odstrani posledny symtable v zozname
 */
void DLLSymtRemoveLast( DLLSymt *list )
{
    DLLStElement element;
    element = list->last;
    list->last = element->previousItem;
    if(element->previousItem != NULL) element->previousItem->nextItem = NULL;

    bst_dispose(element->data, false);
    free(element);
}

/**
 * Funkcia prida novy symtable nakoniec zoznamu
 */
bool DLLSymtAddLast( DLLSymt *list , Sym_table data)
{
    DLLStElement element;
    element = (DLLStElement)malloc(sizeof(struct list_st_element));

    if(element == NULL) return false;

    element->nextItem = NULL;
    element->data = data;

    // Skontrolujeme ci zoznam je prazdny
    if(list->last == NULL)
    {
        element->previousItem = NULL;
    }
    else    // Zoznam nie je prazdny
    {
        // Previazeme posledny prvok s novo pridanym prvkom ktory vlozime nakoniec zoznamu
        element->previousItem = list->last;
        list->last->nextItem = element;
    }

    list->last = element;

    return true;
}

/**
 * Funkcia nastavi aktivnost zoznamu na posledny prvok
 */
void DLLSymtActiveLast( DLLSymt *list )
{
    if(list != NULL) list->active = list->last;
}

/**
 * Funkcia nastavi aktivnym predchadzajuci symtable
 */
void DLLSymtActivePrevious( DLLSymt *list )
{
    if(list != NULL) list->active = list->active->previousItem;
}

/**
 * Funkcia vrati symbolov z aktivneho prvku zoznamu
 */
Sym_table DLLSymtGetTable( DLLSymt *list )
{
    return list->active->data;
}

/**
 * Funkcia vrati boolean hodnotu ci aktivny prvok je prvym prvkom zoznamu
 */
bool DLLSymtIsFirstActive( DLLSymt *list )
{
    return list->active->previousItem == NULL;
}

/**
 * Funkcia vracia boolean hodnotu ci je prazdna
 */
bool DLLSymtIsEmpty( DLLSymt *list )
{
    return list->last == NULL;
}

// --------------------------------------------------------------------------------------------------------------
// Funkcie pre pracu so zoznamom premennych

/**
 * Funkcie inicializuje pociatocne hodnoty zoznamu
 */
void DLLVarInit( DLLVariables *list )
{
    list->first = NULL;
    list->active = NULL;
}

/**
 * Funkcie odstrani vsetky prvky zo zoznamu ale neuvolni pamat, tu uvolni az
 * odstranenie stromu v ktorom sa premenna vyskytuje
 */
void DLLVarDispose( DLLVariables *list )
{
    while(list->first != NULL)
    {
        DLLVarElement ptr = list->first;
        list->first = ptr->nextItem;
        free(ptr);
    }

    list->first = NULL;
    list->active = NULL;
}

/**
 * Funkcia prida premennu na koniec zoznamu
 */
bool DLLVarAddLast( DLLVariables *list , TVariable *data , char *name)
{
    DLLVarElement element;
    element = (DLLVarElement)malloc(sizeof(struct list_var_element));

    if(element == NULL) return false;

    element->nextItem = NULL;
    element->data = data;

    if(list->first == NULL)
    {
        list->first = element;
    }
    else
    {
        DLLVarElement last_element;
        last_element = list->first;
        while(last_element->nextItem != NULL) last_element = last_element->nextItem;

        last_element->nextItem = element;
    }

    return true;
}

/**
 * Funkcia nastavi aktivnost prvku na prvy prvok v zozname
 */
void DLLVarActiveFirst( DLLVariables *list )
{
    if(list != NULL) list->active = list->first;
}

/**
 * Funkcia posunie aktivnost prvku
 */
void DLLVarActiveNext( DLLVariables *list )
{
    if(list != NULL && list->active != NULL) list->active = list->active->nextItem;
}

/**
 * Funkcia vrati aktivne data z daneho zoznamu
 */
TVariable *DLLVarGetActive( DLLVariables *list )
{
    if(list == NULL || list->active == NULL) return NULL;

    return list->active->data;
}

/**
 * Funkcia vrati pocet prvkov v zozname premennych
 */
int DLLVarLength( DLLVariables *list )
{
    // Skontrolujeme ci zoznam nie je prazdny
    if(list->first == NULL) return 0;

    // Zalohujeme si povodnu aktivnu zlozku
    DLLVarElement original_active = list->active;
    
    // Posunieme aktivnost na zaciatok
    DLLVarActiveFirst(list);

    // Definujeme si premennu v ktorej budeme pocitat dlzku zoznamu
    int length;

    // Prechadzame zoznamom kym nie sme na konci
    for(length = 1; list->active->nextItem != NULL; length++)
    {
        DLLVarActiveNext(list);
    }

    // Obnovime aktivnu zlozku zoznamu
    list->active = original_active;

    // Vraciame dlzku zoznamu
    return length;
}

/**
 * Funkcia vracia pravdivostnu hodnotu ci aktivny prvok je poslednym v zozname prvkov
 */
bool DLLVarIsActiveAtEnd( DLLVariables *list )
{
    if(list->active->nextItem == NULL)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// --------------------------------------------------------------------------------------------------------------
// Pomocne funkcie

/**
 * Funkcia prejde vsetky symtables a skontroluje ci je tam dana premenna podla kluca, v pripade uspechu
 * vracia data pomocou ukazatela
 */
bool is_var_in_symt_list( DLLSymt *list , char *key , TData *data )
{
    DLLSymtActiveLast(list);
    // Skontrolujeme ci nahodou zoznam nie je prazdny
    if(list->active == NULL)
    {
        return false;
    }

    bool found = false;
    bool end = false;
    while(!end)
    {
        if(bst_search(list->active->data, key, data))
        {
            // Nasli sme premennu v strome
            end = true;
            found = true;
        }
        else
        {
            // Nenasli sme v strome
            if(DLLSymtIsFirstActive(list))
            {
                // Koncime neuspesne v hladani
                end = true;
            }
            else
            {
                // Posunieme level o jeden vyssie a prehladavame dalsi strom
                DLLSymtActivePrevious(list);
            }
        }
    }

    return found;
}

#endif
