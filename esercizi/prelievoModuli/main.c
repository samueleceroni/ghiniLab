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

int n, m;

void * func( void *arg){
	/*int timeToWait = *((int*) arg);*/
	int rc;
	rc = pthread_mutex_lock(&sync_lock); 
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
	/* initial setup*/
	rc = pthread_mutex_unlock (&sync_lock);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");

	printf("thread initialized\n");
	while(1){
		rc = pthread_mutex_lock(&sync_lock); 
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		/*roba in mutua esclusione*/
		rc = pthread_mutex_unlock (&sync_lock);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
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
	int rc, t1;
	printf("Init prob\n");
	/* init variables*/
	n = 25;
	m = 10;
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

	pthread_exit(0);
}