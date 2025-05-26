#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/mman.h>
#include <errno.h>
#include <string.h>

#include "malloc.h"

#ifdef __linux__
	#include <sys/sysinfo.h>
#elif defined(__APPLE__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__)
	#include <sys/types.h>
	#include <sys/sysctl.h>
#elif defined(_WIN32)
	#include <windows.h>
#endif

// gcc -g -Wno-free-nonheap-object -o testing testing.c -I./inc -L./build/lib -lft_malloc -Wl,-rpath=./build/lib -pthread
//
// -Wno-free-nonheap-object	= Desactiva advertencia de free() al compilar
// -lft_malloc				= -l busca lib + ft_malloc + .so
// -Wl,-rpath=./build/lib	= Pasa al linker el parametro rpath para que busque en esa ruta las bibliotecas en runtime

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
	printf("Zone: %u - index: %u\n", get_zonetype(0), get_freelist_index(0));
	printf("Zone: %u - index: %u\n", get_zonetype(16), get_freelist_index(16));
	printf("Zone: %u - index: %u\n", get_zonetype(325), get_freelist_index(325));
	printf("Zone: %u - index: %u\n", get_zonetype(512), get_freelist_index(512));
	printf("Zone: %u - index: %u\n", get_zonetype(513), get_freelist_index(513));
	printf("Zone: %u - index: %u\n", get_zonetype(4096), get_freelist_index(4096));
	printf("Zone: %u - index: %u\n\n", get_zonetype(5000), get_freelist_index(5000));

	options_initialize();
	printf("%s\n", g_arena_manager.options.LOGFILE);

	// print_freelist_ranges();
	//while(1);
}
