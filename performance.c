#define _POSIX_C_SOURCE 199309L // Notwendig für clock_gettime und CLOCK_MONOTONIC
#include "performance.h"


void performanceFractal(func_ptr algo, float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img, int reruns){
    double resultat = 0;
	unsigned int counter = 0;
	struct timespec starttime;
	struct timespec endtime;
	double time;
    printf("	Start Performance Measurement!\n");
	
	for (int i = 0; i< (reruns+1);i++){
		clock_gettime(CLOCK_MONOTONIC, &starttime);
		algo(start, width, height, res, n, img);
		clock_gettime(CLOCK_MONOTONIC, &endtime);
		time = endtime.tv_sec - starttime.tv_sec + 1e-9 * (endtime.tv_nsec - starttime.tv_nsec);
		resultat += time;
		counter++;
	}
	
	printf("		--> Total duration: %f\n", resultat);
	printf("		--> Total runs: %d\n", counter);
	printf("		--> Average duration: %f\n", resultat/counter);

    printf("	End Performance Measurement!\n");
}
