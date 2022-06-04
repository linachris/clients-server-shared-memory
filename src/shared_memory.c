#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "common.h"
#include "shared_memory.h"

int shm_create(size_t size) {
	int shmid;
	if ((shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0660)) == -1)
		error_exit("shm_create|shmget");
	return shmid;
}

int shm_get(size_t size) {
	int shmid;
	if ((shmid = shmget(IPC_PRIVATE, size, 0)) == -1)
		error_exit("shm_get|shmget");
	return shmid;
}

void shm_destroy(int shmid) {
	if (shmctl(shmid, IPC_RMID, NULL) == -1)
		error_exit("shm_destroy|shmctl");
}

void* shm_attach(int shmid) {
	void* shared_memory;
	if ((shared_memory = shmat(shmid, NULL, 0)) == (void*)-1)
		error_exit("shm_attach|shmat");
	return shared_memory;
}

void shm_detach(void* sh_mem) {
	if (shmdt(sh_mem) == -1)
		error_exit("shm_detach|shmdt");
}