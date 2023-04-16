#include "os.h"

/*
 * Following global vars are defined in mem.S
 */
extern uint32_t TEXT_START;
extern uint32_t TEXT_END;
extern uint32_t DATA_START;
extern uint32_t DATA_END;
extern uint32_t RODATA_START;
extern uint32_t RODATA_END;
extern uint32_t BSS_START;
extern uint32_t BSS_END;
extern uint32_t HEAP_START;
extern uint32_t HEAP_SIZE;

/*
 * _alloc_start points to the actual start address of heap pool
 * _alloc_end points to the actual end address of heap pool
 * _num_pages holds the actual max number of pages we can allocate.
 */
static uint32_t _alloc_start = 0;
static uint32_t _alloc_end = 0;
static uint32_t _num_pages = 0;


#define PAGE_SIZE 4096
#define PAGE_ORDER 12

#define PAGE_TAKEN  (uint8_t)(1 << 0)
#define PAGE_LAST   (uint8_t)(1 << 1)

/*
 * Page Descriptor 
 * flags:
 * - bit 0: flag if this page is taken(allocated)
 * - bit 1: flag if this page is the last page of the memory block allocated
 */
struct Page {
    uint8_t flags;
};

static inline void _clear(struct Page *page)
{
    page->flags = 0;
}

static inline int _is_free(struct Page *page)
{
    return !(page->flags & PAGE_TAKEN);
}

static inline int set_flag(struct Page *page, uint8_t flags)
{
    page->flags |= flags;
}

static inline int _is_last(struct Page *page)
{
    return (page->flags & PAGE_LAST);
}

static inline uint32_t _align_page(uint32_t address)
{
    uint32_t order = (1 << PAGE_ORDER) - 1;
    return (address + order) & ~(order);
}

void page_init()
{
    /* 
	 * We reserved 8 Page (8 x 4096) to hold the Page structures.
	 * It should be enough to manage at most 128 MB (8 x 4096 x 4096) 
	 */
    _num_pages = (HEAP_SIZE / PAGE_SIZE) - 8;
    printf("HEAP_START = %x, HEAP_SIZE = %x, num of pages = %d\n", HEAP_START, HEAP_SIZE, _num_pages);

    struct Page *page = (struct Page *) HEAP_START;
    for (int i = 0; i < _num_pages; i++) {
        _clear(page++);
    }

    _alloc_start = _align_page(HEAP_START + 8 * PAGE_SIZE);
    _alloc_end = _alloc_start + (PAGE_SIZE * _num_pages);

    printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
	printf("HEAP:   0x%x -> 0x%x\n", _alloc_start, _alloc_end);
}