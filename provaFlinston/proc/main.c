/* file:  CondVarSignal.c 
   Routine che fornisce un synchronization point. 
   E' chiamata da ognuno dei SYNC_MAX pthread, che si fermano 
   finche' tutti gli altri sono arrivati allo stesso punto di esecuzione. 
*/ 

#define _THREAD_SAFE

#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdint.h>
#include <inttypes.h>
#include <pthread.h> 
#include "printerror.h"

#define NUMBER_OF_ISLANDS 2
#define NUMBER_OF_PERSONS 3
#define SHMOBJ_PATH "/flinstonesProblem"


struct dragon {
	int tail;
	int position; /* 1 first island, 2 second island*/
};

struct island {
	int numOfPeople;
};

struct person {
	int position; /*0 on the tail, 1 first island, 2 second island*/
};

/*

ACTIONS:
0 -> dragon is moving

1 -> people are getting off

2 -> people are getting on

*/

struct sharedMemory {
	struct dragon carloVan;
	struct person SaucyJohn[NUMBER_OF_PERSONS];
	int presentAction;
	int pInIslands[NUMBER_OF_ISLANDS];
	pthread_mutex_t mutexDragonTail;
	pthread_mutex_t mutexIslands[NUMBER_OF_ISLANDS];
	pthread_mutex_t mutexJohn[NUMBER_OF_PERSONS];
};

void person(struct sharedMemory* shm);
void dragon(struct sharedMemory* shm);

int main () 
{ 
	pthread_t    th[SYNC_MAX]; 
	int i, rc, shmfd, shared_seg_size = sizeof(struct sharedMemory);
	struct sharedMemory *shared_segment;
	pid_t pid;
	
	/*CREATE SHARED MEMORY*/
	shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
	if (shmfd < 0) {
        perror("shm_open() failed");
        exit(1);
    }
    fprintf(stdout, "Created shared memory object %s\n", SHMOBJ_PATH);
	
	/*RESIZE SHARED MEMORY*/
    rc = ftruncate(shmfd, shared_seg_size);
    if (rc != 0) {
        perror("ftruncate() failed");
        exit(1);
    }

    /*MAKE THE VARIABLE POINT TO THE SHARED MEMORY*/    
    shared_segment = (struct sharedMemory *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shared_segment == MAP_FAILED /* is ((void*)-1) */ ) {
        perror("mmap() failed");
        exit(1);
    }
    fprintf(stdout, "Shared memory segment allocated correctly (%d bytes) at address %p\n", shared_seg_size, shared_segment );

    /*FILL THE SHARED MEMORY*/
    shared_segment -> carloVan.tail = 0;
    shared_segment -> carloVan.position = 1;
    shared_segment -> pInIslands[0]=0;
    shared_segment -> pInIslands[1]=0;
    shared_segment -> SaucyJohn[0].position = 1;
    shared_segment -> SaucyJohn[1].position = 1;
    shared_segment -> SaucyJohn[2].position = 2;
	rc = pthread_mutex_init (&(shared_segment->mutexDragonTail), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */
	rc = pthread_mutex_init (&(shared_segment->mutexIslands[0]), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */
	rc = pthread_mutex_init (&(shared_segment->mutexIslands[1]), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */
	rc = pthread_mutex_init (&(shared_segment->mutexJohn[0]), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */
	rc = pthread_mutex_init (&(shared_segment->mutexJohn[1]), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */
	rc = pthread_mutex_init (&(shared_segment->mutexJohn[2]), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */

	for(i = 0; i<NUMBER_OF_PERSONS; i++){
		pid = fork();
		if(pid == 0){
			person(shared_segment);
			exit(1);
		}
	}

	dragon(shared_segment);

	/*
	I should unmap the memory and delete it,
	but i assume i won't ever reach this code
	*/

	exit(1);
}


void person(struct sharedMemory* shm){
	return;
}

void dragon(struct sharedMemory* shm){
	return;
}















/*

1 processo gestisce il dinosauro -- main process
1 processo per ogni flinston
TOT 4 processi

1)
signal a altri passeggeri che possono scendere

2)
il primo scende
controlla se ne devono scendere altri
	se si: fa la signal
	se no: manda la signal al dinosauro

3)
dinosauro svegliato
sveglia chi deve salire
aspetta che salgano

4)
il passeggero si sveglia
controlla se ci sono gia 2 persone
	se vero: manda la signal al dinosauro
	se falso: sale e sveglia un altro passeggero

5)
dinosauro sleep 2 secondi
sposta coda


*/