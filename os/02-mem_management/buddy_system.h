#ifndef _BUDDY_SYSTEM_H_
#define _BUDDY_SYSTEM_H_

// #include <stdio.h>
// #include <stdlib.h>

#include "os.h"
#include "types.h"

#define KB (1 << 10)

struct buddy {
    uint32_t size;
    uint32_t longest[1];
};

struct buddy *buddy_new(unsigned num_of_fragments, uint32_t heap_start);
int buddy_alloc(struct buddy *self, uint32_t size);
void buddy_free(struct buddy *self, int offset);
void buddy_dump(struct buddy *self);
int buddy_size(struct buddy *self, int offset);

#endif /* _BUDDY_SYSTEM_H_ */
