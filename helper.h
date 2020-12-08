/******************************************************************
 * Header file for the helper functions. This file includes the
 * required header files, as well as the function signatures and
 * the semaphore values (which are to be changed as needed).
 ******************************************************************/


# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/types.h>
# include <sys/ipc.h>
# include <sys/shm.h>
# include <sys/sem.h>
# include <sys/time.h>
# include <math.h>
# include <errno.h>
# include <string.h>
# include <pthread.h>
# include <ctype.h>
# include <iostream>
using namespace std;

// Global variables
#define FULL_SEMAPHORE 0
#define MUTEX_SEMAPHORE 1
#define EMPTY_SEMAPHORE 2
#define ID_SEMAPHORE 3
#define COUT_SEMPHORE 4
#define NUM_SEMAPHORES 5
#define MAX_PROD_TIME 5
#define MAX_JOB_TIME 10
#define TIMEOUT 20
#define NO_ERROR 0
#define ERROR -1


union semun {
    int val;               /* used for SETVAL only */
    struct semid_ds *buf;  /* used for IPC_STAT and IPC_SET */
    ushort *array;         /* used for GETALL and SETALL */
};

// Queue data structure
struct CircularQueue {
    int* jobs;
    int head;
    int tail;
    uint num_jobs;
};

// Producer parameters data structure
struct ProducerParams {
  CircularQueue* queue;
  int sem_id;
  int thread_id;
  int queue_size;
  uint num_jobs;
};

// Consumer parameters data structure
struct ConsumerParams {
  CircularQueue* queue;
  int sem_id;
  int thread_id;
  int queue_size;
};

int check_arg (char *);
int sem_create (key_t, int);
int sem_init (int, int, int);
void sem_wait (int, short unsigned int);
void sem_signal (int, short unsigned int);
int sem_close (int);
bool valid_input(int argc, char** argv);

// overloaded wait function with timeout
int sem_wait (int, short unsigned int, const int);

//produce and consume sleep functions
void produce(int min, int max);
void consume(int time);
