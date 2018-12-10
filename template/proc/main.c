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

#define SHMOBJ_PATH "/foo1234"

struct sharedMemory {
    /*  declare shared variables    */
};

void initShm(struct sharedMemory *shm){
    /*  initialize the shm variables    */
    /*  initialize the shared memory    */
}

void procIndex(struct sharedMemory* shm, int id){
    /*  initialize variables of the proc    */
    /*  get aligned with other procs    */
    /*  start the infinite loop */
    while(1){}
    return;
}

void proc0(struct sharedMemory* shm){
    /*  initialize variables of the proc    */
    /*  get aligned with other procs    */
    /*  start the infinite loop */
    while(1){}
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
    fprintf(stdout, "Shared Memory Content:");
}

int main () { 
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
