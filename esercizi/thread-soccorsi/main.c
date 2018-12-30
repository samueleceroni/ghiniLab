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
pthread_cond_t sync_pro; 
pthread_cond_t sync_doct;
pthread_cond_t sync_eso;
pthread_cond_t sync_main;

int nOp = 0;
int proReady = 0;
int doctReady = 0;
int esoReady = 0;


void * pro( void *arg){
	int rc;
	printf("Pro initialized\n");
	rc = pthread_mutex_lock(&sync_lock);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
	proReady = 1;
	rc = pthread_cond_signal (&sync_main);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");
	printf("Pro waiting\n");
	rc = pthread_cond_wait(&sync_pro, &sync_lock);
	if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");
	printf("Pro svegliato\n");
	rc = pthread_mutex_unlock(&sync_lock);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");

	while(1){
		rc = pthread_mutex_lock(&sync_lock);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");

		rc = pthread_cond_signal (&sync_doct);
		if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");

		rc = pthread_cond_wait(&sync_pro, &sync_lock);
		if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");

		rc = pthread_mutex_unlock (&sync_lock);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
		sleep(4);

	}

}

void * doct(void *arg){
	int rc;
	printf("Doct initialized\n");
	pthread_mutex_lock(&sync_lock);
	doctReady = 1;
	rc = pthread_cond_signal (&sync_main);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");
	while(1){
		/*printf("Doct waiting\n");*/
		rc = pthread_cond_wait(&sync_doct, &sync_lock);
		if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");

		rc = pthread_cond_signal (&sync_eso);
		if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");

		rc = pthread_cond_wait(&sync_doct, &sync_lock);
		if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");

		rc = pthread_cond_signal (&sync_pro);
		if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");
	}
}

void * eso(void *arg){
	int rc;
	printf("Eso initialized\n");
	pthread_mutex_lock(&sync_lock);
	esoReady = 1;
	rc = pthread_cond_signal (&sync_main);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");
	while(1){
		/*printf("Eso waiting\n");*/
		rc = pthread_cond_wait(&sync_eso, &sync_lock);
		if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");
		/* comincia intervento */
		printf("Operazione %d\n", ++nOp);
		sleep(2);
		
		rc = pthread_cond_signal (&sync_doct);
		if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");
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
	int rc;
	printf("Init prob\n");
	rc = pthread_cond_init(&sync_pro, NULL);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_init failed");
	rc = pthread_cond_init(&sync_doct, NULL);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_init failed");
	rc = pthread_cond_init(&sync_eso, NULL);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_init failed");
	rc = pthread_mutex_init(&sync_lock, NULL); 
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_init failed");
	printf("Creating threads\n");

	rc = pthread_create( & tid, NULL, eso, NULL);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}
	rc = pthread_create( & tid, NULL, doct, NULL);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}
	rc = pthread_create( & tid, NULL, pro, NULL);
	if( rc !=0) {
		strerror_r(rc, buffer, 500);
		printf("%s\n", buffer);
		exit(1);
	}
	
	rc = pthread_mutex_lock(&sync_lock); 
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
	while(proReady == 0 || doctReady == 0 || esoReady == 0){
		printf("NotReady:\nPro:%d Doct:%d Eso:%d\n", proReady, doctReady, esoReady);

		rc = pthread_cond_wait(&sync_main, &sync_lock);
		if( rc ) PrintERROR_andExit(rc, "pthread_cond_wait failed");

	}

	printf("Pro signalled\n");
	rc = pthread_cond_signal (&sync_pro);
	if( rc ) PrintERROR_andExit(rc,"pthread_cond_signal failed");
	rc = pthread_mutex_unlock (&sync_lock);
	if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");


	pthread_exit(0);
}