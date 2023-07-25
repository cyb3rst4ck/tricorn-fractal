// Function prototype collection so that the compiler knows how to call the respective functions with their arguments
typedef void (*func_ptr)(complex float,  size_t,  size_t,  float,  unsigned int,  unsigned char *);

func_ptr selectAlgorithm(int implementation);
