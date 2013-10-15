#include <stdio.h>
#include <stdlib.h>
#include "sorted-list.h"

/*
 * Efficiency Analysis
 *
 * SLCreate, SLCreateIterator, and generateNode all call malloc to allocate memory
 * on their associated object instantiations.  Malloc is the defining operations,
 * so each call to these is at most O(time to malloc), which is different depending on
 * factors such as OS, hardware, etc.  Generally we can consider it to be O(1) relative to
 * other, more frequent, costly operations, chiefly comparisons.
 *
 * The function deleteNode is defined similarly, as its major operation is a call to free,
 * which is outside of the control of this program.  We therefore also consider free calls to
 * be O(1) for the purposes of this analysis, as again their costs are amortized over the
 * infrequency of their calls.
 *
 * SLInsert, SLRemove, findNode, and SLDestroy operate in O(n) time in the worst case,
 * where n is the number of nodes in the list.
 *
 * SLNextItem and primeNodeForRemoval are O(1) time functions, as they're simply pointer
 * reassignment functions.
 *
 * Since only one function at a time can be called by an external caller, and no O(n)
 * functions are nested internally, then from the perspective of the caller
 * this program has a worst case run time efficiency of O(n), where n is the number of
 * nodes in the list.
 *
 */

/*
 * Create a new list with the associated comparison function.
 * If the comparison function passed in is null, returns null.
 * Otherwise, returns the pointer to the list.
 */
SortedListPtr SLCreate(CompareFuncT cf)
{
	if( cf == NULL )
	{
		return NULL;
	}
	
    SortedListPtr list = malloc( sizeof( *list ) );
    list->function = cf;
    list->head = NULL;
	list->nextList = NULL;
    list->nodeCount = 0;
    
    return list;
}

/*
 * Insert the given new_object pointer into the associated list.
 *
 * Returns 1 if successfully inserted; 0 otherwise.
 */
int SLInsert( SortedListPtr list, void* new_object )
{
	Node new_node = generateNode();
	new_node->object = new_object;
	
	//If the list is empty, insert the new node as head of the list.
    if( list->nodeCount == 0 )
    {
        list->head = new_node;
        new_node->inbound_ptr_count++;
        list->nodeCount++;
		
        return 1;
    }
    
    //Else we iterate through the list, and insert the new node at the proper location.
    Node curr = list->head;
    int compareResult = 0;
    
    while( curr != NULL )
    {
    	compareResult = list->function(new_object, curr->object);
		
    	//If new_object is greater than or equal to the current object, insert
    	//the new node in front of the node wrapping the current object (i.e. curr).
    	if( compareResult > 0 || compareResult == 0 )
    	{
    		//If a node exists before curr (i.e. prev), reassign its pointer to new node.
    		if( curr->prev != NULL )
    		{
    			//If prev points to an existing next node, decrement that node's
    			//inbound ptr count as we're reassigning prev->next to curr.
    			if( curr->prev->next != NULL )
    			{
    				curr->prev->next->inbound_ptr_count--;
    			}
				
    			//Assign prev's next pointer to point to the new node.
    			curr->prev->next = new_node;
    			new_node->inbound_ptr_count++;
				
    			//Assign new node's prev back at curr's prev.
    			new_node->prev = curr->prev;
    			curr->prev->inbound_ptr_count++;
				
    		}//End if
			
    		//Otherwise we're at the head of the list, and
    		//so we assign new node's prev to NULL.
    		else
    		{
    			new_node->prev = NULL;
    		}
			
    		//If curr is not null, then we assign its pointer to the new node.
    		if( curr != NULL )
    		{
    			//If a node exists before curr, we decrement its inbound pointer count,
    			//as curr will now be pointing back at the new node.
    			if( curr->prev != NULL )
    			{
    				curr->prev->inbound_ptr_count--;
    			}
				
    			//Assign curr's prev pointer to point to the new node.
    			curr->prev = new_node;
    			new_node->inbound_ptr_count++;
				
    			//Assign new node's next to point at curr.
    			new_node->next = curr;
    			curr->inbound_ptr_count++;
				
    		}//END if
			
    		//As we're inserting ahead of curr, if curr is head, then we must make the
    		//new node head.
    		if( curr == list->head )
    		{
    			list->head = new_node;
    			curr->inbound_ptr_count--;
    			new_node->inbound_ptr_count++;
    		}
			
    		//Increment the size of the list, and return success.
    		list->nodeCount++;
    		return 1;
			
    	}//END if
		
    	//If new_object is less than the current object, then we continue through the list.
    	if( compareResult < 0 )
    	{
    		//If we reach the end of the list and our new_object is still less than the
    		//current object, then we insert the new node at the tail.
    		if( curr->next == NULL )
    		{
    			//Assign curr's next pointer to point to the new node.
    			curr->next = new_node;
    			new_node->inbound_ptr_count++;
				
    			//Assign the new node's prev point to point back at curr.
    			new_node->prev = curr;
    			curr->inbound_ptr_count++;
				
    			//Since we're inserting at the tail, we assign new node's next to NULL.
    			new_node->next = NULL;
				
    			//Increment the size of the list, and return succss.
    			list->nodeCount++;
    			return 1;
				
    		}//END if
    	}//END if
		
    	//If we have not yet inserted the new node,
    	//get the next nodes and the list and continue.
    	curr = curr->next;
		
    }//END while
    
    //If we were unable to insert the new node, free the node and report failure.
    free( new_node );
    return 0;
	
}//END SLInsert

/*
 * Destroy the given list, free its constituent nodes before itself is freed.
 */
void SLDestroy(SortedListPtr list)
{
	Node curr = list->head;
	Node temp = NULL;
	
	while( curr != NULL )
	{
		temp = curr;
		curr = curr->next;
		deleteNode(temp);
	}
	
	free( list );
}

/*
 * Remove the node containing the target object from the given list.
 * The node is freed if and only if no persistent object has a
 * reference to the node.
 *
 * Returns 1 if the node has been removed from the list; 0 otherwise.
 */
int SLRemove( SortedListPtr list, void* target_object )
{
	//Obtain the node containing the target object.
	Node found_node = findNode( list, target_object );
	
	//If the target object is not in the list, report failure.
	if( found_node == NULL )
	{
		return 0;
	}
	
	//Delink the node from the list s.t. the list cannot reference the node.
	primeNodeForRemoval( list, found_node );
	
	//If the node has no persistent objects referencing it, free it.
	if( found_node->inbound_ptr_count == 0 )
	{
		deleteNode( found_node );
	}
	
	//Decrement the list's size, and report success.
	list->nodeCount--;
	
	return 1;
}

/*
 * Create a new, initialized iterator associated with the given list.
 * If list is NULL, returns NULL.  Otherwise, returns
 * a pointer to the iterator.
 */
SortedListIteratorPtr SLCreateIterator(SortedListPtr list)
{
	if( list == NULL )
	{
		return NULL;
	}
	
	SortedListIteratorPtr iterator = malloc(sizeof(*iterator));
	iterator->current_node = NULL;
	iterator->started = 0;
	iterator->list = list;
	
	return iterator;
}

/*
 * If the given iterator exists, free it.
 */
void SLDestroyIterator(SortedListIteratorPtr iter)
{
	if( iter != NULL )
	{
		//If this iterator still has reference to a node, decrement that node's
		//inbound pointer count before freeing the iterator.
		if( iter->current_node != NULL )
		{
			iter->current_node->inbound_ptr_count--;
		}
		free(iter);
	}
}

int SLHasNext( SortedListIteratorPtr iterator )
{
	if( !iterator->started )
	{
		if( iterator->list->nodeCount > 0 )
		{
			return 1;
		}
		else{
			return 0;
		}
	}
	else
	{
		if( iterator->current_node->next == NULL )
		{
			return 0;
		}
		else
		{
			return 1;
		}
	}
}

/*
 * Obtain the next object in the list associated with the given iterator.
 * This will also cause iterator to have reference to the node wrapping the object
 * through its current_node pointer.
 *
 * If the iterator is NULL, or the iterator has past the end of the list, returns NULL.
 * Otherwise, returns a pointer to the object wrapped by the iterator's current_node.
 */
void *SLNextItem( SortedListIteratorPtr iterator )
{
	if( iterator == NULL )
	{
		return NULL;
	}
	
	//If this is the first time SLNextItem has been called on the given iterator,
	//then we initially point to the list head, and set the started flag to 1 (i.e. true).
	if( !iterator->started )
	{
		iterator->started = 1;
		iterator->current_node = iterator->list->head;
	}
	
	//Otherwise, we decrement the pointer to our current node, and move on to the next node.
	else
	{
		Node temp = iterator->current_node;
		iterator->current_node->inbound_ptr_count--;
		iterator->current_node = iterator->current_node->next;
		
		//If this iterator was the last persistent object referencing it, then
		//free the node.
		if( temp->inbound_ptr_count == 0 )
		{
			deleteNode(temp);
		}
	}
	
	//If the current node exists, increment its inbound pointer count, and return its object.
	if( iterator->current_node != NULL )
	{
		iterator->current_node->inbound_ptr_count++;
		return iterator->current_node->object;
	}
	
	//Otherwise return NULL as we've reached past the end of the list.
	return NULL;
}

/*
 * Free the given target node.
 * Returns 1 if removed successfully; returns 0 if the given target node is NULL.
 */
int deleteNode( Node target )
{
	if( target == NULL )
	{
		return 0;
	}
	
	//If this node still has reference to its next node, decrement the next node's
	//inbound pointer count.
	if( target->next != NULL ){
		target->next->inbound_ptr_count--;
	}
	
	//If this node still has reference to its prev node, decrement the prev node's
	//inbound pointer count.
	if( target->prev != NULL ){
		target->prev->inbound_ptr_count--;
	}
	
	//Set this node's references to null, free the node, and return success.
	target->next = NULL;
	target->prev = NULL;
	
	free(target);
	
	return 1;
}

/*
 * Find the node wrapping the target object within the given list.
 * Returns a pointer to the node if it exists; NULL otherwise.
 */
Node findNode( SortedListPtr list, void* target )
{
	Node curr = list->head;
	int comparison;
	
	while( curr != NULL ){
		
		comparison = list->function( target, curr->object );
		
		//If target is greater than the current object, then we've already past
		//the point where we would have found the target; therefore we report failure.
		if( comparison > 0 ){
			return NULL;
		}
		
		if( comparison == 0 ){
			return curr;
		}
		
		curr = curr->next;
	}
	
	return NULL;
}

/*
 * Create a new, initialized node.
 * Returns a pointer to the new node.
 */
Node generateNode()
{
	Node new_node = malloc( sizeof( *new_node ) );
	new_node->object = NULL;
	new_node->prev = NULL;
	new_node->next = NULL;
	new_node->inbound_ptr_count = 0;
	
	return new_node;
}

/*
 * Prepare the given node for removal from the associated list.
 * Specifically, realign all pointers s.t. the list no longer has reference to the node.
 * The node may, however, still have reference back into the list.
 */
void primeNodeForRemoval( SortedListPtr list, Node node )
{
	if( node->prev != NULL ){
		node->prev->next = node->next;
		node->inbound_ptr_count--;
		
		if( node->next != NULL ){
			node->next->inbound_ptr_count++;
		}
	}
	
	if( node->next != NULL ){
		node->next->prev = node->prev;
		node->inbound_ptr_count--;
		
		if( node->prev != NULL ){
			node->prev->inbound_ptr_count++;
		}
	}
	
	if( node == list->head ){
		list->head = node->next;
		node->inbound_ptr_count--;
		
		if( node->next != NULL ){
			node->next->inbound_ptr_count++;
		}
	}
}

/**
 * Shift the node containing the target up the list, if its contents have changed
 * such that the list no longer has its correct descending order.
 */
void shiftNodeUp( SortedListPtr list, void* target )
{
	Node target_node = findNode( list, target );
	if( target_node == NULL )
	{
		return;
	}

	if( target_node == list->head )
	{
		return;
	}

	int comparison = list->function( target, target_node->prev->object);

	/* While the target is greater than its preceding neighbor. */
	while( comparison > 0 )
	{
		Node prev = target_node->prev;
		Node next = target_node->next;

		//prev next = t next
		//next prev = t prev

		//t prev = prev prev
		//prev prev next = t
		//prev prev = t
		//t next = prev

		prev->next = target_node->next;
		target_node->inbound_ptr_count--;

		if( next != NULL )
		{
			next->inbound_ptr_count++;

			next->prev = target_node->prev;
			prev->inbound_ptr_count++;
			target_node->inbound_ptr_count--;
		}

		target_node->prev = prev->prev;
		prev->inbound_ptr_count--;

		if( prev->prev != NULL )
		{
			prev->prev->inbound_ptr_count++;

			prev->prev->next = target_node;
			target_node->inbound_ptr_count++;
			prev->inbound_ptr_count--;
		}

		Node pp_temp = prev->prev;

		prev->prev = target_node;
		target_node->inbound_ptr_count++;

		if( pp_temp != NULL )
		{
			pp_temp->inbound_ptr_count--;
		}

		target_node->next = prev;
		prev->inbound_ptr_count++;

		if( next != NULL )
		{
			next->inbound_ptr_count--;
		}

		if( prev == list->head )
		{
			list->head = target_node;
			prev->inbound_ptr_count--;
			target_node->inbound_ptr_count++;
			return;
		}

		comparison = list->function( target, target_node->prev->object);
	}
}
