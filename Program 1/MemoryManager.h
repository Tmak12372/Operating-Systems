#ifndef __MEMORYMANAGER_H
#define __MEMORYMANAGER_H
/*

Tyler McCormick
Program 1
MemoryManager.h

*/


#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct __mmalloc_t
{
	int size;
	int magic;
	double padding;
} mmalloc_t;

typedef struct __mmfree_t
{
	int size;
	struct __mmfree_t *next;
	
} mmfree_t;

mmfree_t* head;
#define MAGIC_NUMBER 12345
#define EMPTY NULL //just to replace NULL


void* mem_manager_malloc(int size);
void mem_manager_free(void* ptr);
//traverse the free space list starting from the head, printing out info (for debugging)
void traverse_free_list();
void init_mem(int free_space_size);

//called by malloc
//find a free space chunk large enough for the requested allocation
//obtain some memory from that chunk
mmfree_t* locate_split(mmfree_t* curr,int total_requested_size);

//called by free
//locate the freed memory insert position so free space nodes are sorted by address
mmfree_t* find_sorted_location(mmfree_t* head, mmfree_t* to_insert);



//FINISHED
void* mem_manager_malloc(int size)
{
    void *Split_Located = EMPTY; 
    mmfree_t *sent = head; //send into locate_split()
    Split_Located = locate_split(sent,size); //located location to put data

    if (Split_Located == EMPTY){
        return EMPTY; //if no memory exit
    }

	mmfree_t *current_head = head;
	while(current_head->next != Split_Located && current_head->next != EMPTY)
	{
		current_head = current_head->next;
	}

	mmalloc_t *head_holder = (mmalloc_t*)Split_Located;
	int current_size = head_holder->size;
	head_holder->size = size;
	head_holder->magic = MAGIC_NUMBER;

	void *temp_pointer = ((void*)head_holder) + 16;
	Split_Located = Split_Located + size + 16;

	mmfree_t* tempPointer2 = (mmfree_t*)Split_Located;
	
	if(current_size == size + 16){
		head = head->next;
	}
	else{
		tempPointer2->size = current_size - size - 16;
	}
	if((void*)head != Split_Located - size - 16){
		tempPointer2->size = current_size - size - 16;
		if(tempPointer2->size == 0)
		{
			current_head->next = EMPTY;
		}
		else
		{
			tempPointer2->next = current_head->next->next;
			current_head->next = tempPointer2;
		}
	}
	else
	{
		void* next_head = head->next;
		head = Split_Located;
		head->next = next_head;
	}
    return temp_pointer; //return location
}

void mem_manager_free(void *ptr){
	
	void* temp = ptr - 16;
	
	mmalloc_t* free_head = (mmalloc_t*)temp;
	int currentFreeSize = free_head->size;

	mmfree_t* temp2 = (mmfree_t*)temp;
	temp2->size = currentFreeSize;
	mmfree_t* new_location = find_sorted_location(head, temp2);
	int diff = (int)head - (int)temp2;
	if(new_location == head && (diff > 0)){
		temp2->next = head;
		head = temp2;
	}
	else{
		mmfree_t* prev_location = new_location->next;
		new_location->next = temp2;
		temp2->next = prev_location;
	}

	//Coalescing twice to get all in order

	for(int i = 1; i <= 2; i++)
	{
		mmfree_t* current = head;
		while(current)
		{		
			if((int)(current->next)-(int)current == ((current->size) + 16))
			{
				current->size = current->size + current->next->size + 16;
				current->next = current->next->next;
			}
			current = current->next;
		}
	}
}


void traverse_free_list(){
    mmfree_t *head_display = head;
    while (head_display)
    {
        printf("Size: %d\nLocation: %p\n",head_display->size,head_display);
        head_display = head_display->next;
        if(head_display == EMPTY)
		{
            break;
        }
    }
}


void init_mem(int free_space_size)
{
	head = (mmfree_t*)mmap(EMPTY, free_space_size, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0); 
	head->size = free_space_size - sizeof(mmfree_t);
	head->next = EMPTY;
}


mmfree_t* locate_split(mmfree_t *curr,int total_requested_size)
{
	mmfree_t* next_split = curr;
	while (next_split)
	{
		if(next_split->size >= total_requested_size + 16)
		{
			return next_split;
		}
		else if(next_split == next_split->next)
		{
			next_split->next = EMPTY;
		}
		else
		{
			next_split = next_split->next;
		}
	}
	return EMPTY;
}


mmfree_t* find_sorted_location(mmfree_t* head, mmfree_t* to_insert){
	mmfree_t* temp_head = head;
	mmfree_t* previous = temp_head;
	int sort_number = to_insert - temp_head;
	if(temp_head->next == EMPTY){
		return head;
	}
	while(sort_number > 0 && (temp_head->next != EMPTY)){
		previous = temp_head;
		temp_head = temp_head->next;
		sort_number = to_insert - temp_head;
	}
	return previous;
}

#endif /* __MEMORYMANAGER_H */

