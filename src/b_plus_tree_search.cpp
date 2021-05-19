#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

void BPlusTree::search(float lowerBoundKey, float upperBoundKey)
{ 
  if (rootAddress == nullptr)
  {
    throw std::logic_error("Tree is empty!");
  }
  else
  {
    Address rootDiskAddress{rootAddress, 0};
    root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);

    std::cout << "Index node accessed. Content is -----";
    displayNode(root);

    Node *cursor = root;

    bool found = false;

    while (cursor->isLeaf == false)
    {
      for (int i = 0; i < cursor->numKeys; i++)
      {
        if (lowerBoundKey < cursor->keys[i])
        {
          cursor = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

          std::cout << "Index node accessed. Content is -----";
          displayNode(cursor);

          break;
        }
        if (i == cursor->numKeys - 1)
        {
          cursor = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

          std::cout << "Index node accessed. Content is -----";
          displayNode(cursor);
          break;
        }
      }
    }

    bool stop = false;

    while (stop == false)
    {
      int i;
      for (i = 0; i < cursor->numKeys; i++)
      {
        if (cursor->keys[i] > upperBoundKey)
        {
          stop = true;
          break;
        }
        if (cursor->keys[i] >= lowerBoundKey && cursor->keys[i] <= upperBoundKey)
        {
          std::cout << "Index node (LLNode) accessed. Content is -----";
          displayNode(cursor);

          std::cout << endl;
          std::cout << "LLNode: tconst for average rating: " << cursor->keys[i] << " > ";          

          displayLL(cursor->pointers[i]);
        }
      }

      if (cursor->pointers[cursor->numKeys].blockAddress != nullptr && cursor->keys[i] != upperBoundKey)
      {
        cursor = (Node *)index->loadFromDisk(cursor->pointers[cursor->numKeys], nodeSize);

        std::cout << "Index node accessed. Content is -----";
        displayNode(cursor);

      }
      else
      {
        stop = true;
      }
    }
  }
  return;
}