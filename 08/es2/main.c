#define _THREAD_SAFE
#define _POSIX_C_SOURCE 200112L
#define _REENTRANT
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>

#define NUM_THREADS 10


void * func( void *arg){
	printf("%"  PRIiPTR "\n", (intptr_t) arg );
	pthread_exit(NULL);

}

int main(){
	char buffer[500];
	pthread_t tid[1000];
	int rc,j;
	intptr_t i;
	while(1){
		for(i=0;i<1000; i++) {
			rc = pthread_create( & tid[i], NULL, func, (void*)i);
			if( rc !=0) {
				strerror_r(rc, buffer, 500);
				printf("%s\n", buffer);
				exit(1);
			}
		}

		for(j=0;j<1000;j++){
			pthread_join(tid[j],NULL);

		}

	}

	pthread_exit(0);
}