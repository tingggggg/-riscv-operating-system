#include "os.h"
#include "buddy_system.h"

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
struct buddy *buddy_sys = 0;


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

// static inline void _init_page_linked_list(void *page_tabel_start, uint32_t num_page)
// {
//     struct Page *page = (struct Page *) page_tabel_start;

//     printf("page :0x%x, page + 1: 0x%x\n", page, page + 1);
//     for (int i = 0; i < num_page; i++) {
//         _clear(page);
//         page->page_id = i;
//         page->next = page + 1;
//         page = page->next;
//     }
//     page -= 1;
//     page->next = NULL;
// }

void page_init()
{
    // buddy_sys = buddy_new(1024, HEAP_START);
    buddy_sys = buddy_new(8, HEAP_START);
    printf("***** Page init *****\n");
    buddy_dump(buddy_sys);

    _alloc_start = (HEAP_START + HEAP_SIZE) - 1024 * KB;
    _alloc_end = (HEAP_START + HEAP_SIZE);
    
    printf("HEAP_START = %x, HEAP_SIZE = %x\n", HEAP_START, HEAP_SIZE);
    printf("\t_alloc_start: 0x%x \n\t_alloc_end: 0x%x\n", _alloc_start, _alloc_end);

    printf("TEXT:   0x%x -> 0x%x\n", TEXT_START, TEXT_END);
	printf("RODATA: 0x%x -> 0x%x\n", RODATA_START, RODATA_END);
	printf("DATA:   0x%x -> 0x%x\n", DATA_START, DATA_END);
	printf("BSS:    0x%x -> 0x%x\n", BSS_START, BSS_END);
}

/*
 * Allocate a memory block which is composed of contiguous physical pages
 * - npages: the number of PAGE_SIZE pages to allocate
 */
void *page_alloc(int page_size)
{
    page_size /= KB;
    uint32_t offset = buddy_alloc(buddy_sys, page_size);
    
    // Allocate failed
    if (offset == -1)
        return (void *) 0;

    return _alloc_start + (offset * KB);
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

    uint8_t page_offset = ((uint32_t)p - _alloc_start) / KB;
    buddy_free(buddy_sys, page_offset);
}

void page_test()
{   
    printf("\n***** Page alloc test *****\n");
    void *p = page_alloc(1 * KB);
    printf("Allocate 1KB : 0x%x\n", p);

    void *p2 = page_alloc(2 * KB);
    printf("Allocate 2KB : 0x%x\n", p2);

    void *p3 = page_alloc(4 * KB);
    printf("Allocate 4KB : 0x%x\n", p3);

    page_free(p2);
    void *p4 = page_alloc(1 * KB);
    printf("Allocate 1KB : 0x%x\n", p4);

    void *p5 = page_alloc(2 * KB);
    printf("Allocate 2KB : 0x%x\n", p5);

    buddy_dump(buddy_sys);
}