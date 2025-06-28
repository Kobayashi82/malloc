#include <stdlib.h>

int main(void)
{
    int   i;
    char  *addr;

    i = 0;
    while (i < 1024) 
    {
        addr = (char*)malloc(1024);	
        addr[0] = 42;
		free(addr);					// 5/5
		// if (i >= 15) free(addr);	// 4/5
		// if (i >= 25) free(addr);	// 3/5
		// if (i >= 40) free(addr);	// 2/5
		// if (i >= 55) free(addr);	// 1/5
        i++; 
    }

    return (0);
}
