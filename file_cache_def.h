#include<pthread.h>

#ifndef QUIZ

struct file_cache;
struct file_cache_entry;
struct queue_fc;
struct file_data_header;
struct file_data;
struct free_pool_data;

//struct file_cache* file_cache_head=NULL;
struct file_cache{

      pthread_mutex_t fc_mutex_lock; //this is required only during queue operations and accounting
      unsigned int max_entries; //max entries possible
      int evict_time;           // time after which more vlunerable for eviction
      int water_mark;           // keep the free entries at 10% of max_entries, trigger a bg thread to evict when below this water mark

      struct queue_fc* pinned_dirty;  // queue of pinned dirty files
      struct queue_fc* pinned_clean;   // queue of pinned read files

      struct queue_fc* freeq;           // queue of free buffers

      pthread_cond_t cv;               // this conditional variable is required when there are no free files and need to wait for free up

      struct queue_fc* unpinned_clean; // queue of unpinned clean files
      struct queue_fc* unpinned_dirty;  // queue of unpinned dirty files

      unsigned int num_files_pinned_dirty; // account pinned dirty
      unsigned int num_files_pinned_clean; // account pinned clean
      unsigned int num_files_free;          // account free
      unsigned int num_files_unpinned_clean; // account unpinned clean
      unsigned int num_files_unpinned_dirty; // account unpinned dirty

      struct file_cache_entry* file_cache_table;

      struct file_cache* next_file_cache; // pointer pointing to thalloc_data_headere next file_cache
};

struct file_cache_entry{

       pthread_rwlock_t rwlock; // implement rw locks for every cache entry
       short int pinned_clean; // whether the entry is pinned clean
       short int pinned_dirty; // whether the entry is pinned dirty
       short int unpinned_dirty; // whether the entry is unpinned dirty
       short int unpinned_clean; // whether the entry is unpinned clean
       short int valid;           // whether the entry is free; 1-free 0-alloc

       short int is_buffer_allocated; //don't allocate the buffer immediately
       struct file_data_header* alloc_data_header;   // pointer to the allocated data header
       char* alloc_data;          // pointer to the data itself

       unsigned int pinned_count; // number of pins
       unsigned int last_accessed; //maintain the time stamp for the last accessed time

       struct file_cache_entry* pinned_dirty_next;  // maintain a LRU queue of pinned dirty buffers
       struct file_cache_entry* pinned_clean_next; // maintain a LRU queue of pinned clean buffers
       struct file_cache_entry* unpinned_dirty_next; // maintain a LRU queue of unpinned dirty buffers
       struct file_cache_entry* unpinned_clean_next; // maintain a LRU queue of unpinned clean buffers
       struct file_cache_entry* free_file_next; // maintain a queue of free files

       struct file_cache* file_cache_back_ptr;
};


struct queue_fc{
       struct file_cache_entry* head;
       struct file_cache_entry* tail;
};

struct file_data_header{
      short int used; // is the file used or not
      struct file_data* next_free; // next free buffer in the pool
      struct file_cache_entry* file_entry_ptr; // pointer back to the file_cache_entry
      struct file_cache* file_cache_ptr;    // pointer back to the file_cache_itself
};


struct file_data{
       struct file_data_header file_data_head_info; // header information about the file
       char data[10*1024];      // the actual data itself
       int magic; // detect buffer oveflow
};

struct free_pool_data{     // stats for free pool data
       int num_free_files;    // num of free files
       struct file_data* head;   // queue for free pool
       struct file_data* tail;
};

//struct free_pool_data* free_pool_table=NULL;
struct file_cache_entry* get_free_entry(struct file_cache *);
struct file_cache* initialize_file_cache(int num_max_entries);
void insert_into_global_fc_queue(struct file_cache* file_cache_instance);
struct file_cache* file_cache_construct(int);
void evict_from_cache(struct file_cache*);
#endif
#define QUIZ
//////////////////////////////////////////////////////////////////////////////////
//                                                                              //
//       --------------          --------------------            -------------- //
//       |file cache  | ------>  |file cache entries| ------->   | file header| //
//       |            | <------  |                  | <-------   | file data  | //
//       --------------          --------------------            -------------- //
//                                                                              //
//////////////////////////////////////////////////////////////////////////////////
