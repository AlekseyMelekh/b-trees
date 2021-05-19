#include "b_plus_tree.h"
#include "types.h"

#include <iostream>
#include <cstring>

using namespace std;

void BPlusTree::displayNode(Node *node)
{
  int i = 0;
  std::cout << "|";
  for (int i = 0; i < node->numKeys; i++)
  {
    std::cout << node->pointers[i].blockAddress << " | ";
    std::cout << node->keys[i] << " | ";
  }

  if (node->pointers[node->numKeys].blockAddress == nullptr) {
    std::cout << " Null |";
  }
  else {
    std::cout << node->pointers[node->numKeys].blockAddress << "|";
  }

  for (int i = node->numKeys; i < maxKeys; i++)
  {
    std::cout << " x |";    
    std::cout << "  Null  |"; 
  }

  std::cout << endl;
}

void BPlusTree::displayBlock(void *blockAddress)
{
  void *block = operator new(nodeSize);
  std::memcpy(block, blockAddress, nodeSize);

  unsigned char testBlock[nodeSize];
  memset(testBlock, '\0', nodeSize);

  if (memcmp(testBlock, block, nodeSize) == 0)
  {
    std::cout << "Empty block!" << '\n';
    return;
  }

  unsigned char *blockChar = (unsigned char *)block;

  int i = 0;
  while (i < nodeSize)
  {
    void *recordAddress = operator new(sizeof(Record));
    std::memcpy(recordAddress, blockChar, sizeof(Record));

    Record *record = (Record *)recordAddress;

    std::cout << "[" << record->tconst << "|" << record->averageRating << "|" << record->numVotes << "]  ";
    blockChar += sizeof(Record);
    i += sizeof(Record);
  }
  
}

void BPlusTree::display(Node *cursorDiskAddress, int level)
{
  Address cursorMainMemoryAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorMainMemoryAddress, nodeSize);

  if (cursor != nullptr)
  {
    for (int i = 0; i < level; i++)
    {
      std::cout << "   ";
    }
    std::cout << " level " << level << ": ";

    displayNode(cursor);

    if (cursor->isLeaf != true)
    {
      for (int i = 0; i < cursor->numKeys + 1; i++)
      {
        Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

        display((Node *)mainMemoryNode, level + 1);
      }
    }
  }
}

void BPlusTree::displayLL(Address LLHeadAddress)
{
  Node *head = (Node *)index->loadFromDisk(LLHeadAddress, nodeSize);

  for (int i = 0; i < head->numKeys; i++)
  {

    std::cout << "\nData block accessed. Content is -----";
    displayBlock(head->pointers[i].blockAddress);
    std::cout << endl;

    Record result = *(Record *)(disk->loadFromDisk(head->pointers[i], sizeof(Record)));
    std::cout << result.tconst << " | ";


  }

  for (int i = head->numKeys; i < maxKeys; i++)
  {
    std::cout << "x | ";
  }
  
  if (head->pointers[head->numKeys].blockAddress == nullptr)
  {
    std::cout << "End of linked list" << endl;
    return;
  }

  if (head->pointers[head->numKeys].blockAddress != nullptr)
  {
    displayLL(head->pointers[head->numKeys]);
  }
}
