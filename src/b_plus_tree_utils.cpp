#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

Node *BPlusTree::findParent(Node *cursorDiskAddress, Node *childDiskAddress, float lowerBoundKey)
{
  Address cursorAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorAddress, nodeSize);

  if (cursor->isLeaf)
  {
    return nullptr;
  }

  Node *parentDiskAddress = cursorDiskAddress;

  while (cursor->isLeaf == false)
  {
    for (int i = 0; i < cursor->numKeys + 1; i++)
    {
      if (cursor->pointers[i].blockAddress == childDiskAddress)
      {
        return parentDiskAddress;
      }
    }

    for (int i = 0; i < cursor->numKeys; i++)
    {
      if (lowerBoundKey < cursor->keys[i])
      {
        Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

        parentDiskAddress = (Node *)cursor->pointers[i].blockAddress;

        cursor = (Node *)mainMemoryNode;
        break;
      }

      if (i == cursor->numKeys - 1)
      {
        Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

        parentDiskAddress = (Node *)cursor->pointers[i + 1].blockAddress;

        cursor = (Node *)mainMemoryNode;
        break;
      }
    }
  }

  return nullptr;
}


int BPlusTree::getLevels() {

  if (rootAddress == nullptr) {
    return 0;
  }

  Address rootDiskAddress{rootAddress, 0};
  root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);
  Node *cursor = root;

  levels = 1;

  while (!cursor->isLeaf) {
    cursor = (Node *)index->loadFromDisk(cursor->pointers[0], nodeSize);
    levels++;
  }

  levels++;

  return levels;
}
