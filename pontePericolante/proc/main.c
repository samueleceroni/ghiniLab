#include <unistd.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "printerror.h"

#include <sys/mman.h>  /* Memory MANagement: shm_* stuff, and mmap() */
#include <sys/stat.h>
#include <fcntl.h>     /* flags for shm_open */
#include <sys/types.h> /* ftruncate() */

#define SHMOBJ_PATH "/foo1234"

#define CARS_CLOCKWISE 4
#define CARS_ANTICLOCKWISE 4
#define CLOCKWISE 1
#define ANTICLOCKWISE 0

struct sharedMemory {
    /*  declare shared variables    */
    int autoSulPonte;
    bool autoOltreMeta;
    bool versoAutoSulPonte;
    int autoInAttesa[2];
    int proxBiglietto[2];
    int turno[2];
    pthread_mutex_t mPonte;
    pthread_cond_t cAutomobile[2];
};

void initShm(struct sharedMemory *shm){
    /*  initialize the shared memory    */
    int rc, shmfd, shared_seg_size = sizeof(struct sharedMemory);

    shmfd = shm_open(SHMOBJ_PATH, O_CREAT | O_EXCL | O_RDWR, S_IRWXU | S_IRWXG);
        if (shmfd < 0) {
        perror("shm_open() failed");
        exit(1);
    }
    //fprintf(stdout, "Created shared memory object %s\n", SHMOBJ_PATH);
    rc = ftruncate(shmfd, shared_seg_size);
    if (rc != 0) {
        perror("ftruncate() failed");
        exit(1);
    }
    shm = (struct sharedMemory *)mmap(NULL, shared_seg_size, PROT_READ | PROT_WRITE, MAP_SHARED, shmfd, 0);
    if (shared_segment == MAP_FAILED /* is ((void*)-1) */ ) {
        perror("mmap() failed");
        exit(1);
    }
    fprintf(stdout, "Shared memory segment allocated correctly (%d bytes) at address %p\n", shared_seg_size, (void*)shared_segment );

    /*  initialize the shm variables    */
    shm -> autoSulPonte = 0;
    shm -> autoOltreMeta = 0;
    shm -> versoAutoSulPonte = CLOCKWISE;
    shm -> autoInAttesa[0] = 0;
    shm -> autoInAttesa[1] = 0;
    shm -> proxBiglietto[0] = 1;
    shm -> proxBiglietto[1] = 1;
    shm -> turno[0] = 1;
    shm -> turno[1] = 1;
    initMutex(&(shm -> mPonte));
    initCond(&(shm -> cAutomobile[0]));
    initCond(&(shm -> cAutomobile[1]));
}

void procAuto(struct sharedMemory* shm, bool mioVerso){
    /*  initialize variables of the proc    */
    int mioTurno;
    lockMutex(&(shm-> mPonte));
        mioTurno = shm -> proxBiglietto[mioVerso];
        shm->proxBiglietto[mioVerso]++;
    /* aspetto che tutte le auto si inizializzino. verro svegliato dal main */
    waitCond(&(shm -> cAutomobile[mioVerso]), &(shm -> mPonte));
    unlockMutex(&(shm -> mPonte));
    while(1){
        lockMutex(&(shm -> cAutomobile[mioVerso]));
        while(mioTurno != shm -> turno[mioVerso]
                ||
                shm -> autoSulPonte > 0 && shm -> versoAutoSulPonte != mioVerso
                ||
                autoInAttesa[mioVerso] < autoInAttesa[!mioVerso]
             ){
                waitCond(&(shm -> cAutomobile[mioVerso]), &(shm -> mPonte));

        }
    }

    return;
}

void mainProcess(struct sharedMemory* shm){
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

    /*INITIALIZE AND FILL THE SHARED MEMORY*/
    initShm(shared_segment);

    /*CREATE SONS*/
    for(i = 0; i < CARS_CLOCKWISE; i++){
        /*fprintf(stdout, "Creating son %d\n", i);*/
        pid = fork();
        if(pid == 0){
            procAuto(shared_segment, 1); // parono 4 auto in senso orario
            exit(1);
        }
    }
    for(i = 0; i < CARS_ANTICLOCKWISE; i++){
        /*fprintf(stdout, "Creating son %d\n", i);*/
        pid = fork();
        if(pid == 0){
            procAuto(shared_segment, 0); // parono 4 auto in senso antiorario
            exit(1);
        }
    }
    fprintf(stdout, "Creating main process\n");
    /*CREATE MAIN PROCESS*/
    mainProcess(shared_segment);
    /*
    I should unmap the memory and delete it,
    but i assume i won't ever reach this code
    */
    exit(1);
}
