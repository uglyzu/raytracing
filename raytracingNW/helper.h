#ifndef HELPER
#define HELPER

#define M_PI 3.14159265358979323846

static unsigned long long seed = 1;

double drand48()
{
	seed = (0x5DEECE66DLL * seed + 0xB16) & 0xFFFFFFFFFFFFLL;
	unsigned int x = seed >> 16;
	return  ((double)x / (double)0x100000000LL);
}

void srand48(unsigned int i)
{
	seed = (((long long int)i) << 16) | rand();
}

#endif

