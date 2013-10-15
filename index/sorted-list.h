#ifndef SORTED_LIST_H
#define SORTED_LIST_H
/*
 * sorted-list.h
 */

#include <stdlib.h>

/*
 * Sorted list type.  You need to fill in the type as part of your implementation.
 */
struct SortedList
{
	/* The function by which we compare objects. */
    int (*function)(void *, void *);
	
    /* The head into the list. */
    struct listNode *head;
	
	struct SortedList *nextList;
	
    /* The number of nodes in the list. */
    int nodeCount;
};
typedef struct SortedList* SortedListPtr;

/*
 * The struct that contains a piece of data and the pointer to the next Node
 */
struct listNode {
	/* The object this node is wrapping. */
    void *object;
	
    /* The number of persistent pointers referencing this node. */
    size_t inbound_ptr_count;
	
    /* This node's child, if it exists. */
    struct listNode *next;
	
    /* This node's parent, if it exists. */
    struct listNode *prev;
};
typedef struct listNode *Node;

/*
 * Iterator type for user to "walk" through the list item by item, from
 * beginning to end.  You need to fill in the type as part of your implementation.
 */
struct SortedListIterator
{
	/* Pointer to the node that this iterator currently has reference to. */
    struct listNode* current_node;
	
    /* Pointer to the list backing this iterator. */
    SortedListPtr list;
	
    /*
     * Boolean value stating whether this iterator has started.
     * Value is 0 if iteration has not yet started; otherwise, value is 1.
     */
    int started;
	
};
typedef struct SortedListIterator* SortedListIteratorPtr;


/*
 * When your sorted list is used to store objects of some type, since the
 * type is opaque to you, you will need a comparator function to order
 * the objects in your sorted list.
 *
 * You can expect a comparator function to return -1 if the 1st object is
 * smaller, 0 if the two objects are equal, and 1 if the 2nd object is
 * smaller.
 *
 * Note that you are not expected to implement any comparator functions.
 * You will be given a comparator function when a new sorted list is
 * created.
 */

typedef int (*CompareFuncT)(void *, void *);


/*
 * SLCreate creates a new, empty sorted list.  The caller must provide
 * a comparator function that can be used to order objects that will be
 * kept in the list.
 *
 * If the function succeeds, it returns a (non-NULL) SortedListT object.
 * Else, it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

SortedListPtr SLCreate(CompareFuncT cf);

/*
 * SLDestroy destroys a list, freeing all dynamically allocated memory.
 *
 * You need to fill in this function as part of your implementation.
 */
void SLDestroy(SortedListPtr list);


/*
 * SLInsert inserts a given object into a sorted list, maintaining sorted
 * order of all objects in the list.  If the new object is equal to a subset
 * of existing objects in the list, then the subset can be kept in any
 * order.
 *
 * If the function succeeds, it returns 1.  Else, it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

int SLInsert(SortedListPtr list, void *newObj);


/*
 * SLRemove removes a given object from a sorted list.  Sorted ordering
 * should be maintained.
 *
 * If the function succeeds, it returns 1.  Else, it returns 0.
 *
 * You need to fill in this function as part of your implementation.
 */

int SLRemove(SortedListPtr list, void *newObj);


/*
 * SLCreateIterator creates an iterator object that will allow the caller
 * to "walk" through the list from beginning to the end using SLNextItem.
 *
 * If the function succeeds, it returns a non-NULL SortedListIterT object.
 * Else, it returns NULL.
 *
 * You need to fill in this function as part of your implementation.
 */

SortedListIteratorPtr SLCreateIterator(SortedListPtr list);


/*
 * SLDestroyIterator destroys an iterator object that was created using
 * SLCreateIterator().  Note that this function should destroy the
 * iterator but should NOT affectt the original list used to create
 * the iterator in any way.
 *
 * You need to fill in this function as part of your implementation.
 */

void SLDestroyIterator(SortedListIteratorPtr iter);

int SLHasNext(SortedListIteratorPtr iter);

/*
 * SLNextItem returns the next object in the list encapsulated by the
 * given iterator.  It should return a NULL when the end of the list
 * has been reached.
 *
 * One complication you MUST consider/address is what happens if a
 * sorted list encapsulated within an iterator is modified while that
 * iterator is active.  For example, what if an iterator is "pointing"
 * to some object in the list as the next one to be returned but that
 * object is removed from the list using SLRemove() before SLNextItem()
 * is called.
 *
 * You need to fill in this function as part of your implementation.
 */

void *SLNextItem(SortedListIteratorPtr iter);

/*
 * Free the given node.
 */
int deleteNode( Node );

/*
 * Find the node in the given list wrapping the given void*.
 */
Node findNode( SortedListPtr, void* );

/*
 * Generate a new, initialized node.
 */
Node generateNode();

/*
 * Prepare the given node for removal from the associated list.
 * Specifically, realign all pointers s.t. the list no longer has reference to the node.
 * The node may, however, still have reference back into the list.
 */
void primeNodeForRemoval( SortedListPtr, Node );

/*
 * Shift the node containing the target up the list, if its contents have changed
 * such that the list no longer has its correct descending order.
 */
void shiftNodeUp( SortedListPtr list, void* target );

#endif
