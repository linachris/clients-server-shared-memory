#include <sys/types.h>

// Creates a shared memory segment of size bytes
int shm_create(size_t size);

// Obtains an already created memory segment
int shm_get(size_t size);

// Destroys a shared memory segment with the ID: shmid
void shm_destroy(int shmid);

void* shm_attach(int shmid);

void shm_detach(void* sh_mem);