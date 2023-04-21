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


#define PAGE_SIZE_4K 4096
#define PAGE_ORDER_4K 12
#define PAGE_RESERVED_4K 80

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
    uint8_t page_id;
    struct Page* next;
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

static inline uint32_t _align_page(uint32_t address)
{
    uint32_t order = (1 << PAGE_ORDER_4K) - 1;
    return (address + order) & ~(order);
}

static inline void _init_page_linked_list(void *page_tabel_start, uint32_t num_page)
{
    struct Page *page = (struct Page *) page_tabel_start;

    printf("page :0x%x, page + 1: 0x%x\n", page, page + 1);
    for (int i = 0; i < num_page; i++) {
        _clear(page);
        page->page_id = i;
        page->next = page + 1;
        page = page->next;
    }
    page -= 1;
    page->next = NULL;
}

void page_init()
{
    /* 
	 * We reserved 8 Page (8 x 4096) to hold the Page structures.
	 * It should be enough to manage at most 128 MB (8 x 4096 x 4096) 
	 */
    uint32_t _total_num_page = (HEAP_SIZE / PAGE_SIZE_4K);
    printf("TOTAL number of page: %d\n", _total_num_page);

    _num_pages = (HEAP_SIZE / PAGE_SIZE_4K) - PAGE_RESERVED_4K;
    printf("HEAP_START = %x, HEAP_SIZE = %x, num of pages = %d\n", HEAP_START, HEAP_SIZE, _num_pages);

    _init_page_linked_list((void *) HEAP_START, _num_pages);

    _alloc_start = _align_page(HEAP_START + PAGE_RESERVED_4K * PAGE_SIZE_4K);
    _alloc_end = _alloc_start + (PAGE_SIZE_4K * _num_pages);

    printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
	printf("_alloc_start: 0x%x -> _alloc_end: 0x%x\n", _alloc_start, _alloc_end);
}

/*
 * Allocate a memory block which is composed of contiguous physical pages
 * - npages: the number of PAGE_SIZE pages to allocate
 */
void *page_alloc(int npages)
{
    int found = 0;
    struct Page *page_i = (struct Page *)HEAP_START;

    int idx = 0;
    while (page_i != NULL && idx <= (_num_pages - npages)) {
        if (_is_free(page_i)) {
            found = 1;
            struct Page *prev_page = page_i;;
            struct Page *page_j = page_i->next;

            // check continuous pages
            idx = npages - 1;
            while (page_j && idx--) {
                if (!_is_free(page_j) || page_j->page_id - 1 != prev_page->page_id) {
                    found = 0;
                    break;
                }
                prev_page = page_j;
                page_j = page_j->next;
            }
            
            if (found) {
                struct Page *page_k = page_i;

                idx = npages - 1;
                while (idx--) {
                    _set_flag(page_k, PAGE_TAKEN);
                    page_k = page_k->next;
                }
                // last page
                _set_flag(page_k, PAGE_TAKEN);
                _set_flag(page_k, PAGE_LAST);

                return (void *) (_alloc_start + page_i->page_id * PAGE_SIZE_4K);
            }

        }
        idx++;
        page_i = page_i->next;
    }

    return NULL;
}

/*
 * Free the memory block
 * - p: start address of the memory block
 */
void page_free(void *p)
{
    /*
	 * Assert (TBD) if p is invalid
	 */
    if (!p || (uint32_t)p >= _alloc_end) {
        return;
    }
    /* get the first page descriptor of this memory block */
    struct Page *page = (struct Page *)HEAP_START;
    uint8_t page_id = ((uint32_t)p - _alloc_start) / PAGE_SIZE_4K;

    // loop for find node of page by Page ID
    while (page) {
        // find the target for free operation
        if (page->page_id == page_id) {
            while (!_is_free(page)) {
                if (_is_last(page)) {
                    _clear(page);
                    break;
                } else {
                    _clear(page);
                }
                page = page->next;
            }
            break;
        }
        page = page->next;
    }
}

void page_test()
{
    void *p = page_alloc(2);
    printf("p = 0x%x\n", p);

    void *p2 = page_alloc(7);
    printf("p2 = 0x%x\n", p2);
    page_free(p2);

    void *p3 = page_alloc(4);
    printf("p3 = 0x%x\n", p3);

    void *p4 = page_alloc(2);
    printf("p4 = 0x%x\n", p4);
}