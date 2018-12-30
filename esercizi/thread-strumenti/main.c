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

pthread_mutex_t sync_lock;
pthread_cond_t sync_cond; 
int numLav = 0;
int l1 = 0;
int l2 = 0;

void * func( void *arg){
	int timeToWait = *((int*) arg), rc;
	int myid;
	rc = pthread_mutex_lock(&sync_lock); 
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
	myid = timeToWait == 5 ? l1 : l2;
	timeToWait == 5 ? ++l1 : ++l2;
	rc = pthread_mutex_unlock (&sync_lock);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");

	printf("Lab initialized\n");
	while(1){
		rc = pthread_mutex_lock(&sync_lock); 
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		while(numLav >= 2){
			rc = pthread_cond_wait(&sync_cond, &sync_lock);
			if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");
		}
		printf("Lavorazio %d che occupa %d sec iniziata\n", myid, timeToWait);
		numLav++;

		rc = pthread_mutex_unlock (&sync_lock);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
		sleep(timeToWait);
		rc = pthread_mutex_lock(&sync_lock); 
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		numLav--;

		printf("Lavorazio %d che occupa %d sec finita\n", myid, timeToWait);
		rc = pthread_cond_signal (&sync_cond);
		if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");

		rc = pthread_mutex_unlock (&sync_lock);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
		sleep(1);

	}

}

/*
rc = pthread_cond_wait(&sync_cond, &sync_lock);
if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");

rc = pthread_cond_signal (&sync_cond);
if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");

rc = pthread_mutex_lock(&sync_lock); 
if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");

rc = pthread_mutex_unlock (&sync_lock);
if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");

*/

int main(){
	char buffer[500];
	pthread_t tid;
	int rc, t1 = 5, t2 = 4;
	printf("Init prob\n");
	rc = pthread_cond_init(&sync_cond, NULL);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_init failed");
	rc = pthread_mutex_init(&sync_lock, NULL); 
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_init failed");
	printf("Creating threads\n");

	rc = pthread_create( & tid, NULL, func, (void*)&t1);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}
	rc = pthread_create( & tid, NULL, func, (void*)&t1);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}
	rc = pthread_create( & tid, NULL, func, (void*)&t2);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}
	rc = pthread_create( & tid, NULL, func, (void*)&t2);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}




	pthread_exit(0);
}