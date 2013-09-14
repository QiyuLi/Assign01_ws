/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Copyright 2012 by Gabriel Parmer.
 * Author: Gabriel Parmer, gparmer@gwu.edu, 2012
 */
/* 
 * This is a HTTP server.  It accepts connections on port 8080, and
 * serves a local static document.
 *
 * The clients you can use are 
 * - httperf (e.g., httperf --port=8080),
 * - wget (e.g. wget localhost:8080 /), 
 * - or even your browser.  
 *
 * To measure the efficiency and concurrency of your server, use
 * httperf and explore its options using the manual pages (man
 * httperf) to see the maximum number of connections per second you
 * can maintain over, for example, a 10 second period.
 *
 * Example usage:
 * # make test1
 * # make test2
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <sys/wait.h>
#include <pthread.h>
#include <string.h>
#include <util.h> 		/* client_process */
#include <server.h>		/* server_accept and server_create */

#include <ring_buffer.h>
#include <linked_list.h>
#include <cas.h>

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 4

pthread_mutex_t mutex;
pthread_cond_t worker_condition;
pthread_cond_t request_condition;
RingBuffer rb;
// request_node list;


/* 
 * This is the function for handling a _single_ request.  Understand
 * what each of the steps in this function do, so that you can handle
 * _multiple_ requests.  Use this function as an _example_ of the
 * basic functionality.  As you increase the server in functionality,
 * you will want to probably keep all of the functions called in this
 * function, but define different code to use them.
 */
void
server_single_request(int accept_fd)
{
	int fd;
	/* 
	 * The server thread will always want to be doing the accept.
	 * That main thread will want to hand off the new fd to the
	 * new threads/processes/thread pool.
	 */

	 	fd = server_accept(accept_fd);
		client_process(fd);
	/* 
	 * A loop around these two lines will result in multiple
	 * documents being served.
	 */

	return;
}

/* 
 * The following implementations use a thread pool.  This collection
 * of threads is of maximum size MAX_CONCURRENCY, and is created by
 * pthread_create.  These threads retrieve data from a shared
 * data-structure with the main thread.  The synchronization around
 * this shared data-structure is either done using mutexes + condition
 * variables (for a bounded structure), or compare and swap (__cas in
 * cas.h) to do lock-free synchronization on a stack or ring buffer.
 */

/*
 * Creates a pthread worker, locked based on a condition variable
 * that checks the file descriptor queue.
 */
void *worker()
{
  while (1) {
	 	pthread_mutex_lock(&mutex);
	  
	  while (buffer_size(&rb) == 0 || buffer_size(&rb) >= MAX_DATA_SZ) {
	  	pthread_cond_wait(&request_condition, &mutex);
	  }
	  int file_descriptor;
	  pop(&rb, &file_descriptor);
	  client_process(file_descriptor);
	
	  pthread_cond_signal(&worker_condition);
	  pthread_mutex_unlock(&mutex);
  }
	pthread_exit(0);
}

void
server_thread_pool_bounded(int accept_fd)
{
	// Sets up the buffers/pthreads/mutexes/condition variables
	buffer_init(&rb, MAX_DATA_SZ, sizeof(int));
	pthread_t threads[MAX_CONCURRENCY];
	
	pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&request_condition, NULL);
  pthread_cond_init(&worker_condition, NULL);

	for (int i = 0; i < MAX_CONCURRENCY; ++i)
	{
		pthread_create(&threads[i], NULL, worker, NULL);
	}

	// Starts the main request loop
	while (1) {
		pthread_mutex_lock(&mutex);

		// Notifies the main thread a worker has finished.
		while (buffer_size(&rb) != 0) {
			pthread_cond_wait(&worker_condition, &mutex);
		}

		// Gets the file descriptor and pushes it into the queue
		int fd = server_accept(accept_fd);
		push(&fd, &rb);

		// Unlockes the mutex and signals the pthread it can go to town.
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&request_condition);
	}
	pthread_exit(NULL);
	return;
}

/*
 * Creates a lockless worker that blocks when attempting
 * to fetch a new request.
 */ 
void *cas_worker()
{
  request_node *new_request;

  while (1) {
  	new_request = get_request();
  	client_process(new_request->file_descriptor);
  }
}

/*
 * Main function for the lock-less server.
 */ 
void
server_thread_pool_lock_free(int accept_fd)
{
	// Initializes the desired number of workers, and initializes
	// the global requests variable.
	pthread_t threads[MAX_CONCURRENCY];
	request_list_init();
	
	for (int i = 0; i < MAX_CONCURRENCY; ++i)
	{
		pthread_create(&threads[i], NULL, cas_worker, NULL);
	}
	

	// Infinite loop that checks for new requests.
	// Upon receipt of one, 'puts' the new request onto the
	// request list stack.
  while (1) {
  	int fd = server_accept(accept_fd);
  	request_node *new_request;
		new_request = init_node(fd);
		put_request(new_request);
  }
  pthread_exit(NULL);
	return;
}

typedef enum {
	SERVER_TYPE_ONE = 0,
	SERVER_TYPE_THREAD_POOL_BOUND,
	SERVER_TYPE_THREAD_POOL_ATOMIC,
} server_type_t;

int
main(int argc, char *argv[])
{
	server_type_t server_type;
	short int port;
	int accept_fd;

	if (argc != 3) {
		printf("Proper usage of http server is:\n%s <port> <#>\n"
		       "port is the port to serve on, # is either\n"
		       "0: server only a single request\n"
		       "1: use a thread pool and a _bounded_ buffer with "
		       "mutexes + condition variables\n"
		       "2: use a thread pool and compare and swap to "
		       "implement a lock-free stack of requests\n",
		       argv[0]);
		return -1;
	}

	port = atoi(argv[1]);
	accept_fd = server_create(port);
	if (accept_fd < 0) return -1;
	
	server_type = atoi(argv[2]);

	switch(server_type) {
	case SERVER_TYPE_ONE:
		server_single_request(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_BOUND:
		server_thread_pool_bounded(accept_fd);
		break;
	case SERVER_TYPE_THREAD_POOL_ATOMIC:
		server_thread_pool_lock_free(accept_fd);
		break;
	}
	close(accept_fd);

	return 0;
}
