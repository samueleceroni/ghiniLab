#include <stdio.h>
#include <stdlib.h>

int main(){
	int *punt, i;
	ALLOCAT(punt);
	for(i=0; i<10; i++){
		printf("%d ", punt[i]);
	}

	for(i=0;i<10;i++){
		punt[i]= i - 19;
	}
	return 0;
}