#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "MemoryManager.h"
#include "Matrix.h"

int main()
{
     init_mem(4096);
    void *ptr1 = mem_manager_malloc(100);
    printf("%p\n",ptr1);
    void *ptr2 = mem_manager_malloc(100);
    printf("%p\n",ptr2);
    void *ptr3 = mem_manager_malloc(100);
    printf("%p\n",ptr3);
    void *ptr4 = mem_manager_malloc(100);
    printf("%p\n",ptr4);
    void *ptr5 = mem_manager_malloc(100);
    printf("%p\n",ptr5);
    mem_manager_free(ptr1);
 

    printf("Before issue:\n");
    traverse_free_list();

    void *ptr6 = mem_manager_malloc(400);
    printf("%p\n",ptr6);
    void *ptr7 = mem_manager_malloc(400);
    printf("%p\n",ptr7);
    printf("Now:\n");
    //mem_manager_free(ptr4);
    mem_manager_free(ptr6);
    mem_manager_free(ptr5);
    mem_manager_free(ptr2);
    //printf("Now 2:\n");
    traverse_free_list();
    //void *ptr8 = mem_manager_malloc(300);
    printf("After issue:\n");
    traverse_free_list();
    return 0;
}