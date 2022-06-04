#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>

#include "semaphores.h"
#include "shared_memory.h"
#include "common.h"

struct shared_memory_st {
	unsigned int nlines;
	unsigned int line_num;
	char ret_line[MAX_LINE_SIZE];
};

// Returns a random line number between [1,num_of_lines]
unsigned int random_line_num(int num_of_lines) {
    return rand() % num_of_lines + 1;
}

// Returns the total num of lines in the file from the path given
unsigned int total_lines(char* file_path) {
	FILE* X = fopen(file_path, "r");
	if (X == NULL)
		error_exit("X:fopen failed");
	
	char lines[MAX_LINE_SIZE];
    unsigned int num_of_lines = 0;
	while ((fgets(lines, MAX_LINE_SIZE, X) != NULL))
        num_of_lines++;
	
	fclose(X);
    return num_of_lines;
}
// Destroys Shared Memory Segment and Semaphores
void free_resources(void* shm_st, int shmid, int mutex, int server, int client) {
	shm_detach(shm_st);
	shm_destroy(shmid);
	sem_destroy(mutex);
	sem_destroy(server);
	sem_destroy(client);
}

int main(int argc, char* argv[]) {

	int K, N, shmid, state;
	unsigned int num_of_lines;
	FILE *X, *reqst_fp, *avg_times_fp;
	pid_t pid;
	struct shared_memory_st* shm_st;

	// Correct number of arguments needed
	if (argc != 4) {
		fprintf(stderr, "Error! Correct Usage: ./server <Χ:FileName> <Κ:No of Clients> <Ν:No of Transactions>\n");
		exit(EXIT_FAILURE);
	}

	K = atoi(argv[2]); // Num of clients(children)
	N = atoi(argv[3]); // Num of transactions each child participates in

	// ********************* File Handling ********************* //
	char file_path[PATH_MAX] = "txt/";
	strcat(file_path, argv[1]);

	// Initializing the files of the clients' requests output, and the average time of the response,
	// and deleting their contents if they already exist
	fclose(fopen("requests.log", "w"));
	fclose(fopen("avg_times.log", "w"));

	// ****************** Semaphores and Shared Memory Handling ****************** //
	// Creating and Initializing 3 Binary Semaphores
	int client = sem_create_init(0);	// block server when client on its CS
	int server = sem_create_init(0);	// block client when server on its CS
	int mutex = sem_create_init(1);		// clients mutual exclusion + clients creation handling

	// Creating and Attaching the Shared Memory Segment
	shmid = shm_create(sizeof(struct shared_memory_st));
	shm_st = (struct shared_memory_st*)shm_attach(shmid);

	// The Server, sharing X's num of lines with the clients, through the shm segmt
	shm_st->nlines = total_lines(file_path);

	// Fork and create the clients/children of the server
	// Children are being blocked -> can't enter their CS until all the children have been created
	sem_down(mutex);	
	for (int i = 0; i < K; i++) {
		pid = fork();
		if (pid < 0) {
			free_resources(shm_st, shmid, mutex, server, client);
			error_exit("Server|fork()");
		}
		else if (pid == 0)	{
			// we're in the child process
			printf("Child created with PID: %d and Parent ID: %d\n", getpid(), getppid());
			break;	// in case a child creates its own child
		}
	}
	// Making sure all clients/children have been created.
	// The parent has left the loop of creating the children, so the children are unblocked,
	// and they can enter their if statement..->..CS
	if (pid != 0)
		sem_up(mutex);

	// Server - Parent Process
	if (pid != 0) {
		for (int i = 0; i < K*N; i++) {
			// server sem initialized to 0, so that server is blocked 
			sem_down(server); // and the client enters its CS first	

			char buff[MAX_LINE_SIZE];
			num_of_lines = 1;

			// Opening the file and finding the requested line
			X = fopen(file_path, "r");
			if (X == NULL) {
				error_exit("X:fopen failed");
				free_resources(shm_st, shmid, mutex, server, client);
			}
			while ((fgets(buff, MAX_LINE_SIZE, X) != NULL) && (num_of_lines != shm_st->line_num))
				num_of_lines++;
			strncpy(shm_st->ret_line, buff, MAX_LINE_SIZE); // and updating the shared data
			fclose(X);
			// unblocking the client, to access the shared data
			sem_up(client);
		}
	}
	
	// Client - Child Process
	else if (pid == 0) {
		long double total_time = 0;
		struct timespec start, end;

		srand(time(NULL) % getpid());
		for (int i = 0; i < N; i++) {
			// if the parent isn't done creating all the children, the child is being blocked from entering its CS
			// until the parent releases the mutex semaphore!
			sem_down(mutex);	// also, excluding all other clients from accessing the resource

			// Client requesting a specific random line from the file
			shm_st->line_num = random_line_num(shm_st->nlines);

			// The request was made! -> time: start
			clock_gettime(CLOCK_MONOTONIC_RAW, &start);

			sem_up(server);		// Unblocking and allowing the server to operate and return the requested line through the shm
			sem_down(client);	// Client init to 0, so it's blocked
			
			// Accessing the resource and obtaining the requested line
			// Request was approved and completed! -> time: end
			char* returned_line = shm_st->ret_line;
			clock_gettime(CLOCK_MONOTONIC_RAW, &end);
			total_time += ((end.tv_nsec - start.tv_nsec) / 1000000000.0 + (end.tv_sec  - start.tv_sec));

			// Open the file and append it with the child's request 
			reqst_fp = fopen("requests.log", "a");
			if (reqst_fp == NULL) {
				free_resources(shm_st, shmid, mutex, server, client);
				error_exit("reqst_fp:fopen failed");
			}
			fprintf(reqst_fp, "Child with PID: %d, requested the line %d: %s\n", getpid(), shm_st->line_num, returned_line);
			fclose(reqst_fp);

			sem_up(mutex);
		}
		// Opening the file for writing the average time of the N requests for each child  
		avg_times_fp = fopen("avg_times.log", "a");
		if (avg_times_fp == NULL) {
			free_resources(shm_st, shmid, mutex, server, client);
			error_exit("avg_times_fp:fopen failed");
		}
		fprintf(avg_times_fp, "Child with PID: %d, Request's completion Average time %Lf\n", getpid(), total_time / (long double)N);
		fclose(avg_times_fp);

		exit(EXIT_SUCCESS);	// Child process exits successfully
	}

	// Father waits for all the children to terminate
    while((wait(&state)) > 0) {};

	printf("See files: \"requests.log\" for the clients' requests &,\n  	   \"avg_times.log\" for the average time of the requests' completion\n");

	// Destroy Shared Memory Segment and Semaphores
	free_resources(shm_st, shmid, mutex, server, client);
  	exit(EXIT_SUCCESS);   // Parent process exits successfully
}