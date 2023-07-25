#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

// Function prototype collection so that the compiler knows how to call the respective functions with their arguments
void firstTestCompareArrays(unsigned char* imgArrayBitmap, size_t size);

void callDesiredTestcase(unsigned char* imgArrayBitmap, size_t size, int testcase);
