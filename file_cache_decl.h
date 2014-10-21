#include"file_cache_def.h"
// mothod to initialize file cache

#ifndef QUIZ_DEF

#define PINNED_DIRTY 0
#define PINNED_CLEAN 1
#define UNPINNED_DIRTY 2
#define UNPINNED_CLEAN 3
#define FREE 4

struct free_pool_data* free_pool_table;
struct file_cache* file_cache_head;
#endif
#define QUIZ_DEF
