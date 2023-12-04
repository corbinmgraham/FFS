#include <stdio.h>
#include <unistd.h>
#include "journal.h"

#define NUM_TESTS 2
struct write_request test_requests[NUM_TESTS];

void write_complete() {
	printf("test write complete\n");
}

int main(int argc, char *argv[]) {
	init_journal();
	for (int i=0; i<NUM_TESTS; i++) {
		test_requests[i].bitmap = "x";
		test_requests[i].inode = "y";
		test_requests[i].data = "z";
		test_requests[i].bitmap_idx = 100 + i;
		test_requests[i].inode_idx = 200 + i;
		test_requests[i].data_idx = 300 + i;

		printf("requesting test write\n");
		request_write(&test_requests[i]);
	}

	// give the tests time to complete before exiting
	sleep(10);

	return 0;
}
