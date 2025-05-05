/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 * 
 * Autori ktori na tomto subore pracovali:
 * Adam Dzurilla (xdzuri00)
 * Martin Mores (xmores02)
 *
 */

#ifndef _SYMTABLE_H
#define _SYMTABLE_H

#include <stdbool.h>

/**
 * Podstatna struktura je hlavne struktura TData ktora uchovava informacie bud o premennej alebo o funkcii,
 * pomocnymi strukturami su Data_type a Data_value uchovavajuce typ a hodnotu jednej premennej, vyuzite su pri
 * definicii struktury TVariable, ktora obsahuje aj ukazatel na dalsiu premennu, toto sa vyuziva iba pri funkciach
 * Ak funkcia ma viacero vstupnych pripadne vystupnych premennych su na seba naviazane pomocou ukazatelov,
 * dalej funkcia aj premenna uchovava informaciu ci uz bola definovana jej hodnota
 */
typedef enum
{
	TYPE_UNDEFINED,
	TYPE_INT,
	TYPE_NUMBER,
	TYPE_STRING,
	TYPE_BOOLEAN,
	TYPE_NIL,
} Data_type;

// Prvok zoznamu typov
typedef struct list_type_element
{
    Data_type data;
    struct list_type_element *previousItem;
    struct list_type_element *nextItem;
} *DLLTypesElement;

// Zoznam typov
typedef struct
{
    DLLTypesElement first;
    DLLTypesElement active;
} DLLTypes;

/**
 * Prvok premennej
 */
typedef struct variable_item
{
	// Deklarovanie funkcie sa zisti podla toho ci sa nachadza v symtable
	char *name;
	Data_type type;					// Datovy typ premennej
	char *value;					// Hodnota premennej

	bool is_value_nil;
	bool defined;					// Boolean ci datam uz bola priradena hodnota
} TVariable;

/**
 * Prvok funkcie
 */
typedef struct{
	// Deklarovanie funkcie sa zisti podla toho ci sa uz nachadza v symtable
	char *name;
	bool matters_on_params_count;
	
	DLLTypes *params;
	DLLTypes *return_types;

	bool defined;					// Boolean ci uz bolo definovane telo funkcie
} TFunction;

/**
 * Item je bud funkcia alebo premenna podla toho ci sa jedna o globalnu alebo lokalnu tabulku
 */
typedef union
{
	TVariable *variable;
	TFunction *function;
} TData;

typedef struct bst_item
{
	char *key;					// Identifikator uzlu
	TData data;					// Data uzlu
	struct bst_item *left;		// Lavy syn uzlu
	struct bst_item *right;		// Pravy syn uzlu
} bst_item_t;

// Tabulka symbolov
typedef bst_item_t* Sym_table;

// Prvok zoznamu symtablov
typedef struct list_st_element
{
    Sym_table data;
    struct list_st_element *previousItem;
    struct list_st_element *nextItem;
} *DLLStElement;

// Zoznam sym tablov
typedef struct
{
    DLLStElement last;
    DLLStElement active;
} DLLSymt;

typedef struct list_var_element
{
	TVariable *data;
	struct list_var_element *nextItem;
} *DLLVarElement;

typedef struct
{
	DLLVarElement first;
	DLLVarElement active;
} DLLVariables;

// --------------------------------------------------------------------------------------------------------------
// Funkcie pre pracu s tabulkou symbolov

void bst_init_table(bst_item_t ** );

bool bst_insert(bst_item_t ** , char * , TData );

bool bst_search(bst_item_t * , char * , TData * );

void bst_dispose(bst_item_t * , bool );

TFunction *create_function( bool defined , char *name );

TVariable *create_variable( Data_type , char * , bool defined , char * );

void removeFunction( TFunction * );

void removeVariable( TVariable * );

// --------------------------------------------------------------------------------------------------------------
// Dvojito-viazany zoznam typov		(TYPES)

void DLLTypesInit( DLLTypes * );

void DLLTypesDispose( DLLTypes * );

bool DLLTypesAddLast( DLLTypes *, Data_type );

void DLLTypesActiveFirst( DLLTypes * );

void DLLTypesActiveNext( DLLTypes * );

Data_type DLLTypesGetActiveType( DLLTypes * );

bool DLLTypesIsEmpty( DLLTypes * );

int DLLTypesLength( DLLTypes * );

// --------------------------------------------------------------------------------------------------------------
// Funkcie pre pracu so zoznamom tabuliek symbolov		(SYMTABLE)

void DLLSymtInit( DLLSymt * );

void DLLSymtDispose( DLLSymt * );

void DLLSymtRemoveLast( DLLSymt * );

bool DLLSymtAddLast( DLLSymt * , Sym_table);

void DLLSymtActiveLast( DLLSymt * );

void DLLSymtActivePrevious( DLLSymt * );

Sym_table DLLSymtGetTable( DLLSymt * );

bool DLLSymtIsFirstActive( DLLSymt * );

bool DLLSymtIsEmpty( DLLSymt * );

// --------------------------------------------------------------------------------------------------------------
// Funkcie pre pracu so zoznamom premennych

void DLLVarInit( DLLVariables * );

void DLLVarDispose( DLLVariables * );

bool DLLVarAddLast( DLLVariables * , TVariable * , char * );

void DLLVarActiveFirst( DLLVariables * );

void DLLVarActiveNext( DLLVariables * );

TVariable *DLLVarGetActive( DLLVariables * );

int DLLVarLength( DLLVariables * );

bool DLLVarIsActiveAtEnd( DLLVariables * );

// --------------------------------------------------------------------------------------------------------------
// Pomocne funkcie
bool is_var_in_symt_list( DLLSymt *list , char *key , TData *data );

#endif
