#include<stdio.h>
#include<stdlib.h>
#include"file_cache_def.h"
#include"file_cache_decl.h"

extern struct free_pool_data* free_pool_table;
extern struct file_cache* file_cache_head;

void main(){
        struct file_cache* my_inst = file_cache_construct(10);
        struct file_cache* next_inst = file_cache_construct(20);
        int stat = initialize_free_pool_data(1000);
        //file_cache_pin_files();

}
