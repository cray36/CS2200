/**
 * CS 2110 - Spring 2013 - Homework #11
 * Edited by: Brandon Whitehead, Andrew Wilder
 *
 * list.c: Complete the functions!
 */

#include <stdlib.h>
#include <stdio.h>
#include "list.h"

/* The node struct.  Has a prev pointer, next pointer, and data. */
/* DO NOT DEFINE ANYTHING OTHER THAN WHAT'S GIVEN OR YOU WILL GET A ZERO*/
/* Design consideration only this file should know about nodes */
/* Only this file should be manipulating nodes */
/* DO NOT TOUCH THIS DECLARATION DO NOT PUT IT IN OTHER FILES */
typedef struct lnode
{
  struct lnode* prev; /* Pointer to previous node */
  struct lnode* next; /* Pointer to next node */
  void* data; /* User data */
} node;


/* Do not create any global variables here. Your linked list library should obviously work for multiple linked lists */
/* This function is declared as static since you should only be calling this inside this file.
 *
 */
static node* create_node(void* data);

/** create_node
  *
  * Helper function that creates a node by allocating memory for it on the heap.
  * Be sure to set its pointers to NULL.
  *
  * @param data a void pointer to data the user wants to store in the list
  * @return a node
  */
static node* create_node(void* data)
{

    node *newNode = (node*) malloc(sizeof(node));
    newNode->data = data;
    newNode->prev = NULL;
    newNode->next = NULL;
    return newNode;
}

/** create_list
  *
  * Creates a list by allocating memory for it on the heap.
  * Be sure to initialize size to zero and head/tail to NULL.
  *
  * @return an empty linked list
  */
list* create_list(void)
{

    list *newList = (list*) malloc(sizeof(list));
    newList->head = NULL;
    newList->tail = NULL;
    newList->size = 0;
    return newList;
}

/** push_front
  *
  * Adds the data to the front of the linked list
  *
  * @param llist a pointer to the list.
  * @param data pointer to data the user wants to store in the list.
  */
void push_front(list* llist, void* data)
{
	node* newNode;
    if (llist == NULL)
        llist = create_list();

    newNode = create_node(data);
    if (llist->size)
    {
        newNode->next = llist->head;
        newNode->prev = NULL;
        llist->head->prev = newNode;
        llist->head = newNode;
    }
    else
    {
        llist->head = newNode;
        llist->tail = newNode;
    }
    llist->size++;
}


/** push_back
  *
  * Adds the data to the back/end of the linked list
  *
  * @param llist a pointer to the list.
  * @param data pointer to data the user wants to store in the list.
  */
void push_back(list* llist, void* data)
{
	node* newNode;
    if (llist == NULL)
        llist = create_list();

    newNode = create_node(data);
    if (llist->size)
    {
        newNode->next = NULL;
        newNode->prev = llist->tail;
        llist->tail->next = newNode;
        llist->tail = newNode;
    }
    else
    {
        llist->head = newNode;
        llist->tail = newNode;
    }
    llist->size++;
}

/** front
  *
  * Gets the data at the front of the linked list
  * If the list is empty return NULL.
  *
  * @param llist a pointer to the list
  * @return The data at the first node in the linked list or NULL.
  */
void* front(list* llist)
{
    return (llist->size) ? llist->head->data : NULL;
}

/** back
  *
  * Gets the data at the "end" of the linked list
  * If the list is empty return NULL.
  *
  * @param llist a pointer to the list
  * @return The data at the last node in the linked list or NULL.
  */
void* back(list* llist)
{
    return (llist->size) ? llist->tail->data : NULL;
}

/** remove_front
  *
  * Removes the node at the front of the linked list
  *
  * @warning Note the data the node is pointing to is also freed. If you have any pointers to this node's data it will be freed!
  *
  * @param llist a pointer to the list.
  * @param free_func pointer to a function that is responsible for freeing the node's data.
  * @return -1 if the remove failed (which is only there are no elements) 0 if the remove succeeded.
  */
int remove_front(list* llist, list_op free_func)
{
    int retVal = 0;
    if (is_empty(llist))
        retVal = -1;
    else {
    	node *temp;
        temp = llist->head;
        if (llist->size > 1) {
            llist->head->next->prev = NULL;
            llist->head = llist->head->next;
            free_func(temp->data);
            free(temp);
        }
        else {
            free_func(temp->data);
            free(temp);
            llist->head = NULL;
            llist->tail = NULL;
        }
        llist->size--;
    }
    return retVal;
}

/** remove_back
  *
  * Removes the node at the back of the linked list
  *
  * @warning Note the data the node is pointing to is also freed. If you have any pointers to this node's data it will be freed!
  *
  * @param llist a pointer to the list.
  * @param free_func pointer to a function that is responsible for freeing the node's data.
  * @return -1 if the remove failed 0 if the remove succeeded.
  */
int remove_back(list* llist, list_op free_func)
{
    int retVal = 0;
    if (is_empty(llist))
        retVal = -1;
    else {
        node *temp;
        temp = llist->tail;
        if (llist->size > 1) {
            llist->tail->prev->next = NULL;
            llist->tail = llist->tail->prev;
            free_func(temp->data);
            free(temp);
        }
        else {
            free_func(temp->data);
            free(temp);
            llist->head = NULL;
            llist->tail = NULL;
        }
        llist->size--;
    }
    return retVal;
}

/** copy_list
  *
  * Create a new list structure, new nodes, and new copies of the data by using
  * the copy function. Its implementation for any test structure must copy
  * EVERYTHING!
  *
  * @param llist A pointer to the linked list to make a copy of
  * @param copy_func A function pointer to a function that makes a copy of the
  *        data that's being used in this linked list, allocating space for
  *        every part of that data on the heap. This is some function you must
  *        write yourself for testing, tailored specifically to whatever context
  *        you're using the linked list for in your test.
  * @return The linked list created by copying the old one
  */
list* copy_list(list* llist, list_cpy copy_func)
{

	list* copyList = create_list();
	if (!is_empty(llist)) {
        node* traverserSrc;
        traverserSrc = llist->head;
        do {
            push_back(copyList, copy_func(traverserSrc->data));
            traverserSrc = traverserSrc->next;
        } while(traverserSrc!=NULL);
    }
	return copyList;
}

/** size
  *
  * Gets the size of the linked list
  *
  * @param llist a pointer to the list
  * @return The size of the linked list
  */
int size(list* llist)
{

    return llist->size;
}

/** remove_if
  *
  * Removes all nodes whose data when passed into the predicate function returns true
  *
  * @param llist a pointer to the list
  * @param pred_func a pointer to a function that when it returns true it will remove the element from the list and do nothing otherwise @see list_pred.
  * @param free_func a pointer to a function that is responsible for freeing the node's data
  * @return the number of nodes that were removed.
  */
int remove_if(list* llist, list_pred pred_func, list_op free_func)
{
    /* @todo Implement changing the return value!
    /// @note remember to also free all nodes you remove.
    /// @note be sure to call pred_func on the NODES DATA to check if the node needs to be removed.
    ///
    /// @note free_func is a function that is responsible for freeing the node's data only.
    */
    int count = 0;
    if (!is_empty(llist)) {
        node* traverser;
        node* tmp;
        traverser = llist->head;
        do {
            if (pred_func(traverser->data)) {
                count++;
                /* store a backup of traverser*/
                tmp = traverser;
                /* free data before free node*/

                /* connect prev and next node*/

                if (traverser->prev == NULL) {
                    /* head case*/
                    if (traverser->next != NULL)
                        traverser->next->prev = NULL;
                    llist->head = traverser->next;
                }
                else if (traverser->next == NULL) {
                    /* tail case */
                    if (traverser->prev != NULL)
                        traverser->prev->next = NULL;
                    llist->tail = traverser->prev;
                }
                else {
                    traverser->prev->next = traverser->next;
                    traverser->next->prev = traverser->prev;
                }
                traverser = traverser->next;
                free_func(tmp->data);
                free(tmp);
                llist->size--;
            }
            else
                traverser = traverser->next;
        } while(traverser!=NULL);
    }
    return count;
}

/** is_empty
  *
  * Checks to see if the list is empty.
  *
  * @param llist a pointer to the list
  * @return 1 if the list is indeed empty 0 otherwise.
  */
int is_empty(list* llist)
{
    return !(llist->size);
}

/** empty_list
  *
  * Empties the list after this is called the list should be empty.
  *
  * @param llist a pointer to a linked list.
  * @param free_func function used to free the node's data.
  */
void empty_list(list* llist, list_op free_func)
{
    if (!is_empty(llist)) {
        node* traverser;
        node* tmp;
        traverser = llist->head;
        do {
            free_func(traverser->data);
            tmp = traverser;
            traverser = traverser->next;
            free(tmp);
        } while(traverser!=NULL);
        llist->size = 0;
        llist->head = NULL;
    }
}

/** traverse
  *
  * Traverses the linked list calling a function on each node's data.
  *
  * @param llist a pointer to a linked list.
  * @param do_func a function that does something to each node's data.
  */
void traverse(list* llist, list_op do_func)
{

    if (!is_empty(llist)) {
        node* traverser;
        traverser = llist->head;
        do {
            do_func(traverser->data);
            traverser = traverser->next;
        } while(traverser!=NULL);
    }
}
