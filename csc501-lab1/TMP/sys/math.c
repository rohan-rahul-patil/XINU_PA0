#include<kernel.h>
#include<stdio.h>

double pow(double x, int y) {
	int i;
	double p = 1.0;
	for(i = 0; i < y; i++) {
		p = p * x;
	}
	return p;
}
	
double log(double x) {
	if(x == 1) 
		return MAXINT - 1;
	int i,  n = 20;
	double y = 0.0;

	for( i = 1; i <= n; i++) {
		y = y + (pow(x, i) / (double)i);
	}
	return -1.0 * y;
}

double expdev(double lambda) {
    double dummy;
    do
        dummy= (double) rand() / RAND_MAX;
    while (dummy == 0.0);
    return (int)(-log(dummy) / lambda);
}
