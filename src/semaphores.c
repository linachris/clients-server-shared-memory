#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#include "common.h"
#include "semaphores.h"

union semun {
	int val;					/* Value for SETVAL */
	struct semid_ds* buf;		/* Buffer for IPC_STAT, IPC_SET */
	unsigned short* array;		/* Array for GETALL, SETALL */
};

int sem_create_init(int val) {
	int semid;
	if ((semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0660)) == -1)
		error_exit("sem_create_init|semget");

	union semun arg;
	arg.val = val;
	if (semctl(semid, 0, SETVAL, arg) == -1)
		error_exit("sem_create_init|semctl");
	
	return semid;
}

void sem_destroy(int semid) {
	if (semctl(semid, 0, IPC_RMID) == -1)
		error_exit("sem_remve|semctl");
}

void sem_down(int semid){ 
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = -1;
	sb.sem_flg = 0;

	if (semop(semid, &sb, 1) == -1)
		error_exit("sem_down|semop");
}

void sem_up(int semid) {
	struct sembuf sb;
	sb.sem_num = 0;
	sb.sem_op = 1;
	sb.sem_flg = 0;

	if (semop(semid, &sb, 1) == -1)
		error_exit("sem_up|semop");
}