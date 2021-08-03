/* Malloc and Free implementation. */
/* Author: Gauri Gupta */

#include <stdio.h>
#include <sys/mman.h>

/* Represent the explicit free-list as doubly linked list.
 * prev: points to previous free block
 * next: points to next free block
 * size: size of the usable memory in block */
typedef struct block_data {
   struct block_data *prev;
   struct block_data *next;
   size_t size;
} block;

typedef struct heap_data{
	size_t size_occupied; //malloc size
	int  mem_blocks;
	int n_nodes;
	size_t max_size;
	size_t min_size;
}header;

block *head = NULL;
header *head_heap = NULL;

#define PAGE_SIZE 4096
//returns pointer to the usable memory location
#define BLOCK_MEM( ptr ) ( void * )( (unsigned long)ptr + sizeof( block ) )
//returns pointer to the head of the block 
#define BLOCK_HEADER( ptr ) ( void * )( (unsigned long)ptr - sizeof( block ) )



//request a page of 4KB from the memory and 
//initialize the freelist. 
int my_init(){
   head_heap = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE,
                            MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
   if(head_heap == MAP_FAILED){
      printf("....Mapping Failed...\n");
      return 1;
   }
   else{
   	  head = ( void * )(unsigned long)head_heap + sizeof( header );
      head->prev = NULL;
      head->next = NULL;
      /* only store the usable size */
      head->size = PAGE_SIZE - sizeof( block ) - sizeof ( header );
      head_heap->size_occupied = 0;
      head_heap->n_nodes =1;
      head_heap->mem_blocks = 0;
      head_heap->min_size = head->size;
      head_heap->max_size = head->size;

      return 0;
   }
}

//prints the current status of heap
void my_heapinfo(){

	int a, b, c, d, e, f;
	a = PAGE_SIZE - sizeof ( header );
	b = head_heap->size_occupied + head_heap->n_nodes*sizeof( block );
	c = a-b;
	d = head_heap->mem_blocks;
	//size of smallest available chunk excluding the header
	e = head_heap->min_size;
	//size of largest available chunk excluding the header
	f = head_heap->max_size;
	// Do not edit below output format
	printf("=== Heap Info ================\n");
	printf("Max Size: %d\n", a);
	printf("Current Size: %d\n", b);
	printf("Free Memory: %d\n", c);
	printf("Blocks allocated: %d\n", d);
	printf("Smallest available chunk: %d\n", e);
	printf("Largest available chunk: %d\n", f);
	printf("==============================\n");
	// Do not edit above output format
	return;
}

/* Print the current free-list.*/
void print_list() {
   block *ptr = head;
   while( ptr ) {
      printf( "block addr: %ld, size: %ld \n", (unsigned long)ptr, ptr->size );
      ptr = ptr->next;
   }
}

/* Adds a block to the free list keeping the list sorted by block
 * begin address, this helps when scanning continous blocks */
void addToFreeList( block *freeBlock ) {
   head_heap->n_nodes++;
   freeBlock->next = NULL;
   freeBlock->prev = NULL;
   if( !head || (unsigned long) head >= (unsigned long) freeBlock ) {
      if( head ) {
         head->prev = freeBlock;
      }
      freeBlock->next = head;
      head = freeBlock;
      return;
   }
   block *ptr = head;
   while( ptr->next && (unsigned long) ptr->next < (unsigned long) freeBlock ) {
      ptr = ptr->next;
   }
   freeBlock->next = ptr->next;
   freeBlock->prev = ptr;
   if( ptr->next ) ( ptr->next )->prev = freeBlock;
   ptr->next = freeBlock;
   return;
}

/* Removes a block from the free list */
void removeFromFreeList( block * b ) {
   head_heap->n_nodes--;
   if( !b->prev ) {
      if( b->next ) {
         head = b->next;
      } else {
         head = NULL;
      }
   } else {
      b->prev->next = b->next;
   }
   if( b->next ) {  
      b->next->prev = b->prev;
   }
}

void * my_alloc( size_t size ) {
   if( size%8 != 0){
      // printf("Cannot allocate memory that is not a multiple of 8 bytes\n");
      return NULL;
   }
   if(size + sizeof( block )> PAGE_SIZE - sizeof ( header )-head_heap->size_occupied - head_heap->n_nodes*sizeof( block )){
       return NULL;
   }
   block *ptr = head;
   while( ptr ) {
      /* traverse free list and find suitable block according to first fit strategy*/
      if( ptr->size >= size ) {
         removeFromFreeList( ptr );
         head_heap->mem_blocks++;
         if( ptr->size == size ) {
         	if(size == head_heap->min_size || size == head_heap->max_size){
         		block *temp = head;
         		head_heap->min_size = head->size;
         		head_heap->max_size = head->size;
			   	while( temp ) {
			      if(temp->size < head_heap->min_size){
			      	head_heap->min_size = temp->size;
			      }
			      if(temp->size > head_heap->max_size){
			      	head_heap->max_size = temp->size;
			      }
			      temp = temp->next;
			    }
         	}
         	head_heap->size_occupied += size + sizeof( block );
            return BLOCK_MEM( ptr );
         }
         // block is bigger than requested, split and add
         // split the block after 'sizeof( block ) + size' bytes and form a new spare block
         // add the new block to free-list
         int a = ptr->size;
         block *newBlock = (block *) ( (unsigned long)ptr + sizeof( block ) + size );
         newBlock->size = ptr->size - ( sizeof( block ) + size );
         ptr->size = size;
         addToFreeList( newBlock );
         if(newBlock->size < head_heap->min_size){
         	head_heap->min_size = newBlock->size;
         }
         if(a == head_heap->max_size){
         	block *temp = head;
         	head_heap->min_size = head->size;
         	head_heap->max_size = head->size;
		   	while( temp ) {
		      if(temp->size < head_heap->min_size){
		      	head_heap->min_size = temp->size;
		      }
		      if(temp->size > head_heap->max_size){
		      	head_heap->max_size = temp->size;
		      }
		      temp = temp->next;
		    }
         }
         head_heap->size_occupied += size + sizeof( block );
         return BLOCK_MEM( ptr );
      }
      ptr = ptr->next;
   }
   /* Unable to find free block on free list */
   // printf("Unable to find a free block to accote %ld size of memory\n", size);
   return NULL;
}

/* scans the free list to find continuous free blocks that
 * can be coaleased*/
void scanAndCoalesce() {
   block *curr = head;
   int flag = 0;
   unsigned long curr_addr, next_addr;
   while( curr->next ) {
      while( ( unsigned long ) curr + sizeof( block ) + curr->size == ( unsigned long ) curr->next ) {
      	
      	 if(curr->size == head_heap->min_size || curr->next->size == head_heap->min_size){
      	 	flag = 1;
      	 }
         // found two continous block, merge to form a new large block
      	 head_heap->n_nodes--;
         curr->size += curr->next->size + sizeof( block );
         curr->next = curr->next->next;
         if( curr->next ) {
            curr->next->prev = curr;
         }
         if( flag == 1){
         	block *temp = head;
         	head_heap->min_size = head->size;
		   	while( temp ) {
		      if(temp->size < head_heap->min_size){
		      	head_heap->min_size = temp->size;
		      }
		      temp = temp->next;
		    }
         }
         if(curr->size > head_heap->max_size){
         	head_heap->max_size = curr->size;
         }
         if(curr->next == NULL){
         	break;
         }
      }
      if(curr->next == NULL){
         	break;
       }
      curr = curr->next;
   }
}

/* Puts the block on free-list and merge the contigous block */
void my_free( void * addr ) {
   head_heap->mem_blocks--;
   block *block_addr = BLOCK_HEADER( addr );
   head_heap->size_occupied -= block_addr->size + sizeof( block );
   addToFreeList( block_addr );
   if(block_addr->size < head_heap->min_size){
   	head_heap->min_size = block_addr->size;
   }
   if(block_addr->size > head_heap->max_size){
   	head_heap->max_size = block_addr->size;
   }
   scanAndCoalesce();
}

//return the allocated memory back to the OS 
void my_clean() {
   if( head_heap != NULL ) {
      munmap(head_heap, PAGE_SIZE);
   }
   // head = NULL;
}


