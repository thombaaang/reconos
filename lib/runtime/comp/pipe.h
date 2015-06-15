#ifndef PIPE_H
#define PIPE_H

#include <semaphore.h>

/*
 * Structure representing a pipe
 */
struct pipe {
	sem_t write, read;
	void *buf;
	size_t len;
};

/*
 * Initializes the pipe and must be called before usage.
 *
 *   pipe - pointer to the pipe
 *
 *   @return'errror number
 */
extern int pipe_init(struct pipe *pipe);

/*
 * Frees all used resources of the pipe.
 *
 *   pipe - pointer to the pipe
 */
extern void pipe_destroy(struct pipe *pipe);

/*
 * Writes data to the pipe. Not more than count bytes will be
 * written. Always check return value for number of written bytes. This
 * method is NOT thread safe and should not be called concurrently.
 *
 *   pipe - pointer to the pipe
 *   data - pointer to the data to write
 *   len  - number of bytes to write
 *
 *   @returns number of written bytes
 */
extern ssize_t pipe_write(struct pipe *pipe, void *data, size_t len);

/*
 * Issues a write request to block until pipe allows write.
 *
 *   pipe - pointer to the pipe
 *   len  - number of bytes to write
 *
 *   @returns number of bytes that can be written
 */
extern ssize_t pipe_writereq(struct pipe *pipe, size_t len);

/*
 * Writes to the pipe after the request was granted.
 *
 *   pipe - pointer to the pipe
 *   data - pointer to the data to write
 */
extern void pipe_writeareq(struct pipe *pipe, void *data);

/*
 * Reads data from the pipe. Not more than count bytes will be
 * read. Always chack return value for number of read bytes. This method is
 * NOT thread safe and should not be called concurrently.
 *
 *   pipe - pointer to the pipe
 *   data - pointer to a buffer to write data to
 *   len  - number of bytes to read
 *
 *   @returns nummber of read bytes
 */
extern ssize_t pipe_read(struct pipe *pipe, void *data, size_t len);

#endif /* PIPE_H */