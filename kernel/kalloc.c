// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

#define MAX_PSYC_PAGES (PHYSTOP / PGSIZE) // 物理页面的数量
#define PA2INDEX(pa) ((uint64)pa / PGSIZE)

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run *freelist;
} kmem;

struct {
    struct spinlock lock;
    uint16 refcount[MAX_PSYC_PAGES]; // 引用计数数组
} page_refcounts;

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  initlock(&page_refcounts.lock, "page_refcounts"); // 初始化引用计数锁
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void *pa_start, void *pa_end)
{
  char *p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE){
    page_refcounts.refcount[PA2INDEX(p)] = 1;
    kfree(p);
  }

}

// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void *pa)
{
  struct run *r;

  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");
  acquire(&page_refcounts.lock);
  if(--page_refcounts.refcount[PA2INDEX(pa)] <= 0){ // 先减少引用计数，如果小于等于 0 就真的释放
    memset(pa, 1, PGSIZE);
    // Fill with junk to catch dangling refs.
    r = (struct run*)pa;
    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  release(&page_refcounts.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
  struct run *r;

  acquire(&kmem.lock);
  r = kmem.freelist;
  if(r)
    kmem.freelist = r->next;
  release(&kmem.lock);

  if(r){
    memset((char*)r, 5, PGSIZE); // 填充为垃圾
    // 增加引用计数需要在锁的保护下进行
    acquire(&page_refcounts.lock);
    page_refcounts.refcount[PA2INDEX(r)] = 1;
    release(&page_refcounts.lock);
  }
  return (void*)r;
}

void
cowalloc(void *pa){
  if(((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("cowalloc");
  acquire(&page_refcounts.lock);
  page_refcounts.refcount[PA2INDEX(pa)]++;
  release(&page_refcounts.lock);
}
