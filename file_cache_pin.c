#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"
#include<string.h>
// mothod to initialize file cache

#define ERR_FREE_POOL -1
#define ERR_FREE_POOL_MEM -2
#define SUCCESS 0

extern struct free_pool_data* free_pool_table;
extern struct file_cache* file_cache_head;

// file could either be :
// * in the free pool
// * or could be in the free list of the cache (in which case, 10kb buffer is already associate with it)

void pin_free_file(struct file_cache* cache, const char* file, struct file_data* filehptr){
           struct file_cache_entry* entry;
           if(filehptr->file_data_head_info.used == 0){
                        // set the used flag
                        filehptr->file_data_head_info.used = 1;
                        entry = get_free_entry(cache);
                        // set it as pinned clean
                        entry->pinned_clean = 1;
                        entry->valid        = 0;  //'valid' name is a misnomer, it's not free anymore
                        entry->is_buffer_allocated = 1;
                        entry->alloc_data_header = (struct file_data_header*) filehptr;
                        entry->alloc_data_header->file_entry_ptr = entry; // update the entry in the header
                        entry->alloc_data_header->file_cache_ptr = cache; // update the cache in the header
                        filehptr->file_data_head_info.used = 1;
                        entry->alloc_data = filehptr->data;
                        entry->pinned_count = 1;
                        entry->last_accessed = get_time_stamp();
                        cache->num_files_pinned_clean++;
                        insert_queue(entry, cache->pinned_clean);
           }
           // present in the cache itself as free
           if( (filehptr->file_data_head_info).used == 1 && ((filehptr->file_data_head_info).file_entry_ptr)->valid == 1){
                        entry = ((struct file_data_header *)filehptr)->file_entry_ptr;
                        entry->pinned_clean = 1;
                        entry->valid = 0; // it's not free anymore
                        entry->alloc_data_header = (struct file_data_header *)filehptr;
                        entry->alloc_data_header->file_entry_ptr = entry; // update the entry in the header
                        entry->alloc_data_header->file_cache_ptr = cache; // update the cache in the header
                        filehptr->file_data_head_info.used = 1;
                        remove_queue(entry, cache->freeq);
                        entry->alloc_data = filehptr->data;
                        entry->pinned_count = 1;
                        entry->last_accessed = get_time_stamp();
                        cache->num_files_pinned_clean++;
                        insert_queue(entry, cache->pinned_clean);
           }
}

void file_cache_pin_files(struct file_cache *cache,const char **files,int num_files){
           // first check if any of the files in the list are already pinned
           int i;
           struct file_data header_ptr;
           struct file_data* temp_ptr;
           // this mutex lock is required because multiple threads could pin the files at the same time
           pthread_mutex_lock(&(cache->fc_mutex_lock));
           int offset  = (char*)(&header_ptr.data) - (char *)(&header_ptr);
           int max_files = cache->max_entries;
           int total_pinned = 0;
           int total_files;
           struct file_cache_entry* entry;

           for(i=0;i<num_files;i++){
                     total_files = cache->num_files_pinned_dirty + cache->num_files_pinned_clean + cache->num_files_unpinned_clean + cache->num_files_unpinned_dirty;
                     temp_ptr = (struct file_data*)((char*)(files[i]) - offset);
                     entry =  ((temp_ptr->file_data_head_info).file_entry_ptr);
                     if (total_files == max_files){
                                      // if all are pinned dirty
                                      if(total_files == cache->num_files_pinned_dirty){
                                                          wait_for_free_up();
                                                          // conditional variable is implemented in wait_for_free_up
                                                          // check out stub.c
                                      }
                                      // not all are pinned dirty, there are some unpinned dirty and clean files,
                                      //   perform eviction and proceed
                                      else
                                           // this should not happen unless there was a severe memory pressure
                                           // there is a background thread which does eviction periodically based on watermarks.
                                           evict_from_cache(cache);
                     }
                     // if the file was in the free pool or in the cache as free file, then pin the file
                     if (temp_ptr->file_data_head_info.used == 0 || entry->valid){
                                        //doesn't belong to file_cache
                                        // use the free files to pin
                                        pin_free_file(cache, files[i], temp_ptr);
                                        continue;
                     }
                     else {
                              // handle the case if the file is already pinned.
                              // it doesn't get accounted for the total number of pins in the cache
                              // assumption: process creates only one cache but there could be
                              // many threads accessing the same cache
              	              if (entry->pinned_clean ||  entry->pinned_dirty) {
               	                                     // incr the pinned count in the file cache entry but not in the cache
                                                     //assert((((temp_ptr->file_data_head_info).file_entry_ptr)->pinned_count));
                	                             entry->pinned_count++;
                        	                     continue;
                              }

                              // handle the case if the file is unpinned clean but in the cache
                              if (entry->unpinned_clean ){
                                                    // incr the number of pins of the file
                                                    entry->pinned_count++;
                                                    // update the total pinned files in the file cache
                                                    cache->num_files_pinned_clean++;
                                                    // delete the file from the unpinned clean queue
                                                    delete_queue(entry, cache->unpinned_clean);
                                                    // unset the unpinned clean flag
                                                    entry->unpinned_clean = 0;
                                                    // insert the file_entry into the pinned clean queue
                                                    insert_queue(entry, cache->pinned_clean);
                                                    // set the pinned clean in the file entry
                                                    entry->pinned_clean = 1;
                                                    continue;
                              }

                              // handle the case if the file is unpinned dirty but in the cache
                              // assumption: flush it first and then pin it

                              if (entry->unpinned_dirty ){
                                                    flush_data_to_file(temp_ptr->data);
                                                    memset(temp_ptr->data,0,10*1024);
                                                    // decr the total pinned dirty count
                                                    cache->num_files_pinned_dirty--;
                                                    // unset the pinned dirty for the entry
                                                    entry->pinned_dirty = 0;
                                                    // delete the file from the pinned dirty queue
                                                    // delete operation would have been simpler had I used doubly linked list
                                                    // if time permits will change it
                                                    delete_queue(entry, cache->pinned_dirty);
                                                    // incr  the total pinned clean count
                                                    cache->num_files_pinned_clean++;
                                                    // set the pinned clean flag of the entry and insert into the pinned clean queue
                                                    insert_queue(entry, cache->pinned_clean);
                                                    entry->pinned_clean = 1;
                                                    continue;
                              }
                     }
           }
           pthread_mutex_unlock(&(cache->fc_mutex_lock));
}

