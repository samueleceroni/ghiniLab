#include <stdlib.h>
#define ALLOCAT(x)\
		do{\
			int i;\
			x = (int*) malloc (10*sizeof(int));\
			if(x!=NULL){\
				for(i=0;i<10;i++){\
					x[i] = i-1000;\
				}\
			}\
		} while (0)
