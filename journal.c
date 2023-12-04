#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "journal.h"

#define NAME(x) #x // Return the name of a variable

/**
 * @brief Asyncronous Request Buffer
 * Name, Buffer[MAX_BUFFER], Write Position,
 * Read Position, Mutex Lock, Empty Signal, 
 * Full Signal
 * 
 */
typedef struct {

	// Buffer Queue Data
	char* name;
	struct write_request buffer[BUFFER_SIZE];
	int write;
	int read;
	pthread_mutex_t mutex;
	pthread_cond_t empty;
	pthread_cond_t full;

	// Write Requestion Conditions
	/// Metadata (Buffer_Request)
	sem_t* issue_write_data_cond;
	sem_t* issue_journal_txb_cond;
	sem_t* issue_journal_bitmap_cond;
	sem_t* issue_journal_inode_cond;
	/// Commit Data (Buffer_Metadata)
	sem_t* issue_journal_txe_cond;
	/// Completion Data (Buffer_Commit)
	sem_t* issue_write_bitmap_cond;
	sem_t* issue_write_inode_cond;

} buffer_t; 

// 3 Buffers for 3 phases of journal
//	buffer_request -> buffer_metadata -> buffer_commit
buffer_t buffer_request, buffer_metadata, buffer_commit;

/**
 * @brief Initialize a Request Buffer
 * 
 * @param buffer Buffer
 */
void init_buffer(buffer_t* buffer, char* name) {
	buffer->name = name;
	buffer->write = 0;
	buffer->read = 0;
	pthread_mutex_init(&buffer->mutex, NULL);
	pthread_cond_init(&buffer->empty, NULL);
	pthread_cond_init(&buffer->full, NULL);

	// Write Requestion Conditions
	/// Metadata (Buffer_Request)
	buffer->issue_write_data_cond = sem_open("/issue_write_data_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_write_data_cond");
	buffer->issue_journal_txb_cond = sem_open("/issue_journal_txb_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_journal_txb_cond");
	buffer->issue_journal_bitmap_cond = sem_open("/issue_journal_bitmap_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_journal_bitmap_cond");
	buffer->issue_journal_inode_cond = sem_open("/issue_journal_inode_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_journal_inode_cond");
	/// Commit Data (Buffer_Metadata)
	buffer->issue_journal_txe_cond = sem_open("/issue_journal_txe_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_journal_txe_cond");
	/// Completion Data (Buffer_Commit)
	buffer->issue_write_bitmap_cond = sem_open("/issue_write_bitmap_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_write_bitmap_cond");
	buffer->issue_write_inode_cond = sem_open("/issue_write_inode_cond", O_CREAT, 0644, 0);
	sem_unlink("/issue_write_inode_cond");
}

/**
 * @brief Enqueue a request to the buffer
 * 
 * @param buffer Buffer
 * @param wr Write Request
 */
void enqueue(buffer_t* buffer, struct write_request* wr) {
	pthread_mutex_lock(&buffer->mutex);

	while((buffer->write + 1) % BUFFER_SIZE == buffer->read) {
		printf("Buffer %s is full.\n", buffer->name);
		pthread_cond_wait(&buffer->full, &buffer->mutex);
	}

	buffer->buffer[buffer->write] = *wr;
	buffer->write = (buffer->write + 1) % BUFFER_SIZE;
	pthread_cond_signal(&buffer->empty);

	pthread_mutex_unlock(&buffer->mutex);
}

/**
 * @brief Dequeue a Request from the Buffer
 * 
 * @param buffer Buffer
 * @param wr Write Request
 */
void dequeue(buffer_t* buffer, struct write_request* wr) {
	pthread_mutex_lock(&buffer->mutex);

	while(buffer->read == buffer->write) {
		printf("Buffer %s is empty.\n", buffer->name);
		pthread_cond_wait(&buffer->empty, &buffer->mutex);
	}

	*wr = buffer->buffer[buffer->read];
	buffer->read = (buffer->read + 1) % BUFFER_SIZE;
	pthread_cond_signal(&buffer->full);

	pthread_mutex_unlock(&buffer->mutex);
}

/**
 * @brief Journal Metadata
 * 
 * @param args (NULL)
 * @return void*
 */
void* journal_metadata(void* args) {

	/*
		A journal-metadata-write-thread takes a request out of the request buffer
		(whenever the buffer is not empty), calls the functions to write the data 
		and journal metadata (see example code above), waits for all issued writes
		to complete, and then enqueues the request in the next buffer.
	*/

	while(1) {

		// dequeue from previous buffer
		struct write_request wr;
		dequeue(&buffer_request, &wr);

		// write data and journal metadata
		printf("Request %d ", (wr.bitmap_idx-99)); issue_write_data(wr.data, wr.data_idx);
		printf("Request %d ", (wr.bitmap_idx-99)); issue_journal_txb();
		printf("Request %d ", (wr.bitmap_idx-99)); issue_journal_bitmap(wr.bitmap, wr.bitmap_idx);
		printf("Request %d ", (wr.bitmap_idx-99)); issue_journal_inode(wr.inode, wr.inode_idx);

		// wait for completion
		sem_wait(buffer_request.issue_write_data_cond);
		sem_wait(buffer_request.issue_journal_txb_cond);
		sem_wait(buffer_request.issue_journal_bitmap_cond);
		sem_wait(buffer_request.issue_journal_inode_cond);

		// enqueue in next buffer
		enqueue(&buffer_metadata, &wr);
	}
	pthread_exit(NULL);
}

/**
 * @brief Issues journal txe
 * 
 * @param args (NULL)
 * @return void* 
 */
void* journal_commit(void* args) {

	/*
		A journal-metadata-commit-write-thread takes a request out of the previous
		buffer, issues the journal txe (transaction end), waits for completion of 
		writing the txe block, and then enqueues the request in the next buffer.
	*/

	while(1) {

		// dequeue from previous buffer
		struct write_request wr;
		dequeue(&buffer_metadata, &wr);

		// commit transaction by writing txe
		printf("Request %d ", (wr.bitmap_idx-99)); issue_journal_txe();

		// wait for completion
		sem_wait(buffer_metadata.issue_journal_txe_cond);

		// enqueue in next buffer
		enqueue(&buffer_commit, &wr);
	}
	pthread_exit(NULL);
}

/**
 * @brief Completes writing metadata
 * 
 * @param args (NULL)
 * @return void* 
 */
void* journal_checkpoint(void* args) {

	/*
		The checkpoint-metadata-thread takes a request
		out of the previous buffer, issues writing the metadata, waits 
		for completion of writing the metadata, and then calls write_complete()
	*/

	while(1) {

		// dequeue from previous buffer
		struct write_request wr;
		dequeue(&buffer_commit, &wr);

		// checkpoint by writing metadata
		printf("Request %d ", (wr.bitmap_idx-99)); issue_write_bitmap(wr.bitmap, wr.bitmap_idx);
		printf("Request %d ", (wr.bitmap_idx-99)); issue_write_inode(wr.inode, wr.inode_idx);

		// wait for completion
		sem_wait(buffer_commit.issue_write_bitmap_cond);
		sem_wait(buffer_commit.issue_write_inode_cond);

		// tell the file system that the write is complete
		printf("Request %d ", (wr.bitmap_idx-99)); write_complete();
	}
	pthread_exit(NULL);
}

/* This function can be used to initialize the buffers and threads.
 */
void init_journal() {
	
	// Create buffers
	printf("Initializing Buffers\n");
	init_buffer(&buffer_request, NAME(buffer_request));
	init_buffer(&buffer_metadata, NAME(buffer_metadata));
	init_buffer(&buffer_commit, NAME(buffer_commit));

	// Create threads
	printf("Initializing Threads\n");
	pthread_t journal_metadata_thread, journal_commit_thread, journal_checkpoint_thread;
	pthread_create(&journal_metadata_thread, NULL, journal_metadata, NULL);
	pthread_create(&journal_commit_thread, NULL, journal_commit, NULL);
	pthread_create(&journal_checkpoint_thread, NULL, journal_checkpoint, NULL);
}

/* This function is called by the file system to request writing data to
 * persistent storage.
 * 
 * Thread safe implementation enqueues write requests to a buffer
 */
void request_write(struct write_request* wr) {

	// Enqueue request in request buffer
	enqueue(&buffer_request, wr);
}

/* This function is called by the block service when writing the txb block
 * to persistent storage is complete (e.g., it is physically written to disk).
 */
void journal_txb_complete() {
	sem_post(buffer_request.issue_journal_txb_cond);
	printf("journal txb complete\n");
}

void journal_bitmap_complete() {
	sem_post(buffer_request.issue_journal_bitmap_cond);
	printf("journal bitmap complete\n");
}

void journal_inode_complete() {
	sem_post(buffer_request.issue_journal_inode_cond);
	printf("journal inode complete\n");
}

void write_data_complete() {
	sem_post(buffer_request.issue_write_data_cond);
	printf("write data complete\n");
}

void journal_txe_complete() {
	sem_post(buffer_metadata.issue_journal_txe_cond);
	printf("journal txe complete\n");
}

void write_bitmap_complete() {
	sem_post(buffer_commit.issue_write_bitmap_cond);
	printf("write bitmap complete\n");
}

void write_inode_complete() {
	sem_post(buffer_commit.issue_write_inode_cond);
	printf("write inode complete\n");
}

