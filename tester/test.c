#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>	// para mallopt()
	#include <unistd.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <sys/mman.h>
	#include <errno.h>
	#include <string.h>
	#include <stdarg.h>


#pragma region "Includes"


#pragma endregion

#ifndef DEBUG_MODE
	#define DEBUG_MODE 0
#endif

#pragma region "Structures"

	typedef struct {
		char    *buffer;
		size_t  size;
		size_t  pos;
		int     error;
	} t_buffer;

#pragma endregion

#pragma region "Atomic Printf"

	#pragma region "Char"

		static void print_c_buf(int c, t_buffer *buf) {
			if (buf->error || buf->pos >= buf->size - 1) { buf->error = 1; return; }
			buf->buffer[buf->pos++] = c;
		}

	#pragma endregion

	#pragma region "String"

		static void print_s_buf(char *s, t_buffer *buf) {
			if (!s) s = "(null)";
			while (*s && !buf->error) print_c_buf(*s++, buf);
		}

	#pragma endregion

	#pragma region "Number"

		static void print_n_buf(unsigned long n, unsigned int base, t_buffer *buf, int uppercase) {
			char *strbase = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

			if		(base == 11) {	print_c_buf('-', buf);	base = 10; }
			else if (base == 17) {	print_s_buf("0x", buf);	base = 16; }

			if (n >= base)			print_n_buf(n / base, base, buf, uppercase);
			if (!buf->error)		print_c_buf(strbase[n % base], buf);
		}

	#pragma endregion

	#pragma region "Chooser"

		static void chooser_buf(int c, va_list args, t_buffer *buf) {
			if		(c == '%')		print_c_buf('%', buf);
			else if (c == 'c')		print_c_buf(va_arg(args, int), buf);
			else if (c == 's')		print_s_buf(va_arg(args, char *), buf);
			else if (c == 'u')		print_n_buf(va_arg(args, unsigned int), 10, buf, 0);
			else if (c == 'p')		print_n_buf(va_arg(args, unsigned long), 17, buf, 0);
			else if (c == 'x')		print_n_buf(va_arg(args, unsigned int), 16, buf, 0);
			else if (c == 'X')		print_n_buf(va_arg(args, unsigned int), 16, buf, 1);
			else if (c == 'd' || c == 'i') {
				long long n = va_arg(args, int);
				if (n < 0)			print_n_buf(n * -1, 11, buf, 0);
				else				print_n_buf(n, 10, buf, 0);
			}
		}

	#pragma endregion

	#pragma region "Atomic Printf"

		int ft_aprintf(int fd, char const *format, ...) {
			if (fd < 1) return (0);

			char buffer[4096];
			t_buffer buf = {
				.buffer = buffer,
				.size = sizeof(buffer),
				.pos = 0,
				.error = 0
			};

			va_list args;
			va_start(args, format);

			while (format && *format && !buf.error) {
				if (*format == '%')	chooser_buf(*(++format), args, &buf);
				else				print_c_buf(*format, &buf);
				format++;
			}

			va_end(args);

			if (buf.error && buf.pos > 0) buf.pos = buf.size - 1;
			if (buf.pos > 0) {
				int result = write(fd, buffer, buf.pos);
				return (result == -1 ? -1 : result);
			}

			return (0);
		}

	#pragma endregion

#pragma endregion

















// alineacion
	// #define ALIGNMENT 16
	// #define ALIGN(size) (((size) + ALIGNMENT - 1) & ~(ALIGNMENT - 1))
// arenas
// magic number
// zonas
// bitmap
// freelist
// tcache
// free: double free
// free: invalid pointer
// fusion de bloques
// liberacion de zonas
// lazy coalescing (or not)
// best fit vs first fit

void *my_malloc(size_t size) {   
    void *ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, -1, 0);
    
    if (ptr == MAP_FAILED) {
        perror("mmap failed in malloc");
        return NULL;
    }

    return ptr;
}

void sfree(void *ptr, size_t size) {
    if (!ptr) return;
    
    if (munmap(ptr, size) != 0) {
        ft_aprintf(1, "SFree failed: \t\t\t(%p)\n", ptr);
    } else {
        ft_aprintf(1, "SFree: \t\t\t\t(%p)\n", ptr);
    }
}

void *thread_test(void *arg) {
	int thread_num = *(int *)arg;
	char *str;
	int i;

	// Asignación y liberación
	for (i = 0; i < 1; i++) {
		str = malloc(64);
		if (str) {
			str[0] = 'a';
			ft_aprintf(1, "Hilo %d: Asignación #%d\t\t(%p)\n", thread_num, i, str);
			sleep(1);
			sfree(str, 64);
			//free(str);
		} else {
			ft_aprintf(1, "Hilo %d: Malloc failed\n", thread_num);
		}
	}

	// Asignación LARGE
	// str = malloc(1024 * 1024);
	// if (str) {
	// 	sprintf(str, "Asignación #%d", i);
	// 	// ft_aprintf(1, "Hilo %d: %s\t\t(%p)\n", thread_num, str, str);
	// 	sfree(str, 1024 * 1024);
	// } else {
	// 	// ft_aprintf(1, "Hilo %d: Malloc failed\n", thread_num);
	// }

	return (NULL);
}

void test_realloc() {
	// ft_aprintf(1, "\n=== Realloc ===\n\n");

	int initial = 8, extended = 128;
	char *ptr = malloc(initial);
	if (ptr) {
		strcpy(ptr, "inicial");
		// ft_aprintf(1, "Asignación %s    (%d bytes)\t(%p)\n", ptr, initial, ptr);

		ptr = realloc(ptr, extended);
		if (ptr) {
			strcpy(ptr, "ampliada");
			// ft_aprintf(1, "Asignación %s (%d bytes)\t(%p)\n", ptr, extended, ptr);
		} else {
			// ft_aprintf(1, "Realloc failed\n");
		}
		sfree(ptr, 128);
	} else {
		// ft_aprintf(1, "Realloc failed\n");
	}
}

void test_zones() {
	// ft_aprintf(1, "\n=== Zones ===\n\n");

	// Asignación TINY
	char *small = (char *)malloc(16);
	if (small) {
		strcpy(small, "TINY");
		// ft_aprintf(1, "Asignación %s\t\t\t(%p)\n", small, small);
	}

	// Asignación SMALL
	char *medium = (char *)malloc(570);
	if (medium) {
		strcpy(medium, "SMALL");
		// ft_aprintf(1, "Asignación %s\t\t(%p)\n", medium, medium);
	}

	// Asignación LARGE
	char *large = (char *)malloc(10240);
	if (large) {
		strcpy(large, "LARGE");
		// ft_aprintf(1, "Asignación %s\t\t(%p)\n", large, large);
	}

	sfree(small, 16);
	sfree(medium, 570);
	sfree(large, 10240);
}

int main() {
	//mallopt(7, DEBUG_MODE);
	
	// test_zones();
	// test_realloc();
	// ft_aprintf(1, "\n=== Threads ===\n\n");
	int i, n_threads = 5;
	int thread_args[n_threads];
	pthread_t threads[n_threads];
	
	for (i = 0; i < n_threads; i++) {
		thread_args[i] = i + 1;
		if (pthread_create(&threads[i], NULL, thread_test, &thread_args[i]) != 0) {
			// ft_aprintf(1, "Thread creation failed\n");
			n_threads = i; break;
		}
	}
	sleep(2);
	ft_aprintf(1, "fin\n");
	for (i = 0; i < n_threads; i++) {
		ft_aprintf(1, "Esperando hilo %d...\n", i+1);
		if (pthread_join(threads[i], NULL) != 0) {
			ft_aprintf(1, "Error en pthread_join para hilo %d\n", i+1);
		} else {
			ft_aprintf(1, "Hilo %d terminó correctamente\n", i+1);
		}
	}
	
	return (0);
}

// A partir del 5º hilo hay un munmap failed por hilo, pero lo raro es que solo pasa en el hilo 1
