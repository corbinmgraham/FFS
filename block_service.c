#include <stdio.h>
#include <unistd.h>
#include "journal.h"

void issue_journal_txb() {
	printf("issue journal txb\n");
	sleep(1);
	printf("TXB SLEPT\n");
	journal_txb_complete();
}

void issue_journal_bitmap(char* bitmap, int idx) {
	printf("issue journal bitmap %s at %d\n", bitmap, idx);
	sleep(1);
	printf("JOURNAL BITMAP SLEPT\n");
	journal_bitmap_complete();
}

void issue_journal_inode(char* inode, int idx) {
	printf("issue journal inode %s at %d\n", inode, idx);
	sleep(5);
	printf("INODE SLEPT\n");
	journal_inode_complete();
}

void issue_write_data(char* data, int idx) {
	printf("issue write data %s at %d\n", data, idx);
	sleep(1);
	printf("WRITE SLEPT\n");
	write_data_complete();
}

void issue_journal_txe() {
	printf("issue journal txe\n");
	sleep(1);
	printf("TXE SLEPT\n");
	journal_txe_complete();
}

void issue_write_bitmap(char* bitmap, int idx) {
	printf("issue write bitmap %s at %d\n", bitmap, idx);
	sleep(1);
	printf("WRITE BITMAP SLEPT\n");
	write_bitmap_complete();
}

void issue_write_inode(char* inode, int idx) {
	printf("issue write inode %s at %d\n", inode, idx);
	sleep(1);
	printf("ISSUE WRITE SLEPT\n");
	write_inode_complete();
}

