# Dynamic-Memory-Allocator
Writing a Dynamic Memory Allocator

Introduction

In this lab you will be writing a dynamic memory allocator for C programs, i.e., your own version of
the malloc, free and realloc routines. You are encouraged to explore the design space creatively
and implement an allocator that is correct, efficient and fast.
The mdriver.c program is a driver
program that allows you to evaluate the performance of your solution. Use the command make
to generate the driver code and run it with the command ./mdriver -v. (The -v flag displays
helpful summary information.)

How to Work on the Lab

Your dynamic memory allocator will consist of the following four functions, which are declared in
mm.h and defined in mm.c.
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *ptr, size_t size);
The mm.c file we have given you implements the simplest but still functionally correct malloc
package. Using this as a starting place, modify these functions (and possibly define other private
static functions), so that they obey the following semantics:
• mm init: Before calling mm malloc mm realloc or mm free, the application program (i.e., the
trace-driven driver program that you will use to evaluate your implementation) calls mm init
to perform any necessary initializations, such as allocating the initial heap area. The return
value should be -1 if there was a problem in performing the initialization, 0 otherwise.
• mm malloc: The mm malloc routine returns a pointer to an allocated block payload of at
least size bytes. The entire allocated block should lie within the heap region and should not
overlap with any other allocated chunk. Your malloc implementation should always return
8-byte aligned pointers like that done in the standard C library.
• mm free: The mm free routine frees the block pointed to by ptr. It returns nothing. This
routine is only guaranteed to work when the passed pointer (ptr) was returned by an earlier
call to mm malloc or mm realloc and has not yet been freed.
• mm realloc: The mm realloc routine returns a pointer to an allocated region of at least
size bytes with the following constraints.
– if ptr is NULL, the call is equivalent to mm malloc(size);
– if size is equal to zero, the call is equivalent to mm free(ptr);
– if ptr is not NULL, it must have been returned by an earlier call to mm malloc or
mm realloc. The call to mm realloc changes the size of the memory block pointed to
by ptr (the old block) to size bytes and returns the address of the new block. Notice
that the address of the new block might be the same as the old block, or it might be
different, depending on your implementation, the amount of internal fragmentation in
the old block, and the size of the realloc request.
The contents of the new block are the same as those of the old ptr block, up to the
minimum of the old and new sizes. Everything else is uninitialized. For example, if the
old block is 8 bytes and the new block is 12 bytes, then the first 8 bytes of the new block
are identical to the first 8 bytes of the old block and the last 4 bytes are uninitialized.
Similarly, if the old block is 8 bytes and the new block is 4 bytes, then the contents of
the new block are identical to the first 4 bytes of the old block.
These semantics match the the semantics of the corresponding malloc, realloc, and free routines
in the standard C library (libc). Type man malloc to the shell for complete documentation.

Heap Consistency Checker

Dynamic memory allocators are notoriously tricky beasts to program correctly and efficiently. They
are difficult to program correctly because they involve a lot of untyped pointer manipulation. You
will find it very helpful to write a heap checker that scans the heap and checks it for consistency.
Some examples of what a heap checker might check are:
• Is every block in the free list marked as free?
• Are there any contiguous free blocks that somehow escaped coalescing?
• Is every free block actually in the free list?
• Do the pointers in the free list point to valid free blocks?
• Do any allocated blocks overlap?
• Do the pointers in a heap block point to valid heap addresses?
Your heap checker will consist of the function int mm check(void) in mm.c. It will check any
invariants or consistency conditions you consider prudent. It returns a nonzero value if and only if
your heap is consistent. You are not limited to the listed suggestions nor are you required to check
all of them. You are encouraged to print out error messages when mm check fails.
This consistency checker is for your own debugging during development. When you submit mm.c,
make sure to comment out any calls to mm check as they will slow down your throughput. Style
points will be given for your mm check function. Make sure to put in comments and document
what you are checking.

Support Routines

Instead of using the sbrk or mmap system call to request for memory allocation from OS, your
memory allocator will be invoking support routines from memlib.c package so that we can test its
performance more easily. The memlib.c package “simulates” the effect of sbrk for your dynamic
memory allocator.
Your memory allocator can invoke the following functions in memlib.c:
• void *mem sbrk(int incr): Expands the heap region by incr bytes, where incr is a positive
non-zero integer and returns a generic pointer to the first byte of the newly allocated
heap area.
• void *mem heap lo(void): Returns a generic pointer to the first byte in the heap.
• void *mem heap hi(void): Returns a generic pointer to the last byte in the heap.
• size t mem heapsize(void): Returns the current size of the heap in bytes.
• size t mem pagesize(void): Returns the system’s page size in bytes (4K on Linux systems).

The driver program mdriver.c in the malloclab-handout.tar distribution tests your mm.c package
for correctness, space utilization, and throughput. The driver program uses a set of trace files
in the traces/ directory in the malloclab-handout.tar distribution. Each trace file contains a
sequence of allocate, reallocate, and free directions that instruct the driver to call your mm malloc,
mm realloc, and mm free routines in some sequence.

Below is a list of some useful command line arguments of mdriver.c:
• -h: Print a summary of the command line arguments.
• -f <tracefile>: Use one particular tracefile for testing instead of the default set of
tracefiles defined in config.h. For example, ./mdriver -f traces/short1.rep tests the
allocator using the simplest trace short1.rep.
• -l: Run and measure the memory allocator in libc in addition to the your own malloc
package.
• -v: Verbose output. Print a performance breakdown for each tracefile in a compact table.
• -V: More verbose output. Prints additional diagnostic information as each trace file is processed.
Useful during debugging for determining which trace file is causing your malloc
package to fail.
