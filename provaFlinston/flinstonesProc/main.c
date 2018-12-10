#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdint.h>
#include <inttypes.h>
#include "printerror.h"

#include <sys/mman.h>  /* Memory MANagement: shm_* stuff, and mmap() */
#include <sys/stat.h>
#include <fcntl.h>     /* flags for shm_open */
#include <sys/types.h> /* ftruncate() */

#define NUMBER_OF_ISLANDS 2
#define NUMBER_OF_PERSONS 3
#define SHMOBJ_PATH "/flinstonesProblem"


struct dragon {
	int peopleOnTheTail;
	int position; /*1 first island, 2 second island*/
	pthread_cond_t cond;
};

struct person {
	int position; /*0 on the tail, 1 first island, 2 second island*/
};

/*
ACTIONS:
0 -> dragon is moving
1 -> dragon is arrived
2 -> people are getting off
3 -> people are getting on
*/

struct sharedMemory {
	struct dragon carloVan;
	struct person saucyJohn[NUMBER_OF_PERSONS];
	int actualAction;
	int pInIslands[NUMBER_OF_ISLANDS];
	pthread_mutex_t mutexGeneral;
	pthread_cond_t peopleCond;
};

void person(struct sharedMemory *shm, int id);
void dragon(struct sharedMemory *shm);
void initShm(struct sharedMemory *shm);
void printShm(struct sharedMemory *shm);
void initMutex(pthread_mutex_t *mVar);
void lockMutex(pthread_mutex_t *mVar);
void unlockMutex(pthread_mutex_t *mVar);
void initCond(pthread_cond_t *cVar);
void waitCond(pthread_cond_t *cVar, pthread_mutex_t *mVar);
void signalCond(pthread_cond_t *cVar);
void broadcastCond(pthread_cond_t *cVar);

int main () 
{ 
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
    fprintf(stdout, "Shared memory segment allocated correctly (%d bytes) at address %p\n", shared_seg_size, (void*)shared_segment );

    /*FILL THE SHARED MEMORY*/
    initShm(shared_segment);

    /*CREATE SONS*/
	for(i = 0; i<NUMBER_OF_PERSONS; i++){
		/*fprintf(stdout, "Creating son %d\n", i);*/
		pid = fork();
		if(pid == 0){
			person(shared_segment, i);
			exit(1);
		}
	}
	fprintf(stdout, "Creating dragon\n");
    /*CREATE DRAGON*/
	dragon(shared_segment);
	/*
	I should unmap the memory and delete it,
	but i assume i won't ever reach this code
	*/
	exit(1);
}

void initShm(struct sharedMemory *shm){
	    shm -> actualAction = 1;
	    shm -> carloVan.peopleOnTheTail = 0;
	    shm -> carloVan.position = 1;
	    shm -> pInIslands[0] = 2;
	    shm -> pInIslands[1] = 1;
	    shm -> saucyJohn[0].position = 1;
	    shm -> saucyJohn[1].position = 1;
	    shm -> saucyJohn[2].position = 2;
	    initMutex(&(shm -> mutexGeneral));
	    initCond(&(shm -> carloVan.cond));
	    initCond(&(shm -> peopleCond));
}

void person(struct sharedMemory* shm, int id){
	int actualAction;
	/*printShm(shm);*/
	/*sleep(10);*/

	while(1){
		lockMutex(&(shm -> mutexGeneral));
		fprintf(stdout, "Person %d waiting and releasing mutex\n", id);
		waitCond(&(shm -> peopleCond), &(shm -> mutexGeneral));
		fprintf(stdout, "Person %d reaches not reacheable\n", id);
		/*if i am in the other island or not in the tail i do nothing*/
		if(shm -> carloVan.position != shm -> saucyJohn[id].position
			&& shm -> saucyJohn[id].position != 0){
			unlockMutex(&(shm -> mutexGeneral));
			break;
		}
		actualAction = shm -> actualAction;
		switch(actualAction){
			case 1:/*DRAGON ARRIVED*/
			case 2:/*PEOPLE GETTING OFF*/
				/*I get off*/
				if(shm -> saucyJohn[id].position != 0){break;} /*i have not to get off*/
				shm -> actualAction = 2;
				shm -> saucyJohn[id].position = shm -> carloVan.position;
				shm -> carloVan.peopleOnTheTail--;
				shm -> pInIslands[shm -> saucyJohn[id].position - 1]++;
				if(shm -> carloVan. peopleOnTheTail <= 0){
					/*If i am the last, just change the actualAction*/
					shm -> actualAction = 3;
				}
				signalCond(&(shm -> peopleCond));
				unlockMutex(&(shm -> mutexGeneral));
				sleep(4);
				break;
			case 3:/*PEOPLE ARE GETTING ON THE TAIL*/
				if(shm -> carloVan.position != shm -> saucyJohn[id].position){break;}/*i have not to get on the tail*/
				/*get on the tail*/
				shm -> saucyJohn[id].position = 0;
				shm -> carloVan.peopleOnTheTail++;
				shm -> pInIslands[shm -> carloVan.position - 1]--;
				if(shm -> carloVan.peopleOnTheTail >= 2){
					/*tail full of flinstones*/
					shm -> actualAction = 0; /* dragon is now going to move */
					signalCond(&(shm -> carloVan.cond));
				} else {
					/*other people can get on the tail, so i awake them*/
					signalCond(&(shm -> peopleCond));
				}
				unlockMutex(&(shm -> mutexGeneral));
				break;
		}
	}
	return;
}

void dragon(struct sharedMemory* shm){
	sleep(1);
	fprintf(stdout, "Dragon created\n");
	lockMutex(&(shm -> mutexGeneral));
	fprintf(stdout, "Dragon locked the mutex\n");
	/*printShm(shm);*/
	fprintf(stdout, "Dragon signals people\n");
	signalCond(&(shm -> peopleCond));	/*FLINSTONES CAN GET DOWN*/
	while(1){
		do{
			fprintf(stdout, "Dragon waits and unlocks mutex\n");
			waitCond(&(shm -> carloVan.cond), &(shm -> mutexGeneral));	/*THE DRAGON WAIT FOR THE OTHERS TO GET OFF*/
			fprintf(stdout, "Dragon gets back the mutex and cond\n");
		} while(shm -> actualAction != 0);
		sleep(2);
		shm -> actualAction = 1; /*dragon arrived*/
		if(shm -> carloVan.position == 1){
			shm -> carloVan.position = 2;
		} else {
			shm -> carloVan.position = 1;
		}
		signalCond(&(shm -> peopleCond));
		unlockMutex(&(shm -> mutexGeneral));
	}
	return;
}


void initMutex(pthread_mutex_t *mVar){
	pthread_mutexattr_t mattr;

	int rc;
	rc=pthread_mutexattr_init(&mattr);
    if( rc ) PrintERROR_andExit(rc,"pthread_mutexattr_init  failed");
    rc=pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED);
    if( rc ) PrintERROR_andExit(rc,"pthread_mutexattr_setpshared  failed");
	rc = pthread_mutex_init (mVar, &mattr);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */
}

void lockMutex(pthread_mutex_t *mVar){
	int rc;
	rc = pthread_mutex_lock (mVar);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
}

void unlockMutex(pthread_mutex_t *mVar){
	int rc;
	rc = pthread_mutex_unlock (mVar);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
}

void initCond(pthread_cond_t *cVar){
	pthread_condattr_t cvattr;
	int rc;
	rc=pthread_condattr_init(&cvattr);
    if( rc ) PrintERROR_andExit(rc,"pthread_condattr_init  failed");
    rc=pthread_condattr_setpshared(&cvattr,PTHREAD_PROCESS_SHARED);
    if( rc ) PrintERROR_andExit(rc,"pthread_condattr_setpshared  failed");
	rc = pthread_cond_init( cVar, &cvattr );
	if(rc) PrintERROR_andExit(rc,"pthread_cond_init failed");
}

void waitCond(pthread_cond_t *cVar, pthread_mutex_t *mVar){
	int rc;
	rc = pthread_cond_wait(cVar, mVar);
	if(rc) PrintERROR_andExit(rc, "pthread_cond_wait failed");
}

void signalCond(pthread_cond_t *cVar){
	int rc;	
	rc = pthread_cond_signal(cVar);
	if(rc) PrintERROR_andExit(rc,"pthread_cond_signal failed");
}

void broadcastCond(pthread_cond_t *cVar){
	int rc;
	rc = pthread_cond_broadcast(cVar);
	if(rc) PrintERROR_andExit(rc,"pthread_cond_broadcast failed");
}


void printShm(struct sharedMemory *shm){
	fprintf(stdout, "Situa explained:\nIsland1:%d\nIsland2:%d\nDragonPos:%d\nTail:%d\n",
	shm -> pInIslands[0], shm -> pInIslands[1], shm -> carloVan.position, shm ->carloVan.peopleOnTheTail);
}

/*

1 processo gestisce il dinosauro -- main process
1 processo per ogni flinston
TOT 4 processi

1)
drago fa signal a altri passeggeri che possono scendere

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
/*
ciao,

la soluzione al problema è semplice.
Nella inizializzazione delle strutture
pthread_mutex_t e pthread_cond_t
effettuata mediante le chiamate alle
funzioni pthread_mutex_init e pthread_cond_t
ti sei dimenticato di inizializzare affinché
possano lavorare con i processi invece che
solo con i pthread.


vedi il codice nell'esempio:


     rc=pthread_mutexattr_init(&mattr);
     if( rc ) PrintERROR_andExit(rc,"pthread_mutexattr_init  failed");
     rc=pthread_mutexattr_setpshared(&mattr,PTHREAD_PROCESS_SHARED);
     if( rc ) PrintERROR_andExit(rc,"pthread_mutexattr_setpshared  failed");
     rc=pthread_mutex_init(&buffer->mutex, &mattr);
     if( rc ) PrintERROR_andExit(rc,"pthread_mutex_init  failed");



     rc=pthread_condattr_init(&cvattr);
     if( rc ) PrintERROR_andExit(rc,"pthread_condattr_init  failed");
     rc=pthread_condattr_setpshared(&cvattr,PTHREAD_PROCESS_SHARED);
     if( rc ) PrintERROR_andExit(rc,"pthread_condattr_setpshared  failed");
     rc=pthread_cond_init(&buffer->cond_empty, &cvattr);
     if( rc ) PrintERROR_andExit(rc,"pthread_cond_init  failed");

tu invece metti a NULL il secondoo argomento di
thread_cond_init e pthread_mutex_init.

ciao
    v.ghini*/