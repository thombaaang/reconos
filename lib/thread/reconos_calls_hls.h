/*
 *                                                        ____  _____
 *                            ________  _________  ____  / __ \/ ___/
 *                           / ___/ _ \/ ___/ __ \/ __ \/ / / /\__ \
 *                          / /  /  __/ /__/ /_/ / / / / /_/ /___/ /
 *                         /_/   \___/\___/\____/_/ /_/\____//____/
 *
 * ======================================================================
 *
 *   title:        Calls library header file
 *
 *   project:      ReconOS
 *   author:       Andreas Agne, University of Paderborn
 *                 Christoph Rüthing, University of Paderborn
 *   description:  ReconOS calls to allow interaction with POSIX-API of
 *                 system calls.
 *
 * ======================================================================
 */

<<reconos_preproc>>

#ifndef RECONOS_CALLS_H
#define RECONOS_CALLS_H

#include "reconos_defs.h"

#include "hls_stream.h"
#include "ap_cint.h"


/* == Internal functions =============================================== */

/*
 * Writes blocking to a stream using the non-blocking method. Since the
 * non-blocking write is called in a loop, vivado hls enforces sequential
 * order, which is necessary for osif and memif calls.
 *
 *   stream - reference to stream
 *   data   - data to write
 */
static inline void stream_write(hls::stream<uint32> &stream, uint32 data) {
#pragma HLS inline
	while (!stream.write_nb(data)){}
}

/*
 * Reads blocking from a stream using the non-blocking method. Since the
 * non-blocking read is called in a loop, vivado hls enforces sequential
 * order, which is necessary for osif and memif calls.
 *
 *   stream - reference to stream
 *
 *   @returns read data
 */
static inline uint32 stream_read(hls::stream<uint32> &stream) {
#pragma HLS inline
	uint32 data;
	while (!stream.read_nb(data)){}
	return data;
}

#define CONCAT_CTRL(a,b,c)\
	apint_concatenate(apint_concatenate((uint8)(a),(uint8)(b)), uint16(c))

/* == Call functions =================================================== */

/*
 * Creates a local ram to be used for mem functions. You may only pass
 * rams created by this macro to mem functions.
 *
 *   type - datatype of the ram 
 *   size - size of the ram
 *   name - name of the ram
 */
#define RAM(type,size,name)\
	type name[size]

/*
 * Posts the semaphore specified by handle.
 *
 *   @see sem_post
 */
#define SEM_POST(res)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_SEM_POST),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Waits for the semaphore specified by handle.
 *
 *   @see sem_wait
 */
#define SEM_WAIT(res)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_SEM_WAIT),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Locks the mutex specified by handle.
 *
 *   @see pthread_mutex_lock
 */
#define MUTEX_LOCK(res)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_MUTEX_LOCK),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Unlocks the mutex specified by handle.
 *
 *   @see pthread_mutex_unlock
 */
#define MUTEX_UNLOCK(res)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_MUTEX_UNLOCK),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Tries to lock the mutex specified by handle and returns if successful or not.
 *
 *   @see pthread_mutex_trylock
 */
#define MUTEX_TRYLOCK(res)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_MUTEX_TRYLOCK),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Puts a single word into the mbox specified by handle.
 *
 *   @see mbox_get
 */
#define MBOX_GET(res)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_MBOX_GET),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Reads a single word from the mbox specified by handle.
 *
 *   @see mbox_put
 */
#define MBOX_PUT(res,data)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0002)),\
	stream_write(osif_hw2sw, OSIF_CMD_MBOX_PUT),\
	stream_write(osif_hw2sw, data),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Tries to put a single word into the mbox specified by handle but does not
 * blocks until the mbox gets populated.
 *
 *   @see mbox_tryget
 */
#define MBOX_TRYGET(res,data)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_MBOX_TRYGET),\
	stream_read(osif_sw2hw),\
	data = stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Tries to read a single word from the mbox specified by handle but does not
 * blocks until the mbox gets free.
 *
 *   @see mbox_tryput
 */
#define MBOX_TRYPUT(res,data)(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, res, 0x0002)),\
	stream_write(osif_hw2sw, OSIF_CMD_MBOX_TRYPUT),\
	stream_write(osif_hw2sw, data),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Writes data to the pipe. Len represents bytes, not words.
 */
#define PIPE_WRITE(res,data,len)\
	_PIPE_WRITE(osif_hw2sw, osif_sw2hw, __run_id, res, data, len)

static inline int _PIPE_WRITE(hls::stream<uint32> &osif_hw2sw,
                              hls::stream<uint32> &osif_sw2hw,
                              uint8 run_id,
                              uint8 res, uint32 *data, int len) {
	#pragma HLS inline

	stream_write(osif_hw2sw, CONCAT_CTRL(run_id, res, 0x0002));
	stream_write(osif_hw2sw, OSIF_CMD_PIPE_WRITE);
	stream_write(osif_hw2sw, len);

	stream_read(osif_sw2hw);
	uint16 min_len = apint_get_range(stream_read(osif_sw2hw), 17, 2);
	stream_write(osif_hw2sw, CONCAT_CTRL(run_id, res, min_len));
	for (int i = 0; i < min_len; i++) {
		stream_write(osif_hw2sw, data[i]);
	}

	return len << 2;
}

/*
 * Reads data from the pipe.
 */
#define PIPE_READ(res,data,len)\
	_PIPE_READ(osif_hw2sw, osif_sw2hw, __run_id, res, data, len)

static inline int _PIPE_READ(hls::stream<uint32> &osif_hw2sw,
                             hls::stream<uint32> &osif_sw2hw,
                             uint8 run_id,
                             uint8 res, uint32 *data, int len) {
	#pragma HLS inline

	stream_write(osif_hw2sw, CONCAT_CTRL(run_id, res, 0x0002));
	stream_write(osif_hw2sw, OSIF_CMD_PIPE_READ);
	stream_write(osif_hw2sw, len);

	stream_read(osif_sw2hw);
	uint16 min_len = apint_get_range(stream_read(osif_sw2hw), 17, 2);
	stream_read(osif_sw2hw);
	for (int i = 0; i < min_len; i++) {
		data[i] = stream_read(osif_sw2hw);
	}

	return len << 2;
}

/*
 * Gets the pointer to the initialization data of the ReconOS thread
 * specified by reconos_hwt_setinitdata.
 */
#define GET_INIT_DATA()(\
	stream_write(osif_hw2sw, CONCAT_CTRL(__run_id, 0xFF, 0x0001)),\
	stream_write(osif_hw2sw, OSIF_CMD_THREAD_GET_INIT_DATA),\
	stream_read(osif_sw2hw),\
	stream_read(osif_sw2hw))

/*
 * Reads several words from the main memory into the local ram. Therefore,
 * divides a large request into smaller ones of length at most
 * RECONOS_MEMIF_CHUNK_BYTES and splits request at page borders to guarantee
 * correct address translation.
 *
 *   src - start address to read from the main memory
 *   dst - array to write data into
 *   len - number of bytes to transmit (bytes)
 *   
 */
#define MEM_READ(src,dst,len){\
	uint32 __len, __rem;\
	uint32 __addr = (src), __i = 0;\
	for (__rem = (len); __rem > 0;) {\
		uint32 __to_border = RECONOS_MEMIF_CHUNK_BYTES - (__addr & RECONOS_MEMIF_CHUNK_MASK);\
		uint32 __to_rem = __rem;\
		if (__to_rem < __to_border)\
			__len = __to_rem;\
		else\
			__len = __to_border;\
		\
		stream_write(memif_hwt2mem, RECONOS_MEMIF_CTRL(RECONOS_MEMIF_CMD_READ, __len));\
		stream_write(memif_hwt2mem, __addr);\
		\
		for (; __len > 0; __len -= 4) {\
			(dst)[__i++] = stream_read(memif_mem2hwt);\
			__addr += 4;\
			__rem -= 4;\
		}\
	}}

/*
 * Writes several words from the local ram into main memory. Therefore,
 * divides a large request into smaller ones of length at most
 * RECONOS_MEMIF_CHUNK_BYTES and splits request at page borders to guarantee
 * correct address translation.
 *
 *   src - array to read data from
 *   dst - start address to read from the main memory
 *   len - number of bytes to transmit (bytes)
 */
#define MEM_WRITE(src,dst,len){\
	uint32 __len, __rem;\
	uint32 __addr = (dst), __i = 0;\
	for (__rem = (len); __rem > 0;) {\
		uint32 __to_border = RECONOS_MEMIF_CHUNK_BYTES - (__addr & RECONOS_MEMIF_CHUNK_MASK);\
		uint32 __to_rem = __rem;\
		if (__to_rem < __to_border)\
			__len = __to_rem;\
		else\
			__len = __to_border;\
		\
		stream_write(memif_hwt2mem, RECONOS_MEMIF_CTRL(RECONOS_MEMIF_CMD_WRITE, __len));\
		stream_write(memif_hwt2mem, __addr);\
		\
		for (; __len > 0; __len -= 4) {\
			stream_write(memif_hwt2mem, (src)[__i++]);\
			__addr += 4;\
			__rem -= 4;\
		}\
	}}

#endif /* RECONOS_CALLS_H */