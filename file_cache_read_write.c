#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"

extern struct free_pool_data* free_pool_table;
extern struct file_cache* file_cache_head;

// there are 2 main cases to be handled here:
// * if the entry is pinned clean state
// * if the entry is pinned dirty state
//        - the behavior still should remain the same in both the cases
// another assumption
//              - after the reading is done client notifies us

const char *file_cache_file_data(struct file_cache *cache, const char *file){

           struct file_data header_ptr;
           struct file_data* temp_ptr;
           struct file_cache_entry* entry;
           int offset  = (char*)(&header_ptr.data) - (char *)(&header_ptr);
           temp_ptr = (struct file_data*)((char*)(file) - offset);
           entry = (temp_ptr->file_data_head_info).file_entry_ptr;
           // take the rw lock
           pthread_rwlock_rdlock(&(entry->rwlock));
           // assert( entry->pinned_clean || entry->pinned_dirty)
           if (entry->pinned_clean == 1){
                    entry->last_accessed = get_time_stamp();
                    insert_queue(entry, cache->pinned_clean); // insert in the end of LRU queue
                    return file;
           }
           if (entry->pinned_dirty == 1){
                    entry->last_accessed = get_time_stamp(); // insert in the LRU queue is not required for Dirty                   
                    return file;
           }
}

void file_cache_file_read_done(const char *file){

           struct file_data header_ptr;
           struct file_data* temp_ptr;
           struct file_cache_entry* entry;
           int offset  = (char*)(&header_ptr.data) - (char *)(&header_ptr);
           temp_ptr = (struct file_data*)((char*)(file) - offset);
           entry = (temp_ptr->file_data_head_info).file_entry_ptr;
           entry->last_accessed = get_time_stamp();
           insert_queue(entry, entry->file_cache_back_ptr->pinned_clean); // insert in the end of LRU queue
           pthread_rwlock_unlock(&(entry->rwlock));

}

// Write to a file has the following cases:
// * it could be in pinned_clean state
// * it could be in the pinned_dirty state
//           - flush it first
//           - give it back to the client marked as dirty
// assumption: client notifies when the write is done
char *file_cache_mutable_file_data(struct file_cache *cache, const char *file){

           struct file_data header_ptr;
           struct file_data* temp_ptr;
           struct file_cache_entry* entry;
           int offset  = (char*)(&header_ptr.data) - (char *)(&header_ptr);
           temp_ptr = (struct file_data*)((char*)(file) - offset);
           entry = (temp_ptr->file_data_head_info).file_entry_ptr;
           pthread_rwlock_wrlock(&(entry->rwlock));
           // assert( entry->pinned_clean || entry->pinned_dirty)
           if (entry->pinned_clean == 1){
                         entry->last_accessed = get_time_stamp();
                         entry->pinned_clean = 0;                  // unset the pinned clean flag
                         cache->num_files_pinned_clean--;          // reduce the count of pinned clean buffers in the cache
                         remove_queue(entry, cache->pinned_clean); // remove the entry from the pinned clean queue
                         entry->pinned_dirty = 1;                  // set the pinned dirty flag
                         insert_queue(entry, cache->pinned_dirty); // insert the entry into the pinned dirty queue (LRU)
                         cache->num_files_pinned_dirty++;          // increment the number of dirty pinned files
                         return (char *)file;                              // return the file
           }

           if (entry->pinned_dirty == 1){
                         entry->last_accessed = get_time_stamp();
                         flush_data_to_file(entry->alloc_data);    // flush the file to the disk
                         memset((char *)file, 0 ,10*1024);
                         return (char *)file;
           }
}

void file_cache_file_write_done(const char *file){

 	   struct file_data header_ptr;
           struct file_data* temp_ptr;
           struct file_cache_entry* entry;
           int offset  = (char*)(&header_ptr.data) - (char *)(&header_ptr);
           temp_ptr = (struct file_data*)((char*)(file) - offset);
           entry = (temp_ptr->file_data_head_info).file_entry_ptr;
           entry->last_accessed = get_time_stamp();
           //   if (  ((struct file_data*)entry->alloc_data_header)->magic != 0xdeadbabe ){
                                  // buffer overflow detected, might want to panic
           //                        exit(0);
           //   }
           // no need to insert into LRU queue for dirty as it is not evicted anyways.
           pthread_rwlock_unlock(&(entry->rwlock));
}
