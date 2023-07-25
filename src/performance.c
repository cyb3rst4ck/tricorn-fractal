#define _POSIX_C_SOURCE 199309L // Necessary for clock_gettime and CLOCK_MONOTONIC
#include "performance.h"


// Function to perform the performance analysis
void performanceFractal(func_ptr algo, float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img, int reruns){
    double resultat = 0; // used to sum up the results of each loop iteration
	unsigned int counter = 0; 
	struct timespec starttime;
	struct timespec endtime;
	double time;
    printf("> Performance Analysis: Start Measurement\n");
	
	for (int i = 0; i< (reruns);i++){
		clock_gettime(CLOCK_MONOTONIC, &starttime);
		algo(start, width, height, res, n, img);
		clock_gettime(CLOCK_MONOTONIC, &endtime);
		time = endtime.tv_sec - starttime.tv_sec + 1e-9 * (endtime.tv_nsec - starttime.tv_nsec);
		resultat += time;
		counter++;
	}
	
	printf("\t>> Total duration: %f s\n", resultat);
	printf("\t>> Total runs: %d\n", counter);
	printf("\t>> Average duration per run: %f s\n", resultat/counter);

    printf("> Performance Analysis: End Measurement\n");
}
