#include <stdio.h>
#include <stdlib.h>
#include "SyncDetector.h"

int main()
{
	int a = 101;
	int b = 100;
	a = Disable;
	b = INT_MIN;
	//printf("a:%d,max:%d\t%d", a, b, LimitValue(a, b));
	printf("\na=%d\tb=%d", a,b);
	return 0;
}