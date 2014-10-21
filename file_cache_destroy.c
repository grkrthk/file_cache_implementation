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

void destroy_pinned_dirty(struct file_cache* cache){

          struct file_cache_entry* destroy_head;
          struct file_cache_entry* destroy_tail;
          char *data;
          // destroy pinned dirty files first
          destroy_head = cache->pinned_dirty->head;
          destroy_tail = cache->pinned_dirty->tail;
          if(destroy_head == destroy_tail){
                   // flush the data
                   data = ((struct file_data *)(destroy_head->alloc_data_header))->data;
                   flush_data_to_file(data);
                   // delete it from the queue
                   delete_queue(destroy_head, cache->pinned_dirty);
                   // unset the pinned dirty flag
                   destroy_head->pinned_dirty = 0;
                   // decrement the counter in the cache itself
                   cache->num_files_pinned_dirty--;
                   // insert the entry to free list
                   delete_queue(destroy_head, cache->freeq);
                   // incr the free counter
                   cache->num_files_free++;
                   // set the free flag to indicate the entry is free now
                   destroy_head->valid = 0;
                   return;
          }

          while(destroy_head!=destroy_tail){
                    data = ((struct file_data *)(destroy_head->alloc_data_header))->data;
                    flush_data_to_file(data);
                    delete_queue(destroy_head, cache->pinned_dirty);
                    destroy_head->pinned_dirty = 0;
                    cache->num_files_pinned_dirty--;
                    insert_queue(destroy_head, cache->freeq);
                    cache->num_files_free++;
                    destroy_head->valid = 0;

                    destroy_head=destroy_head->pinned_dirty_next;
          }
}

void destroy_unpinned_dirty(struct file_cache* cache){

          struct file_cache_entry* destroy_head;
          struct file_cache_entry* destroy_tail;
          char *data;
          // destroy pinned dirty files first
          destroy_head = cache->unpinned_dirty->head;
          destroy_tail = cache->unpinned_dirty->tail;
          if(destroy_head == destroy_tail){
                   // flush the data
                   data = ((struct file_data *)(destroy_head->alloc_data_header))->data;
                   flush_data_to_file(data);
                   // delete it from the queue
                   delete_queue(destroy_head, cache->unpinned_dirty);
                   // unset the unpinned dirty flag
                   destroy_head->unpinned_dirty = 0;
                   // decrement the counter in the cache itself
                   cache->num_files_unpinned_dirty--;
                   // insert the entry to free list
                   insert_queue(destroy_head, cache->freeq);
                   // incr the free counter
                   cache->num_files_free++;
                   // set the free flag to indicate the entry is free now
                   destroy_head->valid = 0;
                   return;
          }

          while(destroy_head!=destroy_tail){
                    data = ((struct file_data *)(destroy_head->alloc_data_header))->data;
                    flush_data_to_file(data);
                    delete_queue(destroy_head, cache->unpinned_dirty);
                    destroy_head->unpinned_dirty = 0;
                    cache->num_files_unpinned_dirty--;
                    insert_queue(destroy_head, cache->freeq);
                    cache->num_files_free++;
                    destroy_head->valid = 0;

                    destroy_head=destroy_head->unpinned_dirty_next;
          }
}
void file_cache_destroy(struct file_cache *cache){
          pthread_mutex_lock(&(cache->fc_mutex_lock));
          destroy_pinned_dirty(cache);
          destroy_unpinned_dirty(cache);
          pthread_mutex_lock(&(cache->fc_mutex_lock));
}

