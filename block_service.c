#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include "journal.h"

int test_index;

/**
 * @brief Sleeper Thread
 * Sleeps before calling complete for any function passed
 * 
 * @param args (void(*) (void)) Function Pointer
 * @return void*
 */
void* sleep_thread(void* args) {

	// Conver the complete function from thread
	void (*complete_func)(void) = (void(*) (void)) args;

	sleep(rand() % 2);
	printf("Request %d ", (test_index % 10)+1);
	complete_func();

	pthread_exit(NULL);
}

void issue_journal_txb() {
	printf("issue journal txb\n");
	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)journal_txb_complete);
	// journal_txb_complete();
}

void issue_journal_bitmap(char* bitmap, int idx) {
	test_index = idx;
	printf("issue journal bitmap %s at %d\n", bitmap, idx);
	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)journal_bitmap_complete);
	// journal_bitmap_complete();
}

void issue_journal_inode(char* inode, int idx) {
	test_index = idx;
	printf("issue journal inode %s at %d\n", inode, idx);
	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)journal_inode_complete);
	// journal_inode_complete();
}

void issue_write_data(char* data, int idx) {
	test_index = idx;
	printf("issue write data %s at %d\n", data, idx);
	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)write_data_complete);
	// write_data_complete();
}

void issue_journal_txe() {
	printf("issue journal txe\n");

	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)journal_txe_complete);
	// journal_txe_complete();
}

void issue_write_bitmap(char* bitmap, int idx) {
	test_index = idx;
	printf("issue write bitmap %s at %d\n", bitmap, idx);
	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)write_bitmap_complete);
	// write_bitmap_complete();
}

void issue_write_inode(char* inode, int idx) {
	test_index = idx;
	printf("issue write inode %s at %d\n", inode, idx);
	pthread_t test;
	pthread_create(&test, NULL, sleep_thread, (void*)write_inode_complete);
	// write_inode_complete();
}