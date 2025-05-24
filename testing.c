#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

	#include <stdio.h>
	#include <pthread.h>
	#include <stdlib.h>
	#include <sys/mman.h>
	#include <errno.h>
	#include <string.h>
	#include <unistd.h>

	#ifdef __linux__
		#include <sys/sysinfo.h>
	#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
		#include <sys/types.h>
		#include <sys/sysctl.h>
	#elif defined(_WIN32)
		#include <windows.h>
	#endif

// cc -Wno-free-nonheap-object testing.c
int main() {
	
	// free(): double free detected in tcache 2
	// char popo[] = "hola";
	// free(popo);

	// free(): invalid pointer
	// char *popo = malloc(100);
	// free(&popo[50]);

	// free(): invalid pointer
	// int x = 10;
	// free(&x);

	// invalid size
	// static int x = 10;
	// free(&x);

	// munmap_chunk(): invalid pointer
	// int x = 100;
	// free(&x);

	// kill(getpid(), SIGABRT);		// Abort execution
	while(1);
}
