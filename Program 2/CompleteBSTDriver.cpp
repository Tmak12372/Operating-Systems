#include <cmath>
#include <ctime>
#include <cstdlib>
#include <cstdio>

#include "BinarySearchTree.h"
using CSC1310::BinarySearchTree;
#include "BinaryTreeIterator.h"
using CSC1310::BinaryTreeIterator;
#include "ListArray.h"
using CSC1310::ListArray;
#include "ListArrayIterator.h"
using CSC1310::ListArrayIterator;

#include "Permutation.h"
using CSC1310::Permutation;
#include "Random.h"
using CSC1310::Random;

#include <pthread.h>
#include "CD.h"

#include <iostream>
using namespace std;

int BUFFER_SIZE = 100;
int NUM_PRODUCERS = 10;
int NUM_CONSUMERS = 50;
int NUM_TREES = 10000;
int ERROR_RATE = 10;

CD*** buffer;
int buffer_count = 0;
int producer_index = 0;
int consumer_index = 0;

int num_trees_p, num_tree_producers = 0;
int num_trees_c, num_tree_consumers = 0;


pthread_cond_t empty, full;
pthread_mutex_t mutex;

CD** producer_seq(ListArray<CD>* cds, Random* rand);
void consumer_seq(CD** cds_array, int num_items, int expected_height);
void* producer_parallel(void *Arguments);
void* consumer_parallel(void *Arguments);
void deleteCDs(ListArray<CD>* list);
void put(CD** cds_array);
CD** get();




void put(CD** cds_array)
{
	buffer[producer_index] = cds_array;
	producer_index = (producer_index + 1) % BUFFER_SIZE;
	buffer_count++;  //buffer fills up
}

CD** get()
{
	CD** cds_array = buffer[consumer_index];
	consumer_index = (consumer_index + 1) % BUFFER_SIZE;
	buffer_count--;  //buffer empties out
	return cds_array;
}

void deleteCDs(ListArray<CD>* list)
{
   ListArrayIterator<CD>* iter = list->iterator();

   while(iter->hasNext())
   {
      CD* cd = iter->next();
      delete cd;
   }

   delete iter;
}

CD** producer_seq(ListArray<CD>* cds, Random* rand)
{
//1.  get an array with the cds in a random order	
	    int num_items = cds->size();
		Permutation* p = new Permutation(num_items, num_items, rand);
		p->basicInit();

		CD** permute_cds = new CD*[num_items];
		int count = 0;
		while(p->hasNext())
		{
			int i = p->next();
			permute_cds[count] = cds->get(i);
			count++;
		}
		delete p;
//2.  inset the cds in array order (different each time) into BST
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		for (int i = 0; i < num_items; i++)
		{
			CD* cd = permute_cds[i];
			bst->insert(cd);
		}
		delete[] permute_cds;
  
//3.  create a complete binary search tree
		BinarySearchTree<CD>* complete_bst = bst->minimizeComplete();
		CD** cds_array = new CD*[num_items];
		BinaryTreeIterator<CD>* complete_iter = complete_bst->iterator();

//4.  place the cds into an array using a level order traversal
		//intentionally inserting an error in some cases using a post order traversal
		int traversal_error = rand->getRandomInt(1, NUM_TREES);
		if (traversal_error >= ERROR_RATE)
		{
			complete_iter->setLevelorder();
		}
		else
		{
			//wrong traversal, consumer should detect a height error
			//still will be sorted, however
			complete_iter->setPostorder();
		}
		count = 0;
		while(complete_iter->hasNext())
		{
			cds_array[count] = complete_iter->next();
			count++;
		}
		delete bst;
		delete complete_iter;
		delete complete_bst;
		
//5.  return the array that is supposed to represent a complete binary tree		
		return cds_array;
}

void consumer_seq(CD** cds_array, int num_items, int expected_height)
{
//1.  put the items in the provided array into a BST
//note that if the array represents a complete binary tree,
//this process will create a complete binary tree that is also a BST
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		for (int i = 0; i < num_items; i++)
		{
			CD* cd = cds_array[i];
			bst->insert(cd);
		}

//2.  verify that the items are in sorted order using an inorder traversal
		BinaryTreeIterator<CD>* bst_iter = bst->iterator();
		bst_iter->setInorder();  //this was omitted
		CD* first = bst_iter->next();
		bool sorted = true;
		while(bst_iter->hasNext())
		{
			CD* second = bst_iter->next();
			if (CD::compare_items(first, second) >= 0)
			{
				sorted = false;
				break;
			}
			first = second;
		}
		delete bst_iter;

//3.  check that the BST is minimum height and balanced
//the randomly determined errors will fail the height test   
		int h = bst->getHeight();
		bool bal = bst->isBalanced();
   
		if (!sorted || h != expected_height || !bal)
		{
			cout << "invalid complete binary tree" << endl;
		}
		else
		{
			//cout << "valid complete binary tree" << endl;
		}

		delete bst;
		delete[] cds_array;
}

void* producer_parallel(void *Arguments)
{
	long* Temp_Arguments = (long*) Arguments;
	ListArray<CD>* cds = (ListArray<CD>*) Temp_Arguments[0];
	Random* rand = (Random*) Temp_Arguments[1];
	int num_items = cds->size();

	while(true)
	{
		pthread_mutex_lock(&mutex);
		if (num_trees_p + num_tree_producers >= NUM_TREES)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		else 
		{
			num_tree_producers++;
			pthread_mutex_unlock(&mutex);
		}

		Permutation* p = new Permutation(num_items, num_items, rand);
		p->basicInit();
		CD** permute_cds = new CD*[num_items];
		int count = 0;

		while(p->hasNext())
		{
			int p_next = p->next();
			permute_cds[count] = cds->get(p_next);
			count++;
		}
		delete p;

		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		
        for (int i = 0; i < num_items; i++)
		{
			CD* cd = permute_cds[i];
			bst->insert(cd);
		}
		delete[] permute_cds;


		BinarySearchTree<CD>* complete_bst = bst->minimizeComplete();
		CD** cds_array = new CD*[num_items];
		BinaryTreeIterator<CD>* complete_iter = complete_bst->iterator();


		//intentionally inserting an error
		int traversal_error = rand->getRandomInt(1, NUM_TREES);		
		if (traversal_error >= ERROR_RATE)
		{
			complete_iter->setLevelorder();
		}
		else
		{
			//wrong traversal, consumer should detect a height error
			//still will be sorted, however
			complete_iter->setPostorder();
		}
		count = 0;
		while(complete_iter->hasNext())
		{
			cds_array[count] = complete_iter->next();
			count++;
		}
		delete bst;
		delete complete_iter;
		delete complete_bst;

		pthread_mutex_lock(&mutex); //producer lock

		while (buffer_count == BUFFER_SIZE)
		{
			pthread_cond_wait(&empty, &mutex); //wait if there is something in buffer
		}
		
		put(cds_array);
		num_trees_p++;
		num_tree_producers--;
		pthread_cond_signal(&full); //signal that you put a producer on the table 
		pthread_mutex_unlock(&mutex); //producer unlock
	}
	return 0;
}

void* consumer_parallel(void *Arguments)
{
	int* Temp_Arguments = (int*) Arguments;	
	int num_items = Temp_Arguments[0];
	int expected_height = Temp_Arguments[1];
	
	while (true)
    {
		pthread_mutex_lock(&mutex); //lock
		if (num_trees_c + num_tree_consumers >= NUM_TREES) 
        {
			pthread_mutex_unlock(&mutex);				
			break;
		}
		else
        {
			num_tree_consumers++; 
			pthread_mutex_unlock(&mutex);				
		} //unlock
		
		pthread_mutex_lock(&mutex); //consumer lock

		while(buffer_count == 0)
        {
			pthread_cond_wait(&full, &mutex); //wait if the buffer is empty
		}
		
		CD** cdArray = get();
		num_trees_c++; //increment number since got()

		pthread_cond_signal(&empty); //consumer signal that its not empty
		pthread_mutex_unlock(&mutex); //consumer unlock
		
		//Get bst from the buffer and eat it
		BinarySearchTree<CD>* bst = new BinarySearchTree<CD>(&CD::compare_items, &CD::compare_keys);
		for (int i = 0; i < num_items; i++)
		{
			CD* cd = cdArray[i];
			bst->insert(cd);
		}

		BinaryTreeIterator<CD>* bst_iter = bst->iterator();
		CD* first = bst_iter->next();
		bool sorted = true;
		while(bst_iter->hasNext())
		{
			CD* second = bst_iter->next();
			if (CD::compare_items(first, second) >= 0)
			{
				sorted = false;
				break;
			}
			first = second;
		}
		delete bst_iter;

		int height = bst->getHeight();
		bool balanced = bst->isBalanced();

		if (sorted == false || height != expected_height || balanced == false)
		{
			cout << "invalid complete binary tree" << endl;
		}
		else
		{
			//cout << "valid complete binary tree" << endl;
		}

		delete bst;
		delete[] cdArray;
		pthread_mutex_lock(&mutex); //protect our consumer number;
		num_tree_consumers--; 
		pthread_mutex_unlock(&mutex); //unlock our lock
	}
	return 0;
}



int main()
{
	
	buffer = new CD**[BUFFER_SIZE];
	
	time_t sequential_start;
	time_t sequential_end;
	time_t parallel_start;
	time_t parallel_end;
	Random* rand = Random::getRandom();
	   
    //the unsorted ListArray of cds
    ListArray<CD>* cds = CD::readCDs("cds.txt");
    int num_items = cds->size();
    cout << num_items << endl;
    cout << "based on the number of items, the min height should be: " << endl;
    int expected_height = ceil(log2(num_items + 1));
    cout << expected_height << endl;
    cout << endl;
   
    long* producer_args = new long[2];
    producer_args[0] = (long) cds;
    producer_args[1] = (long) rand;
    long** temp_p = &producer_args;
   
    int* consumer_args = new int[2];
    consumer_args[0] = num_items;
    consumer_args[1] = expected_height;
    int** temp_c = &consumer_args;
   
  
  
  
	//Sequential 
	
	//getting sequential time
    sequential_start = time(NULL);
	printf("Sequential \n");
	for (int i = 1; i <= NUM_TREES; i++)
	{
	    CD** cd_array = producer_seq(cds, rand);
		consumer_seq(cd_array, num_items, expected_height);
	}
   
    sequential_end = time(NULL);
	
	
	//Parallel program running
	
	//initiate our threads
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init(&empty, NULL);
    pthread_cond_init(&full, NULL);	

	
    pthread_t producer_threads[NUM_PRODUCERS];
    pthread_t consumer_threads[NUM_CONSUMERS];


	//getting parallel time from start and finish and creating and joining threads
	parallel_start = time(NULL); 
    cout << "Parallel \n";

	for (int i = 0; i < NUM_PRODUCERS; i++)
	{
		pthread_create(&producer_threads[i], NULL, producer_parallel, producer_args); // creating your producer threads
	}
	for (int i = 0; i < NUM_CONSUMERS; i++)
	{
		pthread_create(&consumer_threads[i], NULL, consumer_parallel, consumer_args); //creating your consumer threads
	}
	for (int i = 0; i < NUM_PRODUCERS; i++)
	{
		pthread_join(producer_threads[i], NULL); //finishing up your producer threads
	}
	for (int i = 0; i < NUM_CONSUMERS; i++)
	{
		pthread_join(consumer_threads[i], NULL); //finishing up your consumer threads
	}
   
    parallel_end = time(NULL); //getting end time


	//printing out times
    printf("Sequential: %ds\n", (int)(sequential_end - sequential_start)); 
    printf("Parallel: %ds\n", (int)(parallel_end - parallel_start));	
    delete[] producer_args;
    delete[] consumer_args;
    deleteCDs(cds);
    delete cds;

    return 0;
}