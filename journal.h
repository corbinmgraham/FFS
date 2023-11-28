#ifndef JOURNAL_H
#define JOURNAL_H

#define BUFFER_SIZE 8 

struct write_request {
    char* bitmap;
    char* inode;
    char* data;
    int bitmap_idx;
    int inode_idx;
    int data_idx;
};


// Implemented in journaling layer.
// YOU IMPLEMENT THESE FUNCTIONS
void init_journal();
void request_write(struct write_request* wr);
void write_complete();
void journal_txb_complete();
void journal_bitmap_complete();
void journal_inode_complete();
void write_data_complete();
void journal_txe_complete();
void write_bitmap_complete();
void write_inode_complete();


// Implemented in block layer.
// YOU USE THESE FUNCTIONS
void issue_journal_txb();
void issue_journal_bitmap(char* bitmap, int idx);
void issue_journal_inode(char* inode, int idx);
void issue_write_data(char* data, int idx);
void issue_journal_txe();
void issue_write_bitmap(char* bitmap, int idx);
void issue_write_inode(char* inode, int idx);

#endif // JOURNAL_H
