#include "b_plus_tree.h"
#include "memory_pool.h"
#include "types.h"

#include <tuple>
#include <iostream>
#include <array>
#include <unordered_map>
#include <cstring>

using namespace std;

bool myNullPtr = false;

Node::Node(int maxKeys)
{
  keys = new float[maxKeys];
  pointers = new Address[maxKeys + 1];

  for (int i = 0; i < maxKeys + 1; i++)
  {
    Address nullAddress{(void *)myNullPtr, 0};
    pointers[i] = nullAddress;
  }
  numKeys = 0;
}

BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool *disk, MemoryPool *index)
{
  size_t nodeBufferSize = blockSize - sizeof(bool) - sizeof(int);

  size_t sum = sizeof(Address);
  maxKeys = 0;

  while (sum + sizeof(Address) + sizeof(float) <= nodeBufferSize)
  {
    sum += (sizeof(Address) + sizeof(float));
    maxKeys += 1;
  }

  if (maxKeys == 0)
  {
    throw std::overflow_error("Error: Keys and pointers too large to fit into a node!");
  }

  rootAddress = nullptr;
  root = nullptr;

  nodeSize = blockSize;

  levels = 0;
  numNodes = 0;
  
  this->disk = disk;
  this->index = index;
}