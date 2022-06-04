#include <stdbool.h>
#define error_exit(msg)		do { perror(msg); exit(EXIT_FAILURE); \
							} while (false)

#define MAX_LINE_SIZE 100