#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <string.h>
#include "printerror.h"

pthread_mutex_t mutexdata[10];
pthread_t callThd;

void * fachiro (void * arg){
	int rc, i;
	int fachiro = *((int*) arg);
	while(1){
		for(i=0; i<10; i++){
			rc = pthread_mutex_lock (&mutexdata[i]);
			if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		}
		printf("Fachiro %d si è trafitto\n", fachiro);
		/*sleep(0.1);*/
		
		for(i=0; i<10; i++){
			rc = pthread_mutex_unlock (&mutexdata[i]);
			if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		}
	}
}

int main(){
	int rc, i;
	void *ptr;
	int a =1;
	int b = 2;
	
	for(i=0; i<10; i++){
		rc = pthread_mutex_init (&mutexdata[i], NULL);
		if( rc!=0 ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
	}

	rc = pthread_create ( &callThd, NULL, fachiro, (void *) &a);
	if( rc != 0 ) PrintERROR_andExit(rc,"pthread_create failed");
	rc = pthread_create ( &callThd, NULL, fachiro, (void *) &b);
	if( rc != 0 ) PrintERROR_andExit(rc,"pthread_create failed");
	
	rc = pthread_join ( callThd, &ptr);
	if( rc != 0 ) PrintERROR_andExit(rc,"pthread_join failed"); /* no EINTR */

	return 0;
}