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

static uint32_t heap_4k_start;
static uint32_t heap_256b_start;

/*
 * _alloc_start points to the actual start address of heap pool
 * _alloc_end points to the actual end address of heap pool
 * _num_pages holds the actual max number of pages we can allocate.
 */
static uint32_t _alloc_start_256b = 0;
static uint32_t _alloc_end_256b = 0;
static uint32_t _num_pages_256b = 0;
static uint32_t _alloc_start_4k = 0;
static uint32_t _alloc_end_4k = 0;
static uint32_t _num_pages_4k = 0;

#define PAGE_SIZE_256B 256
#define PAGE_ORDER_256B 8

#define PAGE_SIZE_4K 4096
#define PAGE_ORDER_4K 12

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

static inline void _set_flag(struct Page *page, uint8_t flags)
{
    page->flags |= flags;
}

static inline int _is_last(struct Page *page)
{
    return (page->flags & PAGE_LAST);
}

static inline uint32_t _align_page(uint32_t address, uint8_t page_order)
{
    uint32_t order = (1 << page_order) - 1;
    return (address + order) & ~(order);
}

void page_init()
{
    printf("HEAP_START: 0x%x, HEAP_SIZE: 0x%x\n", HEAP_START, HEAP_SIZE);
    
    /* 
	 * We reserved 120 Page (120 x 256) to hold the Page structures.
	 */
    uint32_t first_heap_size = HEAP_SIZE / 2;
    _num_pages_256b = (first_heap_size / PAGE_SIZE_256B) - 120;
    heap_256b_start = HEAP_START;
    struct Page *page = (struct Page *) (HEAP_START);
    for (int i = 0; i < _num_pages_256b; i++) {
        _clear(page++);
    }
    _alloc_start_256b =_align_page(HEAP_START + 120 * PAGE_SIZE_256B, PAGE_ORDER_256B);
    _alloc_end_256b = _alloc_start_256b + (PAGE_SIZE_256B * _num_pages_256b);
    

    /* 
	 * We reserved 8 Page (8 x 4096) to hold the Page structures.
	 */
    uint32_t sec_heap_size = HEAP_SIZE - first_heap_size;
    _num_pages_4k = (sec_heap_size / PAGE_SIZE_4K) - 8;
    heap_4k_start = (HEAP_START + first_heap_size);
    page = (struct Page *) (HEAP_START + first_heap_size);
    for (int i = 0; i < _num_pages_4k; i++) {
        _clear(page++);
    }
    _alloc_start_4k = _align_page((HEAP_START + first_heap_size) + 8 * PAGE_SIZE_4K, PAGE_ORDER_4K);
    _alloc_end_4k = _alloc_start_4k + (PAGE_SIZE_4K * _num_pages_4k);
    

    printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
	printf("HEAP:\n");
    printf("\t_alloc_start_256b = %x, _alloc_end_256b = %x, num of pages = %d\n", _alloc_start_256b, _alloc_end_256b, _num_pages_256b);
    printf("\t_alloc_start_4k = %x, _alloc_end_4k = %x, num of pages = %d\n", _alloc_start_4k, _alloc_end_4k, _num_pages_4k);
}

/*
 * Allocate a memory block which is composed of contiguous physical pages
 * - npages: the number of PAGE_SIZE pages to allocate
 */
void *page_alloc(int npages, uint32_t n_pages_type)
{
    int found = 0;
    struct Page *page_i = (struct Page *)HEAP_START;
    if (n_pages_type == _num_pages_4k) {
        page_i = (struct Page *) heap_4k_start;
    } else {
        page_i = (struct Page *) heap_256b_start;
    }

    for (int i = 0; i <= (n_pages_type - npages); i++) {
        if (_is_free(page_i)) {
            found = 1;
            /* 
			 * meet a free page, continue to check if following
			 * (npages - 1) pages are also unallocated.
			 */
            struct Page *page_j = page_i + 1;
            for (int j = i + 1; j < (i + npages); j++) {
                if (!_is_free(page_j)) {
                    found = 0;
                    break;
                }
                page_j++;
            }
            /*
			 * get a memory block which is good enough for us,
			 * take housekeeping, then return the actual start
			 * address of the first page of this memory block
			 */
            if (found) {
                struct Page *page_k = page_i;
                for (int k = i; k < (i + npages); k++) {
                    _set_flag(page_k, PAGE_TAKEN);
                    page_k++;
                }
                page_k--;
                _set_flag(page_k, PAGE_LAST);

                if (n_pages_type == _num_pages_4k) {
                    return (void *) (_alloc_start_4k + i * PAGE_SIZE_4K);
                } else {
                    return (void *) (_alloc_start_256b + i * PAGE_SIZE_256B);
                }
                
            }
        }
        page_i++;
    }
    return NULL;
}

/*
 * Free the memory block
 * - p: start address of the memory block
 */
void page_free(void *p, uint32_t n_pages_type)
{
    /*
	 * Assert (TBD) if p is invalid
	 */
    if (!p || (uint32_t)p >= _alloc_end_4k) {
        return;
    }
    /* get the first page descriptor of this memory block */
    struct Page *page;
    if (n_pages_type == _num_pages_4k) {
        page = (struct Page *) heap_4k_start;
        page += ((uint32_t)p - _alloc_start_4k) / PAGE_SIZE_4K;
    } else {
        page = (struct Page *) heap_256b_start;
        page += ((uint32_t)p - _alloc_start_256b) / PAGE_SIZE_256B;
    }
    
    /* loop and clear all the page descriptors of the memory block */
    while (!_is_free(page)) {
        if (_is_last(page)) {
            _clear(page);
            break;
        } else {
            _clear(page);
            page++;
        }
    }
}

void page_test()
{
    // 4K page test
    void *p = page_alloc(2, _num_pages_4k);
    printf("4k p = 0x%x\n", p);

    void *p2 = page_alloc(7, _num_pages_4k);
    printf("4k p2 = 0x%x\n", p2);
    page_free(p2, _num_pages_4k);

    void *p3 = page_alloc(4, _num_pages_4k);
    printf("4k p3 = 0x%x\n", p3);

    // 256B page test
    void *p4 = page_alloc(4, _num_pages_256b);
    printf("256b p4 = 0x%x\n", p4);

    void *p5 = page_alloc(7, _num_pages_256b);
    printf("256b p5 = 0x%x\n", p5);
    page_free(p5, _num_pages_256b);

    void *p6 = page_alloc(7, _num_pages_256b);
    printf("256b p6 = 0x%x\n", p6);
}