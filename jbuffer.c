/*
 * Bounded-buffer implementation to manage integer values, supporting 
 * multiple readers but only a single writer.
 * The bbuffer module uses the sem module API to synchronize concurrent 
 * access of readers and writers to the bounded buffer.
 */ 

#include <stdlib.h>
#include <limits.h>


#include "sem.h"
#include "jbuffer.h"

/*
 * @brief Opaque type of a bounded buffer. 
 */
struct BNDBUF{
	int *buffer;
	int size;
	int start;
	int end;
	SEM *readsem;
	SEM *writesem;

};
	
/*
 * @brief Creates a new bounded buffer. 
 * 
 * This function creates a new bounded buffer and all the required 
 * helper data structures, including semaphores for synchronization. 
 * If an error occurs during the initialization, the implementation 
 * frees all resources already allocated by then.
 * 
 * @parameter size	The number of integers that can be stored in the 
 * 					bounded buffer. 
 * @return 	Handle for the created bounded buffer, or NULL if an error 
 * 			occurred. 
 */
BNDBUF *bbCreate(size_t size){
	if(size > INT_MAX){
		return NULL;
	}
	
	BNDBUF *bb = (BNDBUF *) malloc(sizeof(BNDBUF));
	if (bb == NULL){
		return NULL;
	}
	
	bb->buffer = (int *) malloc((int) size);
	if (bb->buffer == NULL){
		free(bb);
		return NULL;
	}
	
	//bb->size = buffer[size] + sizeof(int *);
	bb->size = (int) size;
	
	bb->start = 0;
	bb->end = 0;
	
	bb->readsem = semCreate(0);
	if(bb->readsem == NULL){
		free(bb->buffer);
		free(bb);
		return NULL;
	}
	
	bb->writesem = semCreate((int) size);
	if(bb->writesem == NULL){
		free(bb->buffer);
		free(bb);
		return NULL;
	}
	
	
	return bb;
}

/*
 * @brief Destroys a bounded buffer. 
 * 
 * All resources associated with the bounded buffer are released.
 * 
 * @parameter bb	Handle of the bounded buffer that shall be freed. 
 * If a NULL pointer is passed, the implementation does nothing. 
 *
 */
void bbDestroy(BNDBUF *bb){
	if(bb == NULL)
		return;
	
	semDestroy(bb->writesem);
	semDestroy(bb->readsem);
	free(bb->buffer);
	free(bb);
	return;
}

/*
 * @brief Adds an element to a bounded buffer. 
 * 
 * This function adds an element to a bounded buffer. If the buffer is
 * full, the function blocks until an element has been removed from it.
 * 
 * @parameter bb	Handle of the bounded buffer. 
 * @parameter value	Value that shall be added to the buffer. 
 *
 */
void bbPut(BNDBUF *bb, int value){
	
	P(bb->writesem);
	bb->buffer[bb->end] = value;
	bb->end = (bb->end + 1) % bb->size;
	V(bb->readsem);

	return;
}
/*
 * @brief Retrieves an element from a bounded buffer. 
 * 
 * This function removes an element from a bounded buffer. If the buffer
 * is empty, the function blocks until an element has been added.
 * 
 * @parameter bb	Handle of the bounded buffer. 
 * @return int 		The integer element. 
 * 
 */
int bbGet(BNDBUF *bb){
	
	int ret;
	int startold;
	int startnew;
		
	P(bb->readsem);
	do{
		startold = bb->start;
		startnew = (startold + 1) % bb->size;
		ret = bb->buffer[startold];
	}while(
	__sync_bool_compare_and_swap (&(bb->start), startold, startnew) == 0);
	
	V(bb->writesem);
	
	
	return ret;
}
