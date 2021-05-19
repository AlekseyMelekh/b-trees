#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include "types.h"

#include <vector>
#include <unordered_map>
#include <tuple>

class MemoryPool
{
public:
  MemoryPool(std::size_t maxPoolSize, std::size_t blockSize);

  bool allocateBlock();

  Address allocate(std::size_t sizeRequired);

  bool deallocate(Address address, std::size_t sizeToDelete);

  void *loadFromDisk(Address address, std::size_t size);

  Address saveToDisk(void *itemAddress, std::size_t size);

  Address saveToDisk(void *itemAddress, std::size_t size, Address diskAddress);

  std::size_t getMaxPoolSize() const
  {
    return maxPoolSize;
  }

  std::size_t getBlockSize() const
  {
    return blockSize;
  };

  std::size_t getBlockSizeUsed() const
  {
    return blockSizeUsed;
  };

  std::size_t getSizeUsed() const
  {
    return sizeUsed;
  }

  std::size_t getActualSizeUsed() const
  {
    return actualSizeUsed;
  }

  int getAllocated() const
  {
    return allocated;
  };

  int getBlocksAccessed() const
  {
    return blocksAccessed;
  }

  int resetBlocksAccessed()
  {
    int tempBlocksAccessed = blocksAccessed;
    blocksAccessed = 0;
    return tempBlocksAccessed;
  }

  ~MemoryPool();

private:
  std::size_t maxPoolSize;   
  std::size_t blockSize;    
  std::size_t sizeUsed;      
  std::size_t actualSizeUsed; 
  std::size_t blockSizeUsed;  

  int allocated;   
  int blocksAccessed;

  void *pool; 
  void *block; 
};

#endif