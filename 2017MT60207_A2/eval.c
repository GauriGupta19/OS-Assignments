#include "my_alloc.c"

int main(int argc, char *argv[]){

	int tc = 1;		// TC Count
	// Some pointers
	char* PTR_1;
	char* PTR_2;
	char* PTR_3;
	char* PTR_4;
	char* PTR_5;
	char* PTR_6;
	char* PTR_7;

	/* 
	Base Set of TCs:
	check my_heapinfo() working
	check my_alloc: single call, multiple call, very large call
	*/
	// TC 1: Check if my_heapinfo() works
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	my_clean();

	// TC 2: Single allocation
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(64);
	my_heapinfo();
	my_clean();

	// TC 3: Multiple allocations
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(256);
	my_heapinfo();
	PTR_2 = my_alloc(128);
	my_heapinfo();
	PTR_3 = my_alloc(64);
	my_heapinfo();
	my_clean();

	// TC 4: Check very large allocation
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(4096);
	my_heapinfo();
	my_clean();

	// TC : Check my_free
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(128);
	my_heapinfo();
	my_free(PTR_1);
	my_heapinfo();
	my_clean();

	//TC 
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(128);
	//my_heapinfo();
	PTR_2 = my_alloc(64);
	//my_heapinfo();
	PTR_3 = my_alloc(160);
	//my_heapinfo();
	PTR_4 = my_alloc(64);
	//my_heapinfo();
	PTR_5 = my_alloc(512);
	//my_heapinfo();
	PTR_6 = my_alloc(64);
	my_heapinfo();
	my_free(PTR_1);
	my_free(PTR_3);
	my_free(PTR_5);
	my_heapinfo();
	PTR_7 = my_alloc(80);
	my_heapinfo();
	my_clean();
	

	// TC
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(512);
	my_heapinfo();
	PTR_2 = my_alloc(2048);
	my_heapinfo();
	PTR_3 = my_alloc(1024);
	my_heapinfo();
	PTR_4 = my_alloc(128);
	my_heapinfo();
	my_free(PTR_2);
	my_heapinfo();
	my_clean();

	// TC : Fill memory block with as much memory as possible in multiple of 8
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	int mem = 1024;
	while (mem > 8){
		PTR_1 = my_alloc(mem);
		while(PTR_1){
			PTR_1 = my_alloc(mem);
		}
		mem = mem / 2;
	}
	my_heapinfo();
	my_clean();

	// TC: occupy whole memory
	printf("\n********* TC %d **********\n\n", tc++);
	my_init();
	my_heapinfo();
	mem = 4096;
	while(mem > 8){
		PTR_1 = my_alloc(mem);
		if(PTR_1)
			break;
		else
			mem = mem - 8;
	}
	my_heapinfo();
	my_free(PTR_1);
	my_heapinfo();
	my_clean();
	
	// TC :Spam init and clean then check
	printf("\n********* TC %d **********\n\n", tc++);
	for (int i=0; i<10000; i++){
		my_init();
		my_clean();
	}
	my_init();
	my_heapinfo();
	PTR_1 = my_alloc(64);
	my_heapinfo();
	my_clean();

}