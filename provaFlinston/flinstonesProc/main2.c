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
};

void person(struct sharedMemory* shm, int id);
void dragon(struct sharedMemory* shm);

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
    shared_segment -> actualAction = 1;
    shared_segment -> carloVan.peopleOnTheTail = 0;
    shared_segment -> carloVan.position = 1;
    shared_segment -> pInIslands[0] = 2;
    shared_segment -> pInIslands[1] = 1;
    shared_segment -> saucyJohn[0].position = 1;
    shared_segment -> saucyJohn[1].position = 1;
    shared_segment -> saucyJohn[2].position = 2;

	rc = pthread_mutex_init (&(shared_segment->mutexGeneral), NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed"); /* no EINTR */

	for(i = 0; i<NUMBER_OF_PERSONS; i++){
		fprintf(stdout, "Creating son %d\n", i);
		pid = fork();
		if(pid == 0){
			person(shared_segment, i);
			exit(1);
		}
	}
	fprintf(stdout, "Creating dragon\n");
	dragon(shared_segment);
	/*
	I should unmap the memory and delete it,
	but i assume i won't ever reach this code
	*/
	exit(1);
}


void person(struct sharedMemory* shm, int id){
	int rc, actualAction, iHaveToSleep = 0, count = 0;
	fprintf(stdout, "Person %d created\n", id);
	fprintf(stdout, "Situa explained:\nIsland1:%d\nIsland2:%d\nDragonPos:%d\nTail:%d\n",
	shm -> pInIslands[0], shm -> pInIslands[1], shm -> carloVan.position, shm ->carloVan.peopleOnTheTail);

	while(1){
		/*UNDERSTAND WHAT IS GOING ON*/
		rc = pthread_mutex_lock (&(shm -> mutexGeneral));
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		count ++;
		if(count%100 == 0){fprintf(stdout, "%d:Person %d locked the mutex\n", count, id);}
		actualAction = shm ->actualAction;
		switch(actualAction){
			case 0:	/*IF DRAGON IS MOVING NOBODY ELSE CAN DO ANYTHING*/
				break;
			case 1:	/*IF DRAGON IS ARRIVED*/
					if(shm -> carloVan.peopleOnTheTail <= 0){	/*CHECK IF I WAS THE LAST ON THE ISLAND*/
						shm -> actualAction = 3;	/*NOW PEOPLE CAN GO ON THE TAIL OF THE DRAGON*/
						break;
					}
			case 2:	/*IF PEOPLE ARE GETTING DOWN FROM THE TAIL*/
				if(shm -> saucyJohn[id].position == 0){	/*CHECK IF I AM IN THE TAIL OF THE DRAGON*/
					shm -> actualAction = 2;	/*NOW PEOPLE CAN GET DOWN FROM THE TAIL OF THE DRAGON*/
					shm -> saucyJohn[id].position = shm -> carloVan.position;	/*CHANGE MY POSITION*/
					shm -> carloVan.peopleOnTheTail--;	/*DECREMENT THE AMOUNT OF PEOPLE ON THE TAIL*/
					shm -> pInIslands[shm -> saucyJohn[id].position - 1]++;	/*INCREMENT THE PEOPLE IN THE CURRENT ISLAND*/
					if(shm -> carloVan.peopleOnTheTail <= 0){	/*CHECK IF I WAS THE LAST ON THE ISLAND*/
						shm -> actualAction = 3;	/*NOW PEOPLE CAN GO ON THE TAIL OF THE DRAGON*/
					}
					/*BROADCAST ANOTHER FLINSTON WHICH CAN GET DOWN OR GET ON*/
					iHaveToSleep = 1;
				}
				break;
			case 3:	/*IF PEOPLE ARE GETTING ON THE TAIL*/
				if(shm -> saucyJohn[id].position == shm -> carloVan.position){	/*CHECK IF I AM IN THE SAME ISLAND OF THE DRAGON*/
					shm -> saucyJohn[id].position = 0;	/*GET ON THE TAIL*/
					shm -> carloVan.peopleOnTheTail++;	/*INCREMENT THE AMOUNT OF PEOPLE ON THE TAIL*/
					shm -> pInIslands[shm -> carloVan.position - 1]--;	/*DECREMENT THE PEOPLE IN THE CURRENT ISLAND*/
					if(shm -> carloVan.peopleOnTheTail >= 2){	/*CHECK IF THE TAIL IS FULL*/
						shm -> actualAction = 0;	/*NOW THE DRAGON CAN LEAVE THE ISLAND*/
						/*SIGNAL THE DRAGON WHICH CAN LEAVE THE ISLAND*/
					}
				}
				break;
		}
		rc = pthread_mutex_unlock (&(shm -> mutexGeneral));
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
		if(iHaveToSleep){
			sleep(4);
			iHaveToSleep = 0;
		}
	}
	return;
}

void dragon(struct sharedMemory* shm){
	int rc, actualAction, count;
	fprintf(stdout, "Situa explained:\nIsland1:%d\nIsland2:%d\nDragonPos:%d\nTail:%d\n",
	shm -> pInIslands[0], shm -> pInIslands[1], shm -> carloVan.position, shm ->carloVan.peopleOnTheTail);

	while(1){
		/*UNDERSTAND WHAT IS GOING ON*/
		rc = pthread_mutex_lock (&(shm -> mutexGeneral));
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		count++;
		if(count%100 == 0){fprintf(stdout, "%d:Dragon locked the mutex\n", count);}
		actualAction = shm -> actualAction;
		switch(actualAction){
			case 0:	/*IF DRAGON IS MOVING NOBODY ELSE CAN DO ANYTHING*/
				sleep(1);/*TAKE THE TIME NEEDED TO GET TO THE OTHER ISLAND*/
				shm -> actualAction = 1;
				if(shm -> carloVan.position == 1){
					shm -> carloVan.position = 2;
				} else {
					shm -> carloVan.position = 1;
				}
				fprintf(stdout, "Situa explained:\nIsland1:%d\nIsland2:%d\nDragonPos:%d\nTail:%d\n",
	    		shm -> pInIslands[0], shm -> pInIslands[1], shm -> carloVan.position, shm ->carloVan.peopleOnTheTail);
	    		/*SIGNAL A PERSON WHO CAN GET OFF*/
				break;
			case 1:	/*IF DRAGON IS ARRIVED*/
				break;	/*NOTHING TODO*/
			case 2:	/*IF PEOPLE ARE GETTING DOWN FROM THE TAIL*/
				break;	/*NOTHING TODO*/
			case 3:	/*IF PEOPLE ARE GETTING ON THE TAIL*/
				break;	/*NOTHING TODO*/
		}
		rc = pthread_mutex_unlock (&(shm -> mutexGeneral));
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
		sleep(0.1);
	}
	return;
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