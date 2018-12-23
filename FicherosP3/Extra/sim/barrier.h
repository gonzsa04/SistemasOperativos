#ifndef  BARRIER_H
#define  BARRIER_H
#include <pthread.h>

#if defined(__APPLE__) && defined(__MACH__)
#undef POSIX_BARRIER    /* MAC OS X does not implement pthread's barriers so use custom implementation instead */
#endif

#ifdef POSIX_BARRIER
typedef pthread_barrier_t sys_barrier_t;

#else
#include <semaphore.h>
/* Synchronization barrier */
typedef struct {
	sem_t mutex;			/* Barrier lock */
	sem_t cond_slave;		/* Variable where threads remain blocked */
	sem_t cond_master;		/* Handshake master-slave */
	int nr_threads_arrived;		/* Number of threads that reached the barrier */
	int max_threads;		/* Number of threads that rely on the syncronization barrier */
} sys_barrier_t;
#endif

sys_barrier_t mybarrier;

int sys_barrier_init(sys_barrier_t* barrier, unsigned int nthreads);
int sys_barrier_destroy(sys_barrier_t* barrier);
int sys_barrier_wait(sys_barrier_t *barrier);

#endif // Barrier


