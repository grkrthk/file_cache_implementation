#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"
// mothod to initialize file cache

#define ERR_FREE_POOL -1
#define ERR_FREE_POOL_MEM -2
#define SUCCESS 0

extern struct free_pool_data* free_pool_table;
extern struct file_cache* file_cache_head;

struct file_cache* file_cache_construct(int max_cache_entries){
               struct file_cache* file_cache_instance = initialize_file_cache(max_cache_entries);
               return file_cache_instance;
}

struct file_cache* initialize_file_cache(int num_max_entries){

          int ret;
          struct file_cache* file_cache_instance=NULL;
          file_cache_instance = (struct file_cache *)malloc(sizeof(struct file_cache));

          if (file_cache_instance == NULL) //insufficient memory
                         return NULL;

          // this would only be taken during accounting for a very short perid of time
          pthread_mutex_init(&(file_cache_instance->fc_mutex_lock), NULL);
          file_cache_instance->max_entries = num_max_entries;
          file_cache_instance->pinned_dirty = NULL;
          file_cache_instance->pinned_clean = NULL;
          file_cache_instance->evict_time = 2;
          file_cache_instance->water_mark = 0.1*num_max_entries;

          file_cache_instance->freeq = NULL;
          file_cache_instance->unpinned_clean = NULL;
          file_cache_instance->unpinned_dirty = NULL;

          file_cache_instance->num_files_pinned_dirty   = 0;
          file_cache_instance->num_files_pinned_clean   = 0;
          file_cache_instance->num_files_free           = 0;
          file_cache_instance->num_files_unpinned_clean = 0;
          file_cache_instance->num_files_unpinned_dirty = 0;

          pthread_cond_init (&(file_cache_instance->cv), NULL);

          ret = initialize_entries(num_max_entries, file_cache_instance);
          if (!ret)
                      return NULL;

          // update the global file_cache list

          insert_into_global_fc_queue(file_cache_instance);
          return file_cache_instance;
}

// global lock required
void insert_into_global_fc_queue(struct file_cache* file_cache_instance)
{
          if (file_cache_head == NULL){
                      file_cache_head = file_cache_instance;
                      file_cache_instance->next_file_cache = NULL;
          } else {
                      file_cache_instance->next_file_cache = file_cache_head;
                      file_cache_head = file_cache_instance;
          }
}

// initialize the file cache entries. Allocating the buffer is not done prematurely. Only the entries in the
// table are initialized. Buffers will be allocated based on the need.
int initialize_entries(int max_entries, struct file_cache* file_cache_inst){

          int i;
          struct file_cache_entry* trav_ptr;
          file_cache_inst->file_cache_table = (struct file_cache_entry *)malloc( max_entries * (sizeof(struct file_cache_entry)) );
          if (!file_cache_inst->file_cache_table)
                                            return 0;

          file_cache_inst->freeq = (struct queue_fc*)malloc(sizeof(struct queue_fc)); // initialize the freeq list
          file_cache_inst->freeq->head = file_cache_inst->file_cache_table; // head is initialized at the start of the table

          trav_ptr = file_cache_inst->file_cache_table;
          // initialize free entries and create a queue of free entries
          for(i=0;i<max_entries;i++){
                         trav_ptr->pinned_clean   = 0;
                         trav_ptr->pinned_dirty   = 0;
                         trav_ptr->unpinned_dirty = 0;
                         trav_ptr->unpinned_clean = 0;
                         trav_ptr->valid          = 0;
                         trav_ptr->is_buffer_allocated = 0;
                         trav_ptr->alloc_data_header = NULL;
                         trav_ptr->alloc_data = NULL;
                         trav_ptr->pinned_count = 0;
                         trav_ptr->last_accessed = 0;

                         trav_ptr->pinned_dirty_next = NULL;
                         trav_ptr->pinned_clean_next = NULL;
                         trav_ptr->unpinned_dirty_next = NULL;
                         trav_ptr->unpinned_clean_next = NULL;

                         // initialize rw lock on every entry
                         pthread_rwlock_init(&(trav_ptr->rwlock), NULL);

                         if(i == max_entries-1){
                                   trav_ptr->free_file_next = NULL;
                                   file_cache_inst->freeq->tail = trav_ptr;
                                   break;
                         }
                         trav_ptr->free_file_next = trav_ptr + 1; //create a link of free list
                         trav_ptr++;
          }
}

