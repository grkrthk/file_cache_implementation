#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"

extern struct free_pool_data* free_pool_table;
extern struct file_cache* file_cache_head;

/*
 we are trying to unpin the pinned files. If for some reason files aren't pinned
 we ignore it for now.
 pinned files can be of two types
 * pinned clean
 * pinned dirty

*/
void file_cache_unpin_files(struct file_cache *cache, const char **files, int num_files){

           int i;
           struct file_data header_ptr;
           struct file_data* temp_ptr;
           pthread_mutex_lock(&(cache->fc_mutex_lock));
           int offset  = (char*)(&header_ptr.data) - (char *)(&header_ptr);
           int max_files = cache->max_entries;
           struct file_cache_entry* entry;

           for(i=0;i<num_files;i++){
                        temp_ptr = (struct file_data*)((char*)(files[i]) - offset);
                        entry = (temp_ptr->file_data_head_info).file_entry_ptr;
                        // assert(entry->unpinned_clean || entry->unpinned_dirty);
                        if (entry->pinned_clean == 1){
                                        entry->pinned_clean = 0;
                                        entry->unpinned_clean = 1;
                                        entry->last_accessed = get_time_stamp();
                                        remove_queue(cache->pinned_clean, entry);
                                        insert_queue(cache->unpinned_clean, entry);
                                        //account in the entry's pin count
                                        entry->pinned_count--;
                                        // account in the cache itself
                                        cache->num_files_pinned_clean--;
                                        cache->num_files_unpinned_clean++;
                                        entry->valid = 0; // more of an assert(entry->valid == 0)
                        }
                        else            // file can be pinned by more than 1 guy
                                        entry->pinned_count--;

                        if (entry->pinned_dirty == 1){
                                        entry->pinned_dirty = 0;
                                        entry->unpinned_dirty = 1;
                                        entry->last_accessed = get_time_stamp();
                                        remove_queue(cache->pinned_dirty, entry);
                                        insert_queue(cache->unpinned_dirty, entry);
                                        entry->pinned_count--;
                                        cache->num_files_pinned_dirty--;
                                        cache->num_files_unpinned_dirty++;
                                        entry->valid = 0; // more of an assert(entry->valid == 0)
                        }
                        else
                                        entry->pinned_count--;
           }
           pthread_mutex_unlock(&(cache->fc_mutex_lock));
}
