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

struct paramBanca{
	int staDepositando;
	int numeroBanca;
};

unsigned long depositato[3] = {0};
unsigned long nOp[3] = {0};
pthread_mutex_t mutexdata;
pthread_t callThd;

void * azioniBanca(void * arg){
	struct paramBanca task = *((struct paramBanca*)(arg));
	int rc;
	while(1){
		sleep(1);
		/*
		 * CRITIC ZONE
		*/
		rc = pthread_mutex_lock (&mutexdata);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");

		nOp[task.numeroBanca]++;

		if (task.staDepositando == 1){
			/*deposita 10*/
			depositato[task.numeroBanca] += 10;
		} else {
			/*preleva 9*/
			depositato[task.numeroBanca] -= 9;
		}
		
		/*sleep(0.1);*/
		nanosleep(10000);
		/*
		 * END OF CRITIC ZONE
		*/
		rc = pthread_mutex_unlock (&mutexdata);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
	}

}

void * bancaDiItalia (void * arg){
	int rc;
	while(1){
		rc = pthread_mutex_lock (&mutexdata);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_lock failed");
		printf("Totale: %lu\nOperazioni: %lu\n",
			   depositato[0] + depositato[1] + depositato[2],
			   nOp[0] + nOp[1] + nOp[2]);
		sleep(1);
		rc = pthread_mutex_unlock (&mutexdata);
		if( rc ) PrintERROR_andExit(rc,"pthread_mutex_unlock failed");
		sleep(5);
	}
}

int main(){
	int numeroBanche = 3, numeroDepositi = 5, numeroPrelievi = 4;
	int i, j;
	int rc;
	void *ptr;
	rc = pthread_mutex_init (&mutexdata, NULL);
	if( rc != 0 ) PrintERROR_andExit(errno,"pthread_mutex_init failed");

	for (i = 0; i<numeroBanche; i++){

		struct paramBanca taskDep = {1, i};
		struct paramBanca taskPrev = {0, i};
		/*
		taskDep = {1, i};
		taskDep.staDepositando = 1;
		taskDep.numeroBanca = i;
		*/
		for(j = 0; j<numeroDepositi; j++){
			/*lancia thread con valore true*/
			rc = pthread_create ( &callThd, NULL, azioniBanca, (void *) &taskDep);
			if( rc != 0 ) PrintERROR_andExit(rc,"pthread_create failed");
		}

		/*
		taskPrev = {0, i};
		taskPrev.staDepositando = 0;
		taskPrev.numeroBanca = i;
		*/

		for(j = 0; j<numeroPrelievi; j++){
			/*lancia thread con valore false*/
			rc = pthread_create ( &callThd, NULL, azioniBanca, (void *) &taskPrev);
			if( rc != 0 ) PrintERROR_andExit(rc,"pthread_create failed");
		}
	}
	
	rc = pthread_create ( &callThd, NULL, bancaDiItalia, (void *) NULL);
	if( rc != 0 ) PrintERROR_andExit(rc,"pthread_create failed");
	
	rc = pthread_join ( callThd, &ptr);
	if( rc != 0 ) PrintERROR_andExit(rc,"pthread_join failed"); /* no EINTR */

	return 0;
}