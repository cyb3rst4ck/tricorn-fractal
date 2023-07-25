#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <complex.h>
#include <stdlib.h>
#include <immintrin.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Function prototype collection so that the compiler knows how to call the respective functions with their arguments
void tricornSeq_V0(float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img); 

void tricornPar_V1(float complex start, size_t width, size_t height, float res, unsigned n, unsigned char* img);

void print128_num(__m128 var);
