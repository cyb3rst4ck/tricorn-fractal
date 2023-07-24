#include <stdio.h>
#include <time.h>
#include <complex.h>

typedef void (*func_ptr)(complex float,  size_t,  size_t,  float,  unsigned int,  unsigned char *);
void performanceFractal(func_ptr algo, float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img, int reruns);