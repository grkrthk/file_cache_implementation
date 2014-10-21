#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"

#define queue_next g1##g2

// dummy function which just flushes the data to the file
void flush_data_to_file(char *data){
         FILE *fptr;
         char c;
         fptr=fopen("./flush_file.txt","a");
         int i=0;
         while(i!= (10*1024)){
                   fputc(c,fptr);
                   i++;
         }
}

// Delete a node from LRU queue
void delete_queue(struct file_cache_entry* entry, struct queue_fc* queue, int type){

         struct file_cache_entry* head = queue->head;
         struct file_cache_entry* tail = queue->tail;
         struct file_cache_entry* first;
         struct file_cache_entry* second;
         if (type == PINNED_DIRTY){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){                         
                              queue->head = queue->head->pinned_dirty_next;                              
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL; 
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->pinned_dirty_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->pinned_dirty_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->pinned_dirty_next = second->pinned_dirty_next;
                              }
                              first = second;
                              second = second->pinned_dirty_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      } 
         }                          
         
         
         if (type == UNPINNED_DIRTY){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->unpinned_dirty_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->unpinned_dirty_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->unpinned_dirty_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->unpinned_dirty_next = second->unpinned_dirty_next;
                              }
                              first = second;
                              second = second->unpinned_dirty_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }              

         if (type == UNPINNED_CLEAN){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->unpinned_clean_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->unpinned_clean_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->unpinned_clean_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->unpinned_clean_next = second->unpinned_clean_next;
                              }
                              first = second;
                              second = second->unpinned_clean_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }
 
         if (type == PINNED_CLEAN){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->pinned_clean_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->pinned_clean_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->pinned_clean_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->pinned_clean_next = second->pinned_clean_next;
                              }
                              first = second;
                              second = second->pinned_clean_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }

         if (type == FREE){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->free_file_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->free_file_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->free_file_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->free_file_next = second->free_file_next;
                              }
                              first = second;
                              second = second->pinned_clean_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }

}

void insert_queue(struct file_cache_entry* entry, struct queue_fc* queue, int type){

         struct file_cache_entry* head = queue->head;
         struct file_cache_entry* tail = queue->tail;
 
         if (type == PINNED_DIRTY){
                     // if there are no elements
                     if (head == NULL && tail == NULL){
                               queue->tail = entry;
                               return;
                     } 
                     // if there is one elements
                     if (head == tail){
                               queue->tail = entry;
                               return;
                     }
                     // in all other cases : move the tail to the last inserted elements                     
                     queue->tail->pinned_dirty_next = entry;
                     queue->tail = entry;                       
         }

         if (type == UNPINNED_DIRTY){
                     // if there are no elements
                     if (head == NULL && tail == NULL){
                               queue->tail = entry;
                               return;
                     }
                     // if there is one elements
                     if (head == tail){
                               queue->tail = entry;
                               return;
                     }
                     // in all other cases : move the tail to the last inserted elements                     
                     queue->tail->unpinned_dirty_next = entry;
                     queue->tail = entry;
         }
 
         if (type == PINNED_CLEAN){
                     // if there are no elements
                     if (head == NULL && tail == NULL){
                               queue->tail = entry;
                               return;
                     }
                     // if there is one elements
                     if (head == tail){
                               queue->tail = entry;
                               return;
                     }
                     // in all other cases : move the tail to the last inserted elements                     
                     queue->tail->pinned_clean_next = entry;
                     queue->tail = entry;
         }
         
         if (type == UNPINNED_CLEAN){
                     // if there are no elements
                     if (head == NULL && tail == NULL){
                               queue->tail = entry;
                               return;
                     }
                     // if there is one elements
                     if (head == tail){
                               queue->tail = entry;
                               return;
                     }
                     // in all other cases : move the tail to the last inserted elements                     
                     queue->tail->unpinned_clean_next = entry;
                     queue->tail = entry;
         }

         if (type == FREE){
                     // if there are no elements
                     if (head == NULL && tail == NULL){
                               queue->tail = entry;
                               return;
                     }
                     // if there is one elements
                     if (head == tail){
                               queue->tail = entry;
                               return;
                     }
                     // in all other cases : move the tail to the last inserted elements                     
                     queue->tail->free_file_next = entry;
                     queue->tail = entry;
         }
 
}


void wait_for_free_up(struct file_cache *cache,const char **files,int num_files){
              pthread_cond_wait(&cache->cv, &cache->fc_mutex_lock);
}

// not implementing this ... ran out of time...
struct file_cache_entry* get_free_entry(struct file_cache *cache){
}

int get_time_stamp(){
         
          struct timeval start;
          gettimeofday(&start,NULL);
          return start.tv_sec;
}

// same as delete queue. I am running out of time, have to prep for my mid terms!
void remove_queue(struct file_cache_entry* entry, struct queue_fc* queue, int type){


         struct file_cache_entry* head = queue->head;
         struct file_cache_entry* tail = queue->tail;
         struct file_cache_entry* first;
         struct file_cache_entry* second;
         if (type == PINNED_DIRTY){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){                         
                              queue->head = queue->head->pinned_dirty_next;                              
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL; 
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->pinned_dirty_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->pinned_dirty_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->pinned_dirty_next = second->pinned_dirty_next;
                              }
                              first = second;
                              second = second->pinned_dirty_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      } 
         }                          
         
         
         if (type == UNPINNED_DIRTY){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->unpinned_dirty_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->unpinned_dirty_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->unpinned_dirty_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->unpinned_dirty_next = second->unpinned_dirty_next;
                              }
                              first = second;
                              second = second->unpinned_dirty_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }              

         if (type == UNPINNED_CLEAN){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->unpinned_clean_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->unpinned_clean_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->unpinned_clean_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->unpinned_clean_next = second->unpinned_clean_next;
                              }
                              first = second;
                              second = second->unpinned_clean_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }
 
         if (type == PINNED_CLEAN){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->pinned_clean_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->pinned_clean_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->pinned_clean_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->pinned_clean_next = second->pinned_clean_next;
                              }
                              first = second;
                              second = second->pinned_clean_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }

         if (type == FREE){
                      // every time we remove, we don't have to free. We just have to set the pointers
                      // if the entry is at the head
                      if (entry == queue->head){
                              queue->head = queue->head->free_file_next;
                              return;
                      }
                      // if head and tail are equal and head is the element that needs to be removed
                      if (head == tail && entry == head){
                              queue->head = NULL;
                              queue->tail = NULL;
                              return;
                      }
                      // if there are 2 elements and the element to be removed is tail
                      if (head->free_file_next == tail && entry == tail){
                              queue->tail = NULL;
                              return;
                      }
                      // now a general case
                      first = head;
                      second = head->free_file_next;
                      while(second!=tail){
                              if (second == entry){
                                             first->free_file_next = second->free_file_next;
                              }
                              first = second;
                              second = second->pinned_clean_next;
                      }
                      // if tail is the entry to be removed then reset the tail
                      if(second == tail && entry == tail){
                              queue->tail = first;
                      }
         }


}
