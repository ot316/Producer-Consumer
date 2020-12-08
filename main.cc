/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

int main (int argc, char **argv)
{
  // Random seed for semaphore key
  srand(time(NULL));
  const int SEM_KEY = rand() % 0xabcd + 1;

  // Check arguments
  if (!valid_input(argc, argv)) {
    cerr << "Incorrect usage. Try '"<< argv[0] <<" <queuesize> <jobs_per_producer> <num_producers> <num_consumers>' where the arguments are integers.\n";
    return  -1;
  }
  // Parse arguments
  const uint queue_size = check_arg(argv[1]);
  const uint jobs_per_producer = check_arg(argv[2]);
  const uint num_producers = check_arg(argv[3]);
  const uint num_consumers = check_arg(argv[4]);

  int error_code = 0, jobs[queue_size];

  // Create semaphore set
  const int sem_id = sem_create(SEM_KEY, NUM_SEMAPHORES);
  // Mutual exclusion binary semaphore
  error_code = sem_init(sem_id, MUTEX_SEMAPHORE, 1);
  if (error_code) {
    cerr << "Mutex semaphore initialisation error with key: " << SEM_KEY << endl;
    return -1;
  }
  // Prevent a producer from accessing a full buffer
  error_code = sem_init(sem_id, FULL_SEMAPHORE, queue_size);
  if (error_code) {
    cerr << "Full semaphore initialisation error with key: " << SEM_KEY << endl;
    return -1;
  }
  // Prevent a consumer accessing an empty buffer
  error_code = sem_init(sem_id, EMPTY_SEMAPHORE, 0);
    if (error_code) {
    cerr << "Empty semaphore initialisation error with key: " << SEM_KEY << endl;
    return -1;
  }
  // Prevent producers or consumers being assigned the same ID.
  error_code = sem_init(sem_id, ID_SEMAPHORE, 0);
  if (error_code) {
    cerr << "ID semaphore initialisation error with key: " << SEM_KEY << endl;
    return -1;
  }
  // Prevent producers or consumers being assigned the same ID.
  error_code = sem_init(sem_id, COUT_SEMPHORE, 0);
  if (error_code) {
    cerr << "cout semaphore initialisation error with key: " << SEM_KEY << endl;
    return -1;
  }
  
  // Initialise producer and consumer threads
  pthread_t producer_threads[num_producers];
  pthread_t consumer_threads[num_consumers];

  // Initialise circular queue structure
  CircularQueue cq;
  cq.head = 0;
  cq.tail = 0;
  cq.num_jobs = queue_size;
  cq.jobs = jobs;

  ProducerParams prod_params;
  prod_params.sem_id = sem_id;
  prod_params.queue_size = queue_size;
  prod_params.num_jobs = jobs_per_producer;
  prod_params.queue = &cq;

  ConsumerParams cons_params;
  cons_params.sem_id = sem_id;
  cons_params.queue_size = queue_size;
  cons_params.queue = &cq;
  
  // create producer threads
  auto i = 0u;
  while (i < num_producers) {
    prod_params.thread_id = ++i;
    error_code = pthread_create (&producer_threads[i], NULL, producer, (void *) &prod_params);
    if (error_code) {
      cerr << "Error creating producer thread. Thread ID: " << i << endl;
      return -1;
    }
    sem_wait(sem_id, ID_SEMAPHORE);
  }

  // create consumer threads; 
  i = 0;
  while (i < num_consumers) {
    cons_params.thread_id = ++i;
    error_code = pthread_create (&consumer_threads[i], NULL, consumer, (void *) &cons_params);
    if (error_code) {
      cerr << "Error creating consumer thread. Thread ID: " << i << endl;
      return -1;
    }
    sem_wait(sem_id, ID_SEMAPHORE);
  }

  // Join producer threads:
  for (auto i = 0u; i < num_producers; ++i) {
    error_code = pthread_join(producer_threads[i], NULL);
    if (error_code) {
      cerr << "Error joining producer thread. Thread ID: " << i << endl;
      return -1;
    }
  }
  // Join consumer threads:
  for (auto i = 1u; i < num_consumers; ++i) {
    error_code = pthread_join(consumer_threads[i], NULL);
      if (error_code) {
      cerr << "Error joining consumer thread. Thread ID: " << i << endl;
      return -1;
    }
  }
  error_code = sem_close(sem_id);
  if (error_code) {
    cerr << "Failed to close semaphore set, please do this manually.\n";
    return -1;
  }
  return NO_ERROR;
}


void *producer (void *params) 
{

  ProducerParams* parameters = static_cast<ProducerParams*>(params);

  int id = parameters->thread_id;
  bool timeout = false;

  // release ID sempaphore
  sem_signal(parameters->sem_id, ID_SEMAPHORE);
  
  for (auto i = 0u; i < parameters->num_jobs; ++i) {

    uint job_time = rand() % MAX_JOB_TIME + 1;

    produce(1, MAX_PROD_TIME);
    timeout = sem_wait(parameters->sem_id, FULL_SEMAPHORE, TIMEOUT);

    //check timeout
    if (timeout) {
      cerr << "Producer has timed out. Producer ID: " << id << endl;
      pthread_exit(0);
    }

    sem_wait(parameters->sem_id, MUTEX_SEMAPHORE);

    auto& tail = parameters->queue->tail;

    const auto job_id = tail + 1;
    parameters->queue->jobs[tail] = job_time;
    tail = job_id % parameters->queue_size;

    sem_signal(parameters->sem_id, MUTEX_SEMAPHORE);
    sem_signal(parameters->sem_id, EMPTY_SEMAPHORE);

    fprintf(stderr, "Producer(%d): Job ID %d duration %d.\n", id, job_id, job_time);
  }
  if (!timeout)
    fprintf(stderr, "Producer(%d): No more jobs to generate.\n", id);

  pthread_exit(0);
}


void *consumer (void *params) 
{
  ConsumerParams* parameters = static_cast<ConsumerParams*>(params);

  int id = parameters->thread_id;

  sem_signal(parameters->sem_id, ID_SEMAPHORE);

  while(!sem_wait(parameters->sem_id, EMPTY_SEMAPHORE, TIMEOUT)) {

    sem_wait(parameters->sem_id, MUTEX_SEMAPHORE);

    auto& head = parameters->queue->head;
    const auto job_id = head + 1;
    const auto job_time = parameters->queue->jobs[head];
    head = job_id % parameters->queue_size;

    //sem_wait(parameters->sem_id, COUT_SEMPHORE);
    fprintf(stderr, "Consumer(%d): Job ID %d executing sleep duration %d.\n", id, job_id, job_time);
    //sem_signal(parameters->sem_id, COUT_SEMPHORE);

    sem_signal(parameters->sem_id, MUTEX_SEMAPHORE);
    sem_signal(parameters->sem_id, FULL_SEMAPHORE);

    consume(job_time);
    //sem_wait(parameters->sem_id, COUT_SEMPHORE);
    fprintf(stderr, "Consumer(%d): Job ID %d completed.\n", id, job_id);
    //sem_signal(parameters->sem_id, COUT_SEMPHORE);
  }
  fprintf(stderr, "Consumer(%d): No jobs left.\n", id);

  pthread_exit(0);
}

/* produce and consume functions to be modified later, 
current implementation sleeps for a random amount of time.  */
void produce(int min, int max) {
  int sleep_time = rand() % max + min;
  sleep(sleep_time);
}

void consume(int time) {
  sleep(time);
}