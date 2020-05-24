
#define MAX     10000000
static inline void delay(unsigned long max)
{
	int i = 0;
	int j = 0;
	int k = 0;

	while (i++ < max)
		while (j++ < MAX)
			while (k++ < MAX);
}

