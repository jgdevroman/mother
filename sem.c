/*
 * Semaphore implementation for the synchronization of POSIX threads. 
 * 
 * This module implements counting P/V semaphores suitable for the 
 * synchronization of POSIX threads. POSIX mutexes and condition 
 * variables are utilized to implement the semaphor operations. 
 */
#include <pthread.h>
#include <errno.h>
#include <stdlib.h>

#include "sem.h"

/*
 * @brief Opaque type of a semaphore. 
 */
struct SEM{
	pthread_mutex_t m;
	pthread_cond_t c;
	volatile int value;
};

/*
 * @brief Creates a new semaphore. 
 * 
 * This function creates a new semaphore. If an error occurs during the 
 * initialization, the implementation frees all resources already 
 * allocated by then and sets errno to an appropriate value.
 * 
 * It is legal to initialize the semaphore with a negative value. 
 * If this is the case, in order to reset the semaphore counter to zero,
 *  the V-operation must be performed (-initVal) times.
 * 
 * @parameter initVal	The initial value of the semaphore. 
 * @return 	Handle for the created semaphore, or NULL if an error 
 * 			occurred. 
 */
SEM *semCreate(int initVal){
	SEM *sem = (SEM *) malloc(sizeof(SEM));
	if (sem == NULL){
		return NULL;
	}
	
	errno = pthread_mutex_init(&sem->m, NULL);
	if(errno != 0){
		free(sem);
		return NULL;
	}
	
	errno = pthread_cond_init(&sem->c, NULL);
	if(errno != 0){
		pthread_mutex_destroy(&sem->m);
		free(sem);
		return NULL;
	}
	
	sem->value = initVal;
	
	return sem;
}

/*
 * @brief Destroys a semaphore and frees all associated resources. 
 * 
 * @parameter sem	Handle of the semaphore to destroy. If a NULL 
 * 					pointer is passed, the implementation does nothing.
 */

void semDestroy(SEM *sem){
	if(sem == NULL)
		return;
		
	errno = pthread_cond_destroy(&sem->c);
	if(errno != 0) 
		return;
	errno = pthread_mutex_destroy(&sem->m);
	if(errno != 0) 
		return;
	
	free(sem);
	
	return;
} 


/*
 * @brief P-operation. 
 * 
 * Attempts to decrement the semaphore value by 1. If the semaphore 
 * value is not a positive number, the operation blocks until a 
 * V-operation increments the value and the P-operation succeeds.
 * 
 * @parameter sem	Handle of the semaphore to decrement. 
 */
 
void P(SEM *sem){
	pthread_mutex_lock(&sem->m);
	
	while(sem->value < 1){
		pthread_cond_wait(&sem->c, &sem->m);
	}
	sem->value = sem->value - 1;
	pthread_mutex_unlock(&sem->m);
	
	return;
}
 
/*
 * @brief V-operation. 
 * 
 * Increments the semaphore value by 1 and notifies P-operations that 
 * are blocked on the semaphore of the change.
 * 
 * @parameter sem	Handle of the semaphore to increment. 
 * 
 */
 
void V(SEM *sem){
	
	pthread_mutex_lock(&sem->m);
	sem->value++;
	pthread_cond_broadcast(&sem->c);
	pthread_mutex_unlock(&sem->m);
		
	return;
}









