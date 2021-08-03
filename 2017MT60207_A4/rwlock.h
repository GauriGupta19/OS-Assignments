#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>


struct read_write_lock{
	sem_t lock; // binary semaphore (basic lock)
	sem_t writelock; // allow ONE writer/MANY readers
	sem_t priority_w;
	int readers; // #readers in critical section

};

void InitalizeReadWriteLock(struct read_write_lock * rw);
void ReaderLock(struct read_write_lock * rw);
void ReaderUnlock(struct read_write_lock * rw);
void WriterLock(struct read_write_lock * rw);
void WriterUnlock(struct read_write_lock * rw);
