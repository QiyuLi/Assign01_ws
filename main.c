/**
 * Redistribution of this file is permitted under the GNU General
 * Public License v2.
 *
 * Modifier: Qiyu Li, johnnyli@gwu.edu, 2013
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

#include <util.h> 		/* client_process */
#include <server.h>		/* server_accept and server_create */

#include <cas.h>

#include <ring_buffer.h>

#define MAX_DATA_SZ 1024
#define MAX_CONCURRENCY 4

/*
 * Define a mutex.
 */
pthread_mutex_t mutex;

/*
 * Define thread conditions
 */
pthread_cond_t master_cond;
pthread_cond_t worker_cond;

ring_buffer_t ring_buffer; /* define ring buffer */



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
 * Define a pthread worker per request start routine.
 * Each thread will receive a file descripter fd and invoke
 * client_process to deal it.
 */
void *worker_per_request(void *fd)
{
    client_process((int) fd);
    pthread_exit(0);
}

/*
 * The following implementation creates a new thread per client
 * request using the pthread API, and that thread is removed/killed
 * when the request is completed.
 */
void
server_thread_per_req(int accept_fd)
{
    int fd;
    int i = 0;

    pthread_t thread[MAX_CONCURRENCY];

    /* Start main loop */
    while(1) {
        /* create threads until max concurrency */
        for(i = 0; i < MAX_CONCURRENCY; i++) {
            fd = server_accept(accept_fd);
            pthread_create(&thread[i], NULL, &worker_per_request, (void *) fd);
        }

        /* join threads created until max concurrency */
        for(i = 0; i < MAX_CONCURRENCY; i++) {
            pthread_join(thread[i], NULL);
        }
    }

    return;
}

/*
 * Creates a pthread worker, locked on a condition variable that checks
 *  the file descriptor ring buffer. Will wait if ring buffer is empty.
 */
void *server_thread_pool_bounded_worker()
{
    /* Worker's main loop */
    while (1) {
        pthread_mutex_lock(&mutex);

        /* if ring buffer is empty, wait master push data and send signal */
        while (ring_buffer_is_empty(&ring_buffer) == 0) {
            pthread_cond_wait(&master_cond, &mutex);
        }

        int fd;
        ring_buffer_pop(&ring_buffer, &fd); /* get file descriptor from ring buffer */
        pthread_mutex_unlock(&mutex);
        /* send signal if ring buffer is full and master is waiting for signal to wake up */
        pthread_cond_signal(&worker_cond);

        client_process(fd);
    }
    pthread_exit(0);
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
void
server_thread_pool_bounded(int accept_fd)
{
    int i = 0;

    /* Init references */
    ring_buffer_init(&ring_buffer, sizeof(int), MAX_DATA_SZ);
    pthread_t threads[MAX_CONCURRENCY];
    if(pthread_mutex_init(&mutex, NULL) != 0) {
        return -1; /* return if mutex init fails */
    }
    if(pthread_cond_init(&master_cond, NULL) != 0) {
        return -1; /* return if master_cond init fails */
    }
    if(pthread_cond_init(&worker_cond, NULL) != 0) {
        return -1; /* return if worker_cond init fails */
    }

    /* Create worker threads */
    for (i = 0; i < MAX_CONCURRENCY; ++i) {
        pthread_create(&threads[i], NULL, server_thread_pool_bounded_worker, NULL);
    }

    /* Starts main loop */
    while (1) {
        int fd = server_accept(accept_fd);

        pthread_mutex_lock(&mutex);
        /* Check ring buffer, if it is full, then wait workers' signal */
        while(ring_buffer_is_full(&ring_buffer) == 0) {
            pthread_cond_wait(&worker_cond, &mutex);
        }

        ring_buffer_push(&fd, &ring_buffer);

        // Unlockes the mutex and send signal to workers..
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&master_cond);
    }

    pthread_exit(0);
}


typedef enum {
    SERVER_TYPE_ONE = 0,
    SERVER_TYPE_THREAD_PER_REQUEST,
    SERVER_TYPE_THREAD_POOL_BOUND,
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
               "0: serve only a single request\n"
               "1: serve each request with a new thread\n"
               "2: use a thread pool and a _bounded_ buffer with "
               "mutexes + condition variables\n",
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
    case SERVER_TYPE_THREAD_PER_REQUEST:
        server_thread_per_req(accept_fd);
        break;
    case SERVER_TYPE_THREAD_POOL_BOUND:
        server_thread_pool_bounded(accept_fd);
        break;
    }
    close(accept_fd);

    return 0;
}
