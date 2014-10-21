#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include"file_cache_def.h"
// mothod to initialize file cache

#define ERR_FREE_POOL -1
#define ERR_FREE_POOL_MEM -2
#define SUCCESS 0

extern struct free_pool_data* free_pool_table;
extern struct file_cache* file_cache_head;

// initialize the free pool files. preallocate the buffers, when the request comes in allocate the buffers from the pool
// this should be done before the service starts. Maximum files should be based on the memory constraints. The pool is basically
// preallocated. If time permits I would write functionality to expand this pool as well.

int initialize_free_pool_data(int maximum_files){
                        int i;
                        if(free_pool_table)
                                      return ERR_FREE_POOL; // free pool already initialized
                        free_pool_table = (struct free_pool_data*)malloc(sizeof(struct free_pool_data));
                        if (free_pool_table == NULL)
                                      return ERR_FREE_POOL_MEM; // no mem

                        free_pool_table->num_free_files = maximum_files;

                        free_pool_table->head = (struct file_data *)malloc(maximum_files * sizeof(struct file_data));
                        struct file_data* trav_ptr = free_pool_table->head;
                        char *fdata;
                        for(i=0;i< maximum_files; i++){
                                      trav_ptr->file_data_head_info.used = 0;
                                      trav_ptr->magic = 0xdeadbabe; // 4 bytes magic
                                      trav_ptr->file_data_head_info.file_entry_ptr = NULL;
                                      trav_ptr->file_data_head_info.file_cache_ptr = NULL;
                                      fdata = trav_ptr->data;
 				      // zero out the data
                                      memset((char *)fdata,0,10*1024);
                                      if(i == maximum_files-1){
                                                trav_ptr->file_data_head_info.next_free = NULL;
                                                free_pool_table->tail = trav_ptr;
                                                break;
                                      }
                                      trav_ptr->file_data_head_info.next_free = trav_ptr + 1;
                                      trav_ptr++;
                        }
                        return SUCCESS;
}

