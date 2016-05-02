//Steven Zhu EID: sz4464
//Adam Alloy EID: ara2338
//Allocation system: buddy allocation. Data structure: tree. 

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "mymalloc.h"

// global variables
static void *mem;

// tNode struct used for tree
typedef struct tNode{
   size_t nSize;
   int ID;
   void* ptr;
   int used;
   struct tNode *left;
   struct tNode *right;
}tNode;

tNode tree;

// this builds the initial tree used to hold the obj_mems we want to manage
tNode* rectreebuilder(size_t blocksize, int num, void* parent_ptr){
   tNode* newNode = malloc (sizeof (tNode));
   newNode->nSize = blocksize;
   newNode->ID = num;
   newNode->ptr = parent_ptr;
   if (blocksize/2 >= 1*MiB) {
      long mem_temp = (long)newNode->ptr;
      newNode->left = rectreebuilder(blocksize/2, num*2, (void*)mem_temp);
      newNode->right = rectreebuilder(blocksize/2, num*2+1, (void*)(mem_temp + blocksize/2));
   } else {
      newNode->left = NULL;
      newNode->right = NULL;
   }
   return newNode;
}

// initial function call that returns the block of memory used during mem allocation
void my_malloc_init(size_t size) {
   mem = malloc(size);
   tree.nSize = size;
   tree.ID = 1;
   tree.ptr = mem;
   long mem_temp = (long)tree.ptr;
   tree.left = rectreebuilder(size/2, tree.ID*2, (void*)mem_temp);
   tree.right = rectreebuilder(size/2, tree.ID*2+1, (void*)(mem_temp + size/2));
}


// checks to see if all descends of this node is empty so that nothing is 
// overwritten if this node is changed to used
int check_descendents_empty(tNode* cur) {
   if (cur == NULL) {
      return 1;   
   } else if (cur->used == 1) {
      return 0;
   } else {
      int valid = check_descendents_empty(cur->left);
      if (valid == 1) {
         valid = check_descendents_empty(cur->right);
      }
      return valid;
   }
}


// helper method to my_malloc that recursively traverses the tree to find a suitable node
void* my_malloc_helper(size_t size, tNode *cur) {
   // If the node is used, return to parent node with negative num 
   if(cur->used == 1)
      return (void*)-1;

   // if the size of the children is too small to store the obj in memory and the node is unused,
   // as well as there are no descendents from this node being used, if all is true, 
   // then set current node to used, to indicate it has stored the data
   if((cur->nSize/2 < size || cur->nSize == size) && cur->used == 0 && check_descendents_empty(cur) == 1){
      cur->used = 1;     
      return cur->ptr;

   // else if the object could fit into the children nodes, check the children nodes
   } else if (cur->nSize/2 >= size){
      void* ptr;
      // check left node
      ptr = my_malloc_helper(size, cur->left);
      // if nothing in left side works, check right side
      if (ptr == (void*)-1) {
         ptr = my_malloc_helper(size, cur->right);
      }
      return ptr;

   // if all else fails, this means no suitable node is found, return -1
   } else {
      return (void*)-1;
   }
}


// returns void pointer that indicates where the object we want to allocate memory for is located
void *my_malloc(size_t size) {
   tNode* root = &tree;
   return my_malloc_helper(size, root);
}


// helper method for free that recursively traverses the tree to free up the correct node
void free_helper(void* obj_ptr, tNode* cur) {
   if(cur != NULL) {
      if(obj_ptr == cur->ptr) {
         cur->used = 0;
         free_helper(obj_ptr, cur->left);
      } else {
         free_helper(obj_ptr, cur->left);
         free_helper(obj_ptr, cur->right);
      }
   }
}


// frees up allocated memory
void my_free(void *ptr) {
   free_helper(ptr, &tree);
}


// given function that draws heap, changed size param to double
// and changed spacing to preserve formatting
static void draw_box(FILE *stream, double size, int empty, int last) {
   int i;
   int pad = size / 2;

   fprintf(stream, "+---------------------+\n");

   if (!empty) fprintf(stream, "%c[%d;%dm", 0x1B, 7, 37);

   for (i=0; i<pad; i++)
   {
      fprintf(stream, "|                     |\n");
   }

   //this line has been updated for double
   fprintf(stream, "|      %f       |\n", size);

   for (i++; i<size; i++)
   {
      fprintf(stream, "|                     |\n");   
   }

   if (!empty) fprintf(stream, "%c[%dm", 0x1B, 0);

   if (last) fprintf(stream, "+---------------------+\n");
}

// function traverses tree and calls drawbox whenever it needs to
// base case checks if the node is used for storage or if it's children are null
// and thus it is in the final row of the tree. when size_left = 0 we know that the
// tree has been traversed. recusive case traverses tree. 
double print_heap(FILE* stream, tNode* cur, double size_left) {
   int last = 0;
   if(cur->used == 1 || cur->left == NULL){
      size_left -= cur->nSize;
      if (size_left == 0) {
         last = 1;
      }
      if (cur->used == 1) {
         draw_box(stream, (double)(cur->nSize)/MiB, 0, last);
      } else {
         draw_box(stream, (double)(cur->nSize)/MiB, 1, last);
      }
      return size_left;
   } else {
      size_left = print_heap(stream, cur->left, size_left);
      return print_heap(stream, cur->right, size_left);
   }
}

// function calls print_heap
void my_dump_mem(FILE *stream) {
   print_heap(stream, &tree, tree.nSize);
   
}

uint64_t my_address(void *ptr) {
   return ((((uint64_t) ptr) - ((uint64_t) mem)) / MiB);
}
