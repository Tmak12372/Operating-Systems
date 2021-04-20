#ifndef __MATRIX_H
#define __MATRIX_H

/*

Tyler McCormick
Program 1
Matrix.h

*/

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include "MemoryManager.h"


typedef struct __matrix
{
	int num_rows;
	int num_cols;
	double* elements;
} matrix;

/*
############################################################
MATRIX_MALLOC

Initializing how many rows and columns your matrix will have.

############################################################
*/

matrix* matrix_malloc(int num_rows, int num_cols)
{
	int total_size = num_rows*num_cols;
	if (num_rows <= 0 || num_cols <= 0)
	{
		printf("Cannot have 0 or less rows or columns\n");
		return 0;
	}
	matrix* temp_matrix = (matrix *)mem_manager_malloc(sizeof(matrix));
	temp_matrix->num_rows = num_rows;
	temp_matrix->num_cols = num_cols;
	temp_matrix->elements = (double *)mem_manager_malloc(total_size*(sizeof(double)));
	for (int i = 0; i < total_size; i++)
	{
		temp_matrix->elements[i] = 0;
	}
	return temp_matrix;
}

/*
############################################################
MATRIX_FREE

Frees your matrix and its elements

############################################################
*/

void matrix_free(matrix* mat)
{
	if (mat->num_rows == 0 || mat->num_cols == 0 || mat == NULL)
	{
		printf("No matrix to free!\n");
		return;
	}
	mem_manager_free(mat->elements); //free all of the elements in the matrix
	mem_manager_free(mat);			 //free the matrix itself
}


/*
############################################################
SET_ELEMENT

Sets each individual value of the matrix

############################################################
*/


void set_element(matrix* mat, int row, int col, double val)
{
	if (mat->num_rows == 0 || mat->num_cols == 0 || mat == NULL)
	{
		printf("No matrix to give data!\n");
		return;
	}
	int index = (row - 1)*mat->num_cols + col - 1;
	mat->elements[index] = val;
}


/*
############################################################
GET_ELEMENT

Gets each individual value of the matrix

############################################################
*/

double get_element(matrix* mat, int row, int col)
{
	if (mat->num_rows == 0 || mat->num_cols == 0 || mat == NULL)
	{
		printf("No matrix to get data from!\n");
		return 0;
	}
	int index = (row - 1)*mat->num_cols + col - 1;
	return mat->elements[index];
}


/*
############################################################
MULTIPLY

Multiplies matrices of the same size

############################################################
*/
matrix* multiply(matrix* left, matrix* right)
{
	int left_rows = left->num_rows;
	int left_cols = left->num_cols;
	int right_rows = right->num_rows;
	int right_cols = right->num_cols;
	matrix* result = matrix_malloc(left_rows, right_cols);
	
	for (int i = 1; i <= left_rows; i++)
	{
		for (int j = 1; j <= right_cols; j++)
		{
			
			double val = 0;
			for (int k = 1; k <= left_cols; k++)
			{
				double element_left = get_element(left, i, k);
				double element_right = get_element(right, k, j);
				double mul = element_left * element_right;
				val += mul;
			}
			set_element(result, i, j, val);
		}
	}
	return result;
}
	
/*
############################################################
DISPLAY

Displays matrix in matrix format.

############################################################
*/

void display(matrix* mat)
{
	if (mat->num_rows == 0 || mat->num_cols == 0 || mat == NULL)
	{
		printf("No matrix to display!\n");
		return;
	}
	for (int rows = 1; rows <= mat->num_rows; rows++)
	{
		for (int columns = 1; columns <= mat->num_cols; columns++)
		{	
			int index = ((rows - 1)*mat->num_cols + columns - 1);
			printf("%f ", mat->elements[index]);
		}
		printf("\n");
	}
}

#endif /* __MATRIX_H */