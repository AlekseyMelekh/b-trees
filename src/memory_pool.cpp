#include "memory_pool.h"
#include "types.h"

#include <iostream>
#include <vector>
#include <tuple>
#include <cstring>

MemoryPool::MemoryPool(std::size_t maxPoolSize, std::size_t blockSize)
{
  this->maxPoolSize = maxPoolSize;
  this->blockSize = blockSize;
  this->sizeUsed = 0;
  this->actualSizeUsed = 0;
  this->allocated = 0;

  this->pool = operator new(maxPoolSize);
  std::memset(pool, '\0', maxPoolSize);
  this->block = nullptr;
  this->blockSizeUsed = 0;

  this->blocksAccessed = 0;
}


bool MemoryPool::allocateBlock()
{
  if (sizeUsed + blockSize <= maxPoolSize)
  {
    sizeUsed += blockSize;
    block = (char *)pool + allocated * blockSize; 
    blockSizeUsed = 0;  
    allocated += 1;
    return true;
  }
  else
  {
    std::cout << "Error: No memory left to allocate new block (" << sizeUsed << "/" << maxPoolSize << " used)." << '\n';
    return false;
  }
}

Address MemoryPool::allocate(std::size_t sizeRequired)
{
  if (sizeRequired > blockSize)
  {
    std::cout << "Error: Size required larger than block size (" << sizeRequired << " vs " << blockSize << ")! Increase block size to store data." << '\n';
    throw std::invalid_argument("Requested size too large!");
  }

  if (allocated == 0 || (blockSizeUsed + sizeRequired > blockSize))
  {
    bool isSuccessful = allocateBlock();
    if (!isSuccessful)
    {
      throw std::logic_error("Failed to allocate new block!");
    }
  }

  short int offset = blockSizeUsed;

  blockSizeUsed += sizeRequired;
  actualSizeUsed += sizeRequired;

  Address recordAddress = {block, offset};

  return recordAddress;
}

bool MemoryPool::deallocate(Address address, std::size_t sizeToDelete)
{
  try
  {
    void *addressToDelete = (char *)address.blockAddress + address.offset;
    std::memset(addressToDelete, '\0', sizeToDelete);

    actualSizeUsed -= sizeToDelete;

    unsigned char testBlock[blockSize];
    memset(testBlock, '\0', blockSize);

    if (memcmp(testBlock, address.blockAddress, blockSize) == 0)
    {
      sizeUsed -= blockSize;
      allocated--;
    }

    return true;
  }
  catch (...)
  {
    std::cout << "Error: Could not remove record/block at given address (" << address.blockAddress << ") and offset (" << address.offset << ")." << '\n';
    return false;
  };
}

void *MemoryPool::loadFromDisk(Address address, std::size_t size)
{
  void *mainMemoryAddress = operator new(size);
  std::memcpy(mainMemoryAddress, (char *)address.blockAddress + address.offset, size);

  blocksAccessed++;

  return mainMemoryAddress;
}

Address MemoryPool::saveToDisk(void *itemAddress, std::size_t size)
{
  Address diskAddress = allocate(size);
  std::memcpy((char *)diskAddress.blockAddress + diskAddress.offset, itemAddress, size);

  blocksAccessed++;

  return diskAddress;
}

Address MemoryPool::saveToDisk(void *itemAddress, std::size_t size, Address diskAddress)
{
  std::memcpy((char *)diskAddress.blockAddress + diskAddress.offset, itemAddress, size);

  blocksAccessed++;

  return diskAddress;
}

MemoryPool::~MemoryPool(){};