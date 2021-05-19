#ifndef B_PLUS_TREE_H
#define B_PLUS_TREE_H

#include "types.h"
#include "memory_pool.h"

#include <cstddef>
#include <array>

class Node
{
private:
  Address *pointers;     
  float *keys;         
  int numKeys;        
  bool isLeaf;   
  friend class BPlusTree; 

public:

  Node(int maxKeys);
};

class BPlusTree
{
private:
  MemoryPool *disk;  
  MemoryPool *index; 
  Node *root; 
  void *rootAddress; 
  int maxKeys;
  int levels;
  int numNodes; 
  std::size_t nodeSize;

  void insertInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  void removeInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress);

  Node *findParent(Node *, Node *, float lowerBoundKey);

public:

  BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index);

  void search(float lowerBoundKey, float upperBoundKey);

  void insert(Address address, float key);

  Address insertLL(Address LLHead, Address address, float key);

  void display(Node *, int level);

  void displayNode(Node *node);

  void displayBlock(void *block);

  void displayLL(Address LLHeadAddress);

  int remove(float key);

  void removeLL(Address LLHeadAddress);

  Node *getRoot()
  {
    return root;
  };

  int getLevels();

  int getNumNodes()
  {
    return numNodes;
  }

  int getMaxKeys()
  {
    return maxKeys;
  }
};

#endif