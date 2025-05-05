/**
 * Projekt: Implementacia prekladaca jazyka IFJ2021
 *
 * Autori ktori na tomto subore pracovali:
 *  Jakub Kasem
 */

#ifndef _STACK_C
#define _STACK_C

#include <stdio.h>
#include "error.h"
#include "stack.h"

//Definicie funkcii

/**
 * Inicializacia zasobniku, nastavenie vrcholu
 */
bool stack_init(Stack *stack) {
    if(stack == NULL) return false;

    stack->topIndex = -1;

    return true;
}

/**
 * Vracia true, ak je zasobnik prazdny. Inak vracia false.
 */
bool stack_is_empty( const Stack *stack )
{
    return (stack->topIndex == -1) ? true : false;
}

/**
 * Vracia a zo zasobniku cez *symbol
 */
void stack_top_a( const Stack *stack, Table_index *symbol) {

    if (!stack_is_empty(stack))
    {
        //musi ignorovat STACK_SHIFT A STACK_EXPRESSION
        int index = stack->topIndex;
        bool finish = false;

        while(!(finish))
        {
            if ((stack->array[index] != STACK_SHIFT) && (stack->array[index] != STACK_EXPRESSION))
            {
                *symbol = stack->array[index];
                finish = true;
            }
            else
                index--;
        } 
    }

    return;
}

/**
 * Skontroluje ci je na vrchole zasobniku <y
 * a cez *shift_index vracia poziciu <
 */
bool stack_top_y(const Stack *stack, int *shift_index){

    if (!stack_is_empty(stack))
    {
        //najdi <
        int index = stack->topIndex;
        bool finish = false;

        while(!(finish))
        {
            if (stack->array[index] == STACK_END)
                return false;
            else if (stack->array[index] == STACK_SHIFT)
                finish = true;
            else
                index--;
        }
        *shift_index = index;

        return true;
    }

    return false;
}

/**
 * Funkcia vrati vrchny Table_index, ak je prazdna vracia STACK_VOID
 */
Table_index stack_top(Stack *stack)
{
    return stack->array[stack->topIndex];
}

/**
 * Zameni a za a< na zasobniku
 */
void stack_change_top_a(Stack *stack)
{
    //musi ignorovat STACK_SHIFT A STACK_EXPRESSION
    int index = stack->topIndex;
    bool finish = false;

    while(!(finish))
    {
        if ((stack->array[index] != STACK_SHIFT) && (stack->array[index] != STACK_EXPRESSION))
            finish = true;
        else
            index--;
    }

    //Rozsir stack o 1
    stack_push(stack, STACK_VOID);

    //Poposuvaj symboly pred a
    for (int i = stack->topIndex; i > (index + 1); i--) //mozno zle index + 1
    {
        stack->array[i] = stack->array[i - 1];
    }

    //zamen a za a<
    stack->array[index + 1] = STACK_SHIFT;
}

/**
 * Zameni <y za A
 */
void stack_change_top_b(Stack *stack, Prec_rule rule, int shift_index) //mozno vymazat shift_index
{
    switch(rule)
    {
        case RULE_OPERAND:
            stack_pop(stack);
            stack_pop(stack);
            break;
        case RULE_LENGTH:
            stack_pop(stack);
            stack_pop(stack);
            stack_pop(stack);
            break;
        case RULE_BRACKETS: //toto idk
            stack_pop(stack);
            stack_pop(stack);
            stack_pop(stack);
            stack_pop(stack);
            break;
        default:
            stack_pop(stack);
            stack_pop(stack);
            stack_pop(stack);
            stack_pop(stack);
            break;
    }
    stack_push(stack, STACK_EXPRESSION);
}

/**
 * Odstrani prvok z vrcholu zasobniku
 */
void stack_pop(Stack *stack) {

    if (!stack_is_empty(stack)) stack->topIndex = stack->topIndex - 1;
    
    return;
}

/**
 * Vlozi znak na vrchol zasobniku
 */
void stack_push(Stack *stack, Table_index symbol) {

    stack->topIndex = stack->topIndex + 1;
    stack->array[stack->topIndex] = symbol;

    return;
}

#endif
