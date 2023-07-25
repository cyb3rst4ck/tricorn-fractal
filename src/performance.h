#include <stdio.h>
#include <time.h>
#include <complex.h>

// Function prototype collection so that the compiler knows how to call the respective functions with their arguments
typedef void (*func_ptr)(complex float,  size_t,  size_t,  float,  unsigned int,  unsigned char *);
void performanceFractal(func_ptr algo, float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img, int reruns);
