#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"

int evict_time = 2; // if the file has stayed for more than 2 sec, good to evict (should be ideally based on heurestics)
     // is free buffer is less than or equal to 10% of MAX_FILES then evict to make room for free files


void evict_from_cache(struct file_cache* cache){
             // is lock required ?
             pthread_mutex_lock(&(cache->fc_mutex_lock));
             int free_files = cache->max_entries - (cache->num_files_pinned_dirty + cache->num_files_pinned_clean + cache->num_files_unpinned_clean + cache->num_files_unpinned_dirty);
             // assert (cache->num_files_free == free_files)
             struct file_cache_entry* entry;
             int cur_time;
             // first, if we are facing severe crunch, get free files to atleast threshold value, later we can evict based on eviction time too.
             // don't even check the timestamp first
             if (cache->num_files_free < cache->water_mark){

                       entry = cache->unpinned_clean->head;
                       // first evict from the unpinned clean
                       while (cache->num_files_free < cache->water_mark && cache->unpinned_clean->head!=cache->unpinned_clean->tail){
                                          entry->unpinned_clean = 0;
                                          entry->valid = 1;
                                          cache->num_files_unpinned_clean--;
                                          cache->num_files_free++;
                                          entry = entry->unpinned_clean_next;
                                          remove_queue(entry, cache->unpinned_clean);
 					  insert_queue(entry, cache->freeq);
                       }

                      // if it's still not good then go ahead and evict unpinned_dirty but don't forget to flush
                      if (cache->num_files_free < cache->water_mark && cache->unpinned_dirty->head!=cache->unpinned_dirty->tail){
                                      entry = cache->unpinned_dirty->head;
                                      while (cache->num_files_free < cache->water_mark){
                                                    entry->unpinned_dirty = 0;
                                                    entry->valid = 1;
                                                    cache->num_files_unpinned_dirty--;
                                                    cache->num_files_free++;
                                                    entry = entry->unpinned_clean_next;
                                                    flush_data_to_file(entry->alloc_data);
                                                    remove_queue(entry, cache->unpinned_dirty);
                                                    insert_queue(entry, cache->freeq);
                                      }

                      }

             }
             // now evict based on time stamp

                       // go through the unpinned clean queue first
                       // it is anyways LRU queue, the moment the time is leeser than evict time, stop.
             entry = cache->unpinned_clean->head;
             cur_time = get_time_stamp();
             while(entry != cache->unpinned_clean->tail){
                            if ((cur_time - entry->last_accessed) > evict_time){
                                       entry->unpinned_clean = 0;
                                       entry->valid = 1; // don't go by the varibale 'valid' it means free
                                       cache->num_files_unpinned_clean--;
                                       cache->num_files_free++;
                                       entry = entry->unpinned_clean_next; // increment the entry before the entry is removed
                                       remove_queue(entry, cache->unpinned_clean);
                                       insert_queue(entry, cache->freeq);
                            }
                            else break;
             }
             if (cache->num_files_free > 0)
                          pthread_cond_signal(&(cache->cv));
             pthread_mutex_unlock(&(cache->fc_mutex_lock));
}



