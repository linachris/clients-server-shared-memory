// Creates a set of one semaphore, and initializes its value to val
int sem_create_init(int val);

// Destroys the semaphore with ID: semid
void sem_destroy(int semid);

// Decrements the sem_op by 1
void sem_down(int semid);

// Increments the sem_op by 1
void sem_up(int semid);