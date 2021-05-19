#include "b_plus_tree.h"
#include "types.h"

#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

void BPlusTree::insert(Address address, float key)
{
  if (rootAddress == nullptr)
  {
    Node *LLNode = new Node(maxKeys);
    LLNode->keys[0] = key;
    LLNode->isLeaf = false; 
    LLNode->numKeys = 1;
    LLNode->pointers[0] = address;

    Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);

    root = new Node(maxKeys);
    root->keys[0] = key;
    root->isLeaf = true; 
    root->numKeys = 1;
    root->pointers[0] = LLNodeAddress; 

    rootAddress = index->saveToDisk(root, nodeSize).blockAddress;
  }
  else
  {
    Node *cursor = root;
    Node *parent;                          
    void *parentDiskAddress = rootAddress;
    void *cursorDiskAddress = rootAddress;

    while (cursor->isLeaf == false)
    {

      parent = cursor;
      parentDiskAddress = cursorDiskAddress;

      for (int i = 0; i < cursor->numKeys; i++)
      {
        if (key < cursor->keys[i])
        {
          Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);

          cursorDiskAddress = cursor->pointers[i].blockAddress;

          cursor = mainMemoryNode;
          break;
        }
        if (i == cursor->numKeys - 1)
        {
          Node *mainMemoryNode = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

          cursorDiskAddress = cursor->pointers[i + 1].blockAddress;

          cursor = (Node *)mainMemoryNode;
          break;
        }
      }
    }

    if (cursor->numKeys < maxKeys)
    {
      int i = 0;
      while (key > cursor->keys[i] && i < cursor->numKeys)
      {
        i++;
      }

      if (cursor->keys[i] == key)
      {
        cursor->pointers[i] = insertLL(cursor->pointers[i], address, key);
      }
      else
      {
        Address next = cursor->pointers[cursor->numKeys];

        for (int j = cursor->numKeys; j > i; j--)
        {
          cursor->keys[j] = cursor->keys[j - 1];
          cursor->pointers[j] = cursor->pointers[j - 1];
        }

        cursor->keys[i] = key;

        Node *LLNode = new Node(maxKeys);
        LLNode->keys[0] = key;
        LLNode->isLeaf = false; 
        LLNode->numKeys = 1;
        LLNode->pointers[0] = address;

        Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);

        cursor->pointers[i] = LLNodeAddress;
        cursor->numKeys++;
  
        cursor->pointers[cursor->numKeys] = next;

        Address cursorOriginalAddress{cursorDiskAddress, 0};
        index->saveToDisk(cursor, nodeSize, cursorOriginalAddress);
      }
    }
    else
    {
      Node *newLeaf = new Node(maxKeys);

      float tempKeyList[maxKeys + 1];

      Address tempPointerList[maxKeys + 1];
      Address next = cursor->pointers[cursor->numKeys];

      int i = 0;
      for (i = 0; i < maxKeys; i++)
      {
        tempKeyList[i] = cursor->keys[i];
        tempPointerList[i] = cursor->pointers[i];
      }

      i = 0;
      while (key > tempKeyList[i] && i < maxKeys)
      {
        i++;
      }

      if (i < cursor->numKeys) {
        if (cursor->keys[i] == key)
        {
          cursor->pointers[i] = insertLL(cursor->pointers[i], address, key);
          return;
        } 
      }

      for (int j = maxKeys; j > i; j--)
      {
        tempKeyList[j] = tempKeyList[j - 1];
        tempPointerList[j] = tempPointerList[j - 1];
      }

      tempKeyList[i] = key;

      Node *LLNode = new Node(maxKeys);
      LLNode->keys[0] = key;
      LLNode->isLeaf = false;
      LLNode->numKeys = 1;
      LLNode->pointers[0] = address; 

      Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);
      tempPointerList[i] = LLNodeAddress;
      
      newLeaf->isLeaf = true; 

      cursor->numKeys = (maxKeys + 1) / 2;
      newLeaf->numKeys = (maxKeys + 1) - ((maxKeys + 1) / 2);

      newLeaf->pointers[newLeaf->numKeys] = next;

      for (i = 0; i < cursor->numKeys; i++)
      {
        cursor->keys[i] = tempKeyList[i];
        cursor->pointers[i] = tempPointerList[i];
      }

      for (int j = 0; j < newLeaf->numKeys; i++, j++)
      {
        newLeaf->keys[j] = tempKeyList[i];
        newLeaf->pointers[j] = tempPointerList[i];
      }

      Address newLeafAddress = index->saveToDisk(newLeaf, nodeSize);

      cursor->pointers[cursor->numKeys] = newLeafAddress;

      for (int i = cursor->numKeys; i < maxKeys; i++) {
        cursor->keys[i] = float();
      }
      for (int i = cursor->numKeys+1; i < maxKeys + 1; i++) {
        Address nullAddress{nullptr, 0};
        cursor->pointers[i] = nullAddress;
      }

      Address cursorOriginalAddress{cursorDiskAddress, 0};
      index->saveToDisk(cursor, nodeSize, cursorOriginalAddress);

      if (cursor == root)
      {
        Node *newRoot = new Node(maxKeys);

        newRoot->keys[0] = newLeaf->keys[0];

        Address cursorDisk{cursorDiskAddress, 0};

        newRoot->pointers[0] = cursorDisk;
        newRoot->pointers[1] = newLeafAddress;

        newRoot->isLeaf = false;
        newRoot->numKeys = 1;

        Address newRootAddress = index->saveToDisk(newRoot, nodeSize);

        rootAddress = newRootAddress.blockAddress;
        root = newRoot;
      }
      else
      {
        insertInternal(newLeaf->keys[0], (Node *)parentDiskAddress, (Node *)newLeafAddress.blockAddress);
      }
    }
  }

  numNodes = index->getAllocated();
}

void BPlusTree::insertInternal(float key, Node *cursorDiskAddress, Node *childDiskAddress)
{
  Address cursorAddress{cursorDiskAddress, 0};
  Node *cursor = (Node *)index->loadFromDisk(cursorAddress, nodeSize);

  if (cursorDiskAddress == rootAddress)
  {
    root = cursor;
  }

  Address childAddress{childDiskAddress, 0};
  Node *child = (Node *)index->loadFromDisk(childAddress, nodeSize);

  if (cursor->numKeys < maxKeys)
  {
    int i = 0;
    while (key > cursor->keys[i] && i < cursor->numKeys)
    {
      i++;
    }

    for (int j = cursor->numKeys; j > i; j--)
    {
      cursor->keys[j] = cursor->keys[j - 1];
    }

    for (int j = cursor->numKeys + 1; j > i + 1; j--)
    {
      cursor->pointers[j] = cursor->pointers[j - 1];
    }

    cursor->keys[i] = key;
    cursor->numKeys++;

    Address childAddress{childDiskAddress, 0};
    cursor->pointers[i + 1] = childAddress;

    Address cursorAddress{cursorDiskAddress, 0};
    index->saveToDisk(cursor, nodeSize, cursorAddress);
  }
  else
  {
    Node *newInternal = new Node(maxKeys);

    float tempKeyList[maxKeys + 1];
    Address tempPointerList[maxKeys + 2];

    for (int i = 0; i < maxKeys; i++)
    {
      tempKeyList[i] = cursor->keys[i];
    }

    for (int i = 0; i < maxKeys + 1; i++)
    {
      tempPointerList[i] = cursor->pointers[i];
    }

    int i = 0;
    while (key > tempKeyList[i] && i < maxKeys)
    {
      i++;
    }

    int j;
    for (int j = maxKeys; j > i; j--)
    {
      tempKeyList[j] = tempKeyList[j - 1];
    }

    tempKeyList[i] = key;

    for (int j = maxKeys + 1; j > i + 1; j--)
    {
      tempPointerList[j] = tempPointerList[j - 1];
    }

    Address childAddress = {childDiskAddress, 0};
    tempPointerList[i + 1] = childAddress;
    newInternal->isLeaf = false; 

    cursor->numKeys = (maxKeys + 1) / 2;
    newInternal->numKeys = maxKeys - (maxKeys + 1) / 2;

    for (int i = 0; i < cursor->numKeys; i++)
    {
      cursor->keys[i] = tempKeyList[i];
    }
    
    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys; i++, j++)
    {
      newInternal->keys[i] = tempKeyList[j];
    }

    for (i = 0, j = cursor->numKeys + 1; i < newInternal->numKeys + 1; i++, j++)
    {
      newInternal->pointers[i] = tempPointerList[j];
    }

    for (int i = cursor->numKeys; i < maxKeys; i++) 
    {
      cursor->keys[i] = float();
    }

    for (int i = cursor->numKeys + 1; i < maxKeys + 1; i++)
    {
      Address nullAddress{nullptr, 0};
      cursor->pointers[i] = nullAddress;
    }

    cursor->pointers[cursor->numKeys] = childAddress;

    Address cursorAddress{cursorDiskAddress, 0};
    index->saveToDisk(cursor, nodeSize, cursorAddress);

    Address newInternalDiskAddress = index->saveToDisk(newInternal, nodeSize);

    if (cursor == root)
    {
      Node *newRoot = new Node(nodeSize);
      newRoot->keys[0] = cursor->keys[cursor->numKeys];

      Address cursorAddress = {cursorDiskAddress, 0};
      newRoot->pointers[0] = cursorAddress;
      newRoot->pointers[1] = newInternalDiskAddress;

      newRoot->isLeaf = false;
      newRoot->numKeys = 1;

      root = newRoot;

      Address newRootAddress = index->saveToDisk(root, nodeSize);

      rootAddress = newRootAddress.blockAddress;
    }
    else
    {
      Node *parentDiskAddress = findParent((Node *)rootAddress, cursorDiskAddress, cursor->keys[0]);

      insertInternal(tempKeyList[cursor->numKeys], parentDiskAddress, (Node *)newInternalDiskAddress.blockAddress);
    }
  }
}

Address BPlusTree::insertLL(Address LLHead, Address address, float key)
{
  Node *head = (Node *)index->loadFromDisk(LLHead, nodeSize);

  if (head->numKeys < maxKeys)
  {

    for (int i = head->numKeys; i > 0; i--)
    {
      head->keys[i] = head->keys[i - 1];
    }

    for (int i = head->numKeys + 1; i > 0; i--)

    {
      head->pointers[i] = head->pointers[i - 1];
    }

    head->keys[0] = key;
    head->pointers[0] = address; 
    head->numKeys++;
    
    LLHead = index->saveToDisk((void *)head, nodeSize, LLHead);

    return LLHead;
  }
  else
  {
    Node *LLNode = new Node(maxKeys);
    LLNode->isLeaf = false;
    LLNode->keys[0] = key;
    LLNode->numKeys = 1;

    LLNode->pointers[0] = address;

    LLNode->pointers[1] = LLHead;

    Address LLNodeAddress = index->saveToDisk((void *)LLNode, nodeSize);

    return LLNodeAddress;
  }
}