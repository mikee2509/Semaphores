#ifndef __semaphore_h
#define __semaphore_h

#include <stdio.h> 
#include <stdlib.h> 


#include <sys/types.h> 
#include <sys/stat.h> 
#include <string.h> 
#include <errno.h> 
#include <fcntl.h> 
#include <pthread.h> 
#include <unistd.h>
#include <semaphore.h>

class Semaphore
{
private:
	sem_t sem;
public:
	Semaphore( int value ) {
		if( sem_init( & sem, 0, value ) != 0 )
			throw "sem_init: failed";
	}
	~Semaphore() { 
		sem_destroy( & sem ); 
	}

	void p() {
		if( sem_wait( & sem ) != 0 )
			throw "sem_wait: failed";
	}

	void v() {
		if( sem_post( & sem ) != 0 )
			throw "sem_post: failed";
	}
};

class Mutex
{
private:
	pthread_mutex_t sem = PTHREAD_MUTEX_INITIALIZER;
public:
	Mutex() {
		//pthread_mutex_init(&sem, value);
	}

	~Mutex() { 
		pthread_mutex_destroy(&sem);
	}

	void p() {
		pthread_mutex_lock(&sem);
	}

	void try_p() {
		pthread_mutex_trylock(&sem);
	}

	void v() {
		pthread_mutex_unlock(&sem);
	}
};

#endif //__semaphore_h

