#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

//data entry for hasharray
struct DataItem {
   struct node* head;   
   struct node* tail;
   unsigned int key;
};
struct node{
	int value;
	struct node * next;
};
//page table entry
struct pte_struct {
   char present;
   unsigned int* physical_address;
   int lru_counter;
   char reference_bit;
   unsigned char dirty;
};
// 32-Bit memory frame data structure
struct frame_struct {
   unsigned int frame_number;
   unsigned int* physical_address;
   unsigned int virtual_address;
   struct pte_struct* pte_pointer;
   struct frame_struct* next;
};

// #define VPN
#define PTE_INDEX(x)  (((x) >> 12) & 0xfffff)
// 4KB (2^12) page size
#define PAGE_SIZE_4KB   4096    
// Page table size = Maximum size of the 32-bit memory (4GB)
// divided by page size (4kB): 2^32 / 2^12 = 2^20 (1MB)
#define PT_SIZE  1048576 
// Page table entry size
#define PTE_SIZE_BYTES sizeof(struct pte_struct*)
// Number of physical frames
uint32_t numframes;
// Physical memory
unsigned int* physical_frames = NULL;
// Page Table
struct pte_struct** page_table = NULL;
// Page Table Entry
struct pte_struct* pte = NULL;
// Fifo page replacement current index
int current_index = -1;
//Define clock hand
struct frame_struct* clk_hand;


unsigned int hashCode(unsigned int key, int size) {
   return key % size;
}

struct DataItem *search(struct DataItem** hashArray, unsigned int key, int size) {
   //get the hash 
   unsigned int hashIndex = hashCode(key, size);  
   
   //move in array until an empty 
   while(hashArray[hashIndex] != NULL) {
      if(hashArray[hashIndex]->key == key)
         return hashArray[hashIndex]; 
      ++hashIndex;
      //wrap around the table
      hashIndex %= size;
   }          
   return NULL;        
}

void insert(struct DataItem** hashArray, unsigned int key,int data, int size) {
   int found = 0;
   //get the hash 
   unsigned int hashIndex = hashCode(key, size); 
   //move in array until an empty or deleted cell
   while(hashArray[hashIndex] != NULL) {
        if(hashArray[hashIndex]->key == key){
         found = 1;
         break;
        }
      ++hashIndex;
      //wrap around the table
      hashIndex %= size;
   }
   if(found == 0){ 
     struct DataItem *item = (struct DataItem*) malloc(sizeof(struct DataItem));
     item->key = key;
     struct node* head = (struct node*) malloc(sizeof(struct node));
     item->head = head;
     item->head->value = data;
     item->tail = head;
     head->next = NULL; 
     hashArray[hashIndex] = item;
   }
   else{
      struct node * new = (struct node*) malloc(sizeof(struct node));
      hashArray[hashIndex]->tail->next = new;
      hashArray[hashIndex]->tail = hashArray[hashIndex]->tail->next;
      hashArray[hashIndex]->tail->value = data;
      hashArray[hashIndex]->tail->next = NULL;
   } 
}

void delete(struct DataItem** hashArray, unsigned int k, int size) {
   unsigned int key = k;
   //get the hash 
   unsigned int hashIndex = hashCode(key, size);
   //move in array until an empty
   while(1) {
      if(hashArray[hashIndex] != NULL){
         if(hashArray[hashIndex]->key == key){ 
         //if there is more than 1 values in the value list
         if(hashArray[hashIndex]->head->next != NULL){
            struct node* temp = hashArray[hashIndex]->head->next;
            free(hashArray[hashIndex]->head);
            hashArray[hashIndex]->head = temp;
            temp = NULL;
         }
         //if its the only value
         else{
            hashArray[hashIndex]->tail = NULL;
            free(hashArray[hashIndex]->head);
            hashArray[hashIndex] = NULL; 
         }
         break;
        }
      }  
      ++hashIndex;
      //wrap around the table
      hashIndex %= size;
   }      
}

// Allocate dynamic memory
void* allocate(unsigned long int size) {
    void* ptr = NULL;
    if(size) {        
        ptr = malloc(size);
        if(!ptr) {
            fprintf(stderr, "Error on mallocing memory...\n");
            exit(1);
        }
        memset(ptr, 0, size);
    }
    return(ptr);
}

// Handles page faults
struct frame_struct * handle_page_fault(unsigned int fault_address) {
    pte = (struct pte_struct*) page_table[PTE_INDEX(fault_address)];
    //if no memory address on that page table entry(page table entry is empty)
    if(!pte) {
        pte = allocate(sizeof(struct pte_struct));
        pte->present = 0;
        pte->reference_bit = 0;
        pte->physical_address = NULL;
        pte->lru_counter= 0;
        page_table[PTE_INDEX(fault_address)] = (struct pte_struct*) pte;
    }
    return ((struct frame_struct *) pte);
}


//opt implementation
int opt_evict(unsigned int* address_array, int numaccesses, struct DataItem** hashArray, struct frame_struct* frames) {
   //loop through frames to find a null pte reference
    struct frame_struct* curr = frames;
    while(curr)
    {  // if found send that frames frame number
      if(curr->pte_pointer == NULL)
      {
         return curr->frame_number;
      }
      curr= curr->next;
    }
    curr = frames;
    //else loop through frames to find the page from hashtable which has the highest value greater than i
    int furthest_time = -1;
    int frame_n = numframes-1;
    int lru = numaccesses;
    int never_accessed = 0;

    while(curr){
      //if the value corresponding to the virtual page is greater than furthest_time, set furthest_address equal to that page
      struct DataItem* page = search(hashArray, PTE_INDEX(curr->virtual_address), numaccesses);
      //if page is in hashtable, get its time, otherwise it will never be accessed, set never accessed and in case of ties, evict the page with minimum physical frame number.
      if(page == NULL)
      {
         never_accessed ++;
         //return curr->frame_number;
         // struct pte_struct* newpte = (struct pte_struct*)page_table[PTE_INDEX(curr->virtual_address)];
         if(never_accessed == 1) frame_n = curr->frame_number;
         if (curr->frame_number < frame_n)
         {
            // lru = newpte->lru_counter;
            frame_n = curr->frame_number;
         }
      }
      else if(page != NULL)
      {
         if(never_accessed == 0)
         {
            int time = page->head->value;
              if(time > furthest_time)
              {
               furthest_time = time;
               frame_n = curr->frame_number;
              }
         }
      }
        curr = curr->next;
    } 
    return frame_n;
}

//fifo implementation
int fifo_evict() {
   current_index++;
   int res= current_index;
   res = res % numframes;
   return res;
}

//ramdom implementation
int rand_evict(struct frame_struct* frames) {
   //loop through frames to find a null pte reference
    struct frame_struct* curr = frames;
    while(curr)
    {  // if found send that frames frame number
      if(curr->pte_pointer == NULL)
      {
         return curr->frame_number;
      }
      curr= curr->next;
    }
    //else return a random frame for replacement
    int idx = (int)(rand() % numframes);
    return idx;
}

//LRU implementation
int lru_evict(struct frame_struct* frames, int numaccesses) {
   //loop through frames to find a null pte reference
    struct frame_struct* curr = frames;
    while(curr)
    {  // if found send that frames frame number
      if(curr->pte_pointer == NULL)
      {
         return curr->frame_number;
      }
      curr= curr->next;
    }
    curr = frames;
    //else loop through frames to find the page that has least time stamp using lru counter
    int frame_n = 0;
    int lru = numaccesses;
    while(curr){
      struct pte_struct* newpte = (struct pte_struct*)page_table[PTE_INDEX(curr->virtual_address)];
      if (newpte->lru_counter < lru){
         lru = newpte->lru_counter;
         frame_n = curr->frame_number;
      }
      curr= curr->next;
   }   
    return frame_n;
}

// For clock algorithm
int clock_evict(struct frame_struct* frames){
//loop through frames to find a null pte reference
    struct frame_struct* curr = frames;
    while(curr)
    {  // if found send that frames frame number
      if(curr->pte_pointer == NULL)
      {
         return curr->frame_number;
      }
      curr= curr->next;
    }

   // find first ref bit that is off i.e. hasn't been used recently start at saved clock hand position
   curr = clk_hand;
    while( curr ){
        struct pte_struct* newpte = (struct pte_struct*)page_table[PTE_INDEX(curr->virtual_address)];
        if (newpte->reference_bit) { // ref bit is on
            // turn it off, go to next page
            newpte->reference_bit = 0; 
        }
        else{
            //if the last replaced page was at position x∈{0,…,n−1} in the circular list, 
            //and then the algorithm circles back not being able to find any page with use bit 0, then next we evict (x+1)th frame.
            // ref bit is off, found it
            // set to 1 so it doesn't get stuck
            newpte->reference_bit = 1;
            // update clock hand position to the next frame
            if(curr->next) clk_hand = curr->next;
            else clk_hand = frames;
            
            return curr->frame_number;
            
        }
        if(curr->next==NULL) curr = frames;
        else curr= curr->next;
    }
    // does not get here
    return 0;
}


int main(int argc, char* argv[]) { 
    bool flag_verbose=false;
    numframes = atoi(argv[2]);
    char* algorithm= argv[3];
    char* filename;
    if(argv[4]){
      flag_verbose=true;
    }
    filename = argv[1];
    
    FILE* file = fopen(filename,"r");
    if(!file) {
        fprintf(stderr, "Error on opening %s\n", filename);
        exit(1); 
    }
    /* 
     * Calculate the trace file's length
     * and read in the trace file
     * and write it into addr_arr and mode_arr arrays 
     */
    unsigned int numaccesses = 0;
    unsigned char mode = '\0';
    unsigned int addr = 0;

    static unsigned char mode_array[10000000];
    static unsigned int address_array[10000000];
    unsigned int i = 0;

    while(fscanf(file, "%x %c\n", &addr, &mode) == 2) {
        address_array[i] = addr;
        mode_array[i] = mode;
        i++;
    }
    numaccesses = i;
    if(fclose(file)) {
        fprintf(stderr, "Error on closing %s\n", filename);
        exit(1);
    }

    // Initialize the physical memory address space
    long frame_size = PAGE_SIZE_4KB;
    long memory_size = frame_size * numframes;
    // printf("check3\n");
    physical_frames = (unsigned int*) allocate(memory_size);
    // printf("check4\n");
    // Create the first frame of the frames linked list
    struct frame_struct* head = NULL;

    struct frame_struct* curr = NULL;
    for(i = 0; i < numframes; i++) {
        if(i == 0) {
            head = (struct frame_struct*) allocate(sizeof(struct frame_struct));
            curr = head;
        }
        else {
            curr->next = (struct frame_struct*) allocate(sizeof(struct frame_struct));
            curr = curr->next;
        }
        curr->frame_number = i;
        curr->physical_address = physical_frames + (i * frame_size);
        curr->virtual_address = 0;
        curr->pte_pointer     = NULL;
    }

    // Initialize page table
    long page_table_size = PT_SIZE * PTE_SIZE_BYTES;
    page_table = (struct pte_struct**) allocate(page_table_size*sizeof(struct pte_struct*));
    
    struct pte_struct* new_pte = NULL;
    unsigned char mode_type = '\0';
    unsigned int fault_address = 0;
    int hit = 0;
    int page2evict = 0;
    int numfaults = 0;
    int nummisses = 0;
    int numdrops = 0;
    int numwrites =0;
   
    //seed for random algorithm
    srand( 5635 );
    clk_hand = head;
    //if algorithm is opt, process memory accesses to populate the hashtable
    struct DataItem** hashArray = allocate(numaccesses * sizeof(struct DataItem*));
    for(int k=0; k < numaccesses; k++){
          hashArray[k] = NULL;
    }

    if(!strcmp(algorithm, "OPT")){
       for(int j = 0; j < numaccesses; j++) {
         //insert all addresses in hashtable
         insert(hashArray, PTE_INDEX(address_array[j]), j, numaccesses);
        }
    }

    // Main loop to process memory accesses
    for(i = 0; i < numaccesses; i++) {
        fault_address = address_array[i];
        mode_type = mode_array[i];
        hit = 0;
        // Perform page walk for the fault address
        new_pte = (struct pte_struct*) handle_page_fault(fault_address); //pte of the fault address

        if(mode_type == 'W') {
            new_pte->dirty = 1;
        }

        if(!strcmp(algorithm, "OPT")){
        //delete the page key and time from hashtable
        delete(hashArray, PTE_INDEX(address_array[i]), numaccesses);
        //update the lru counter
        ((struct pte_struct*) page_table[PTE_INDEX(fault_address)])->lru_counter = i; 
         }

        if(!strcmp(algorithm, "LRU")){
            ((struct pte_struct*) page_table[PTE_INDEX(fault_address)])->lru_counter = i; 
        }
        /*
         * Traverse the frames linked list    
         * to see if the requested page is present in
         * the frames linked list.
         */
        curr = head;
        while(curr) {
         //do the base addresses match
            if(curr->physical_address == new_pte->physical_address) {
               //is page present in memory
                if(new_pte->present) {
                    curr->virtual_address = fault_address;
                    hit = 1;
                    // printf("hit\n"); 
                    //its a hit so reset reference bit to 1 
                    new_pte->reference_bit = 1;    
                    // printf("%d\n", curr->pte_pointer->reference_bit);                  
                    break;
                }  
            }
         curr = curr->next;
        }
        // Requested page not present, page replacement  
       if(!hit) {
            nummisses++;
            // printf("miss\n");

            if(!strcmp(algorithm, "FIFO")) {
                page2evict = fifo_evict();
            }
            else if (!strcmp(algorithm, "OPT")){
               page2evict = opt_evict(address_array, numaccesses, hashArray, head);
            }
            else if (!strcmp(algorithm, "LRU")){
               page2evict = lru_evict(head, numaccesses);
            }
            else if (!strcmp(algorithm, "RANDOM")){
               page2evict = rand_evict(head);
            }
            else if (!strcmp(algorithm, "CLOCK")){
               page2evict = clock_evict(head);
            }
            /* Traverse the frames linked list to find the victim frame and swap it out
            * Set the present bit and dirty bit */
            curr = head;
            while(curr) {
                if(curr->frame_number == page2evict) {
                    numfaults++;
                    if(curr->pte_pointer) {
                        // printf("%d\n", curr->pte_pointer->reference_bit);
                        curr->pte_pointer->present = 0;
                        if(curr->pte_pointer->dirty) {
                            if(flag_verbose) printf("Page 0x%05x was read from disk, page 0x%05x was written to the disk.\n", (unsigned int) (uintptr_t) PTE_INDEX(fault_address), (unsigned int) (uintptr_t) PTE_INDEX(curr->virtual_address));
                            numwrites++;
                            curr->pte_pointer->dirty = 0;
                        }
                        else{
                           numdrops++;
                           if(flag_verbose) printf("Page 0x%05x was read from disk, page 0x%05x was dropped (it was not dirty).\n", (unsigned int) (uintptr_t) PTE_INDEX(fault_address), (unsigned int) (uintptr_t) PTE_INDEX(curr->virtual_address));
                        }
                    }
                    curr->pte_pointer = (struct pte_struct*) new_pte;
                    new_pte->physical_address = curr->physical_address;
                    new_pte->present = 1;
                    new_pte->reference_bit = 1;
                    curr->virtual_address = fault_address; 
                    // printf("%d\n", curr->pte_pointer->reference_bit);
                    break;
                }
                curr = curr->next; 
            }
       }
    }

    /* Release the memory you allocated for frames and page_table */
    free(hashArray);
    free(page_table);
    int h;
    struct frame_struct* temp = head;
    for(h = 0; h <numframes - 1; h++){
      head = head->next;
      free(temp);
      temp = head;
    }
    free(head);
    free(physical_frames);

    printf("Number of memory accesses: %d\n", numaccesses);
    printf("Number of misses: %d\n", nummisses);
    printf("Number of writes: %d\n", numwrites);
    printf("Number of drops: %d\n", numdrops);
    return(0);
}






