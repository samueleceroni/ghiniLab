#include <stdio.h>
#include "dimezza.h"
#include "quadrato.h"

int main(){
	printf("%e", square(halfer(13.17)));
	return 0;
}