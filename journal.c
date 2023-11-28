#include <stdio.h>
#include "journal.h"

/* This function can be used to initialize the buffers and threads.
 */
void init_journal() {
}

/* This function is called by the file system to request writing data to
 * persistent storage.
 *
 * This is the non-thread-safe solution to the problem. It issues all writes in
 * the correct order, but it doesn't wait for each phase to complete before
 * beginning the next. As a result the journal can become inconsistent and
 * unrecoverable.
 */
void request_write(struct write_request* wr) {
	// write data and journal metadata
	issue_write_data(wr->data, wr->data_idx);
	issue_journal_txb();
	issue_journal_bitmap(wr->bitmap, wr->bitmap_idx);
	issue_journal_inode(wr->inode, wr->inode_idx);
	// commit transaction by writing txe
	issue_journal_txe();
	// checkpoint by writing metadata
	issue_write_bitmap(wr->bitmap, wr->bitmap_idx);
	issue_write_inode(wr->inode, wr->inode_idx);
	// tell the file system that the write is complete
	write_complete();
}

/* This function is called by the block service when writing the txb block
 * to persistent storage is complete (e.g., it is physically written to disk).
 */
void journal_txb_complete() {
	printf("journal txb complete\n");
}

void journal_bitmap_complete() {
	printf("journal bitmap complete\n");
}

void journal_inode_complete() {
	printf("journal inode complete\n");
}

void write_data_complete() {
	printf("write data complete\n");
}

void journal_txe_complete() {
	printf("jounrnal txe complete\n");
}

void write_bitmap_complete() {
	printf("write bitmap complete\n");
}

void write_inode_complete() {
	printf("write inode complete\n");
}

