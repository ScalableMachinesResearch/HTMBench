#ifndef _PRIVATE_TREE_H_
#define _PRIVATE_TREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "private/writer.h"
#include "private/pages.h"

#ifdef RTM
#include "tm.h"
#else
#include <pthread.h>
#endif /* RTM */

#define BP__HEAD_SIZE  sizeof(uint64_t) * 4

#ifdef RTM
#define BP_TREE_PRIVATE         \
    BP_WRITER_PRIVATE           \
    bp__tree_head_t head;       \
    bp_compare_cb compare_cb;
#else
#define BP_TREE_PRIVATE         \
    BP_WRITER_PRIVATE           \
    pthread_rwlock_t rwlock;    \
    bp__tree_head_t head;       \
    bp_compare_cb compare_cb;
#endif

typedef struct bp__tree_head_s bp__tree_head_t;

int bp__init(bp_db_t *tree);
void bp__destroy(bp_db_t *tree);

int bp__tree_read_head(bp__writer_t *w, void *data);
int bp__tree_write_head(bp__writer_t *w, void *data);

int bp__default_compare_cb(const bp_key_t *a, const bp_key_t *b);
int bp__default_filter_cb(void *arg, const bp_key_t *key);


struct bp__tree_head_s {
    uint64_t offset;
    uint64_t config;
    uint64_t page_size;
    uint64_t hash;

    bp__page_t *page;
};

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _PRIVATE_TREE_H_ */
