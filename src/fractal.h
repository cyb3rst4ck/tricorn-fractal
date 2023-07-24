typedef void (*func_ptr)(complex float,  size_t,  size_t,  float,  unsigned int,  unsigned char *);

func_ptr selectAlgorithm(int implementation);