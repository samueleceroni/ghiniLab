#define _THREAD_SAFE
#define POSIX_C_SOURCE 200112L
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#define NUM_THREADS 10


void * func( void *arg){
	double indice;
	indice = (double) *(double*)arg;
	sleep(5);
	printf("ho ricevuto %f\n ", indice);
	free(arg);
	pthread_exit(NULL);

}

int main(){
	pthread_t tid;
	int rc;
	int i;
	double *t;
	srand(time(NULL));
	for(i=0;i < NUM_THREADS; i++) {
		t = (double*) malloc (sizeof(double));
		*t = (double)(rand())/1000;
		rc = pthread_create( & tid, NULL, func, (void*) t);
		if( rc !=0) { printf("errore\n"); exit(1); }
	}
	pthread_exit(0);
}