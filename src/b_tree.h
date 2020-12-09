#ifndef COURSE_PROJECT_B_TREE_H
#define COURSE_PROJECT_B_TREE_H

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <vector>

template <typename TKey> class BTree
{
  private:
    bool binary;
    int parameter;
    int rootIndex;
    int lastIndex;
    std::string storageLocation;
    struct Node
    {
        int size;
        bool leaf;
        int index;
        TKey *keys = nullptr;
        int *children = nullptr;
        ~Node()
        {
            delete[] keys;
            delete[] children;
        }
    };
    std::string GetTempFileName(const std::string &storageLocation, int index) const;
    void ReadNode(std::shared_ptr<Node> &node, int index) const;
    void WriteNode(std::shared_ptr<Node> node);
    void CreateNode(std::shared_ptr<Node> &node);
    void Insert(std::shared_ptr<Node> node, const TKey &key);
    void RemoveFromLeaf(std::shared_ptr<Node> leaf, int keyIndex, bool rootCall);
    void RemoveFromInternal(std::shared_ptr<Node> node, int keyIndex, bool rootCall);
    TKey RemoveMin(std::shared_ptr<Node> node);
    void Erase(std::shared_ptr<Node> node, const TKey &key);
    bool Find(std::shared_ptr<Node> node, const TKey &key) const;
    std::shared_ptr<Node> Split(std::shared_ptr<Node> parent, std::shared_ptr<Node> childPtr, int child);
    void Merge(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart, std::shared_ptr<Node> secondPart,
               int firstPartIndex, bool direction);
    void KeyRepositioning(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart,
                          std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction);
    bool EraseResolve(std::shared_ptr<Node> parent, std::shared_ptr<Node> child, int childIndex);
    void BackupTree() const;
    void Print(std::shared_ptr<Node> node, int level) const;

  public:
    BTree(int parameter, bool binary, std::string storageLocation);
    void Insert(TKey key);
    void Erase(TKey key);
    bool Find(TKey key) const;
    void Print() const;
    ~BTree();
};

template <typename TKey> BTree<TKey>::BTree(int parameter, bool binary, std::string storageLocation)
{
    FILE *input;
    fopen_s(&input, (storageLocation + "/treeParams.bin").c_str(), "rb");
    if (input)
    {
        fread(&this->parameter, sizeof(int), 1, input);
        fread(&rootIndex, sizeof(int), 1, input);
        fread(&lastIndex, sizeof(int), 1, input);
        fread(&this->binary, sizeof(bool), 1, input);
        this->storageLocation = storageLocation;
        fclose(input);
    }
    else
    {
        lastIndex = 0;
        rootIndex = 0;
        this->parameter = parameter;
        this->storageLocation = storageLocation;
        this->binary = binary;
        std::shared_ptr<Node> root = nullptr;
        CreateNode(root);
        WriteNode(root);
        BackupTree();
    }
}

template <typename TKey> BTree<TKey>::~BTree()
{
    BackupTree();
}

template <typename TKey>
std::string BTree<TKey>::GetTempFileName(const std::string &storageLocation, int index) const
{
    return (storageLocation + "/node" + std::to_string(index) + ".bin");
}

template <typename TKey> void BTree<TKey>::BackupTree() const
{
    FILE *output;
    fopen_s(&output, (storageLocation + "/treeParams.bin").c_str(), "wb");
    fwrite(&parameter, sizeof(int), 1, output);
    fwrite(&rootIndex, sizeof(int), 1, output);
    fwrite(&lastIndex, sizeof(int), 1, output);
    fwrite(&binary, sizeof(bool), 1, output);
    fclose(output);
}

template <typename TKey> void BTree<TKey>::ReadNode(std::shared_ptr<Node> &node, int index) const
{
    node = std::shared_ptr<Node>(new Node());
    if (binary)
    {
        FILE *fileIn;
        if (int x = fopen_s(&fileIn, GetTempFileName(storageLocation, index).c_str(), "rb"))
            return;
        node->keys = new TKey[2 * parameter - 1];
        node->index = index;
        fread(&node->size, sizeof(int), 1, fileIn);
        fread(&node->leaf, sizeof(bool), 1, fileIn);
        fread(node->keys, sizeof(TKey), node->size, fileIn);
        if (!node->leaf)
        {
            node->children = new int[2 * parameter];
            fread(node->children, sizeof(int), node->size + 1, fileIn);
        }
        fclose(fileIn);
    }
    else
    {
        std::ifstream fileIn(GetTempFileName(storageLocation, index));
        fileIn >> node->size;
        fileIn >> node->leaf;
        node->index = index;
        node->keys = new TKey[2 * parameter - 1];
        for (int i = 0; i < node->size; i++)
            fileIn >> node->keys[i];
        if (!node->leaf)
        {
            node->children = new int[2 * parameter];
            for (int i = 0; i < node->size + 1; i++)
                fileIn >> node->children[i];
        }
        fileIn.close();
    }
}

template <typename TKey> void BTree<TKey>::WriteNode(std::shared_ptr<Node> node)
{
    if (node->size == 0 && node->index != rootIndex)
    {
        remove(GetTempFileName(storageLocation, node->index).c_str());
        return;
    }
    if (binary)
    {
        FILE *fileOut;
        fopen_s(&fileOut, GetTempFileName(storageLocation, node->index).c_str(), "wb");
        fwrite(&node->size, sizeof(int), 1, fileOut);
        fwrite(&node->leaf, sizeof(bool), 1, fileOut);
        fwrite(node->keys, sizeof(TKey), node->size, fileOut);
        if (!node->leaf)
            fwrite(node->children, sizeof(int), node->size + 1, fileOut);
        fclose(fileOut);
    }
    else
    {
        std::ofstream fileOut(GetTempFileName(storageLocation, node->index));
        fileOut << node->size << " ";
        fileOut << node->leaf << " ";
        for (int i = 0; i < node->size; i++)
            fileOut << node->keys[i] << " ";
        if (!node->leaf)
        {
            for (int i = 0; i < node->size + 1; i++)
                fileOut << node->children[i] << " ";
        }
        fileOut.close();
    }
}

template <typename TKey> void BTree<TKey>::CreateNode(std::shared_ptr<Node> &node)
{
    node = std::shared_ptr<Node>(new Node());
    node->size = 0;
    node->index = lastIndex++;
    node->leaf = true;
    node->keys = new TKey[2 * parameter - 1];
    node->children = new int[2 * parameter];
}

template <typename TKey>
std::shared_ptr<typename BTree<TKey>::Node> BTree<TKey>::Split(std::shared_ptr<Node> parent,
                                                                     std::shared_ptr<Node> childPtr, int child)
{
    std::shared_ptr<Node> firstPart, secondPart = nullptr;
    firstPart = childPtr;
    CreateNode(secondPart);
    secondPart->leaf = firstPart->leaf;
    for (int keyIndex = 0; keyIndex < parameter - 1; keyIndex++)
    {
        secondPart->keys[keyIndex] = firstPart->keys[keyIndex + parameter];
    }
    if (!firstPart->leaf)
        for (int childIndex = 0; childIndex < parameter; childIndex++)
            secondPart->children[childIndex] = firstPart->children[childIndex + parameter];
    firstPart->size = parameter - 1;
    secondPart->size = parameter - 1;
    for (int keyIndex = parent->size; keyIndex > child; keyIndex--)
    {
        parent->keys[keyIndex] = parent->keys[keyIndex - 1];
    }
    parent->keys[child] = firstPart->keys[parameter - 1];
    for (int childIndex = parent->size + 1; childIndex > child + 1; childIndex--)
        parent->children[childIndex] = parent->children[childIndex - 1];
    parent->children[child + 1] = secondPart->index;
    parent->size++;
    WriteNode(parent);
    WriteNode(firstPart);
    WriteNode(secondPart);
    return secondPart;
}

template <typename TKey>
void BTree<TKey>::Merge(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart,
                           std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction)
{
    if (direction)
    {
        firstPart->keys[firstPart->size] = parent->keys[firstPartIndex];
        for (int i = 0; i < secondPart->size; i++)
        {
            firstPart->keys[firstPart->size + i + 1] = secondPart->keys[i];
            if (!firstPart->leaf)
                firstPart->children[firstPart->size + i + 1] = secondPart->children[i];
        }
        if (!firstPart->leaf)
            firstPart->children[firstPart->size + secondPart->size + 1] = secondPart->children[secondPart->size];
        for (int i = firstPartIndex; i < parent->size - 1; i++)
        {
            parent->keys[i] = parent->keys[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
    }
    else
    {
        for (int i = 0; i < firstPart->size; i++)
        {
            firstPart->keys[i + secondPart->size + 1] = firstPart->keys[i];
            if (!firstPart->leaf)
                firstPart->children[i + secondPart->size + 1] = firstPart->children[i];
        }
        if (!firstPart->leaf)
            firstPart->children[firstPart->size + secondPart->size + 1] = firstPart->children[firstPart->size];
        for (int i = 0; i < secondPart->size; i++)
        {
            firstPart->keys[i] = secondPart->keys[i];
            if (!firstPart->leaf)
                firstPart->children[i] = secondPart->children[i];
        }
        firstPart->keys[firstPart->size] = parent->keys[firstPartIndex - 1];
        if (!firstPart->leaf)
            firstPart->children[firstPart->size] = secondPart->children[secondPart->size];
        for (int i = firstPartIndex - 1; i < parent->size - 1; i++)
        {
            parent->keys[i] = parent->keys[i + 1];
            parent->children[i] = parent->children[i + 1];
        }
        parent->children[parent->size - 1] = parent->children[parent->size];
    }
    firstPart->size = firstPart->size + secondPart->size + 1;
    parent->size--;
    secondPart->size = 0;
    WriteNode(parent);
    WriteNode(firstPart);
    WriteNode(secondPart);
}

template <typename TKey>
void BTree<TKey>::KeyRepositioning(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart,
                                      std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction)
{
    if (direction)
    {
        firstPart->keys[firstPart->size] = parent->keys[firstPartIndex];
        if (!firstPart->leaf)
            firstPart->children[firstPart->size + 1] = secondPart->children[0];
        firstPart->size++;
        parent->keys[firstPartIndex] = secondPart->keys[0];
        for (int i = 1; i < secondPart->size; i++)
        {
            secondPart->keys[i - 1] = secondPart->keys[i];
            if (!firstPart->leaf)
                secondPart->children[i - 1] = secondPart->children[i];
        }
        if (!firstPart->leaf)
            secondPart->children[secondPart->size - 1] = secondPart->children[secondPart->size];
        secondPart->size--;
    }
    else
    {
        for (int i = firstPart->size; i >= 1; i--)
        {
            firstPart->keys[i] = firstPart->keys[i - 1];
            if (!firstPart->leaf)
                firstPart->children[i + 1] = firstPart->children[i];
        }
        if (!firstPart->leaf)
        {
            firstPart->children[1] = firstPart->children[0];
            firstPart->children[0] = secondPart->children[secondPart->size];
        }
        firstPart->keys[0] = parent->keys[firstPartIndex - 1];
        firstPart->size++;
        parent->keys[firstPartIndex - 1] = secondPart->keys[secondPart->size - 1];
        secondPart->size--;
    }
    WriteNode(parent);
    WriteNode(firstPart);
    WriteNode(secondPart);
}

template <typename TKey>
bool BTree<TKey>::EraseResolve(std::shared_ptr<Node> parent, std::shared_ptr<Node> child, int childIndex)
{
    std::shared_ptr<Node> predecessor = nullptr, successor = nullptr;
    if (childIndex)
    {
        ReadNode(predecessor, parent->children[childIndex - 1]);
        if (predecessor->size != parameter - 1)
        {
            KeyRepositioning(parent, child, predecessor, childIndex, 0);
            return true;
        }
    }
    if (childIndex != parent->size)
    {
        ReadNode(successor, parent->children[childIndex + 1]);
        if (successor->size != parameter - 1)
        {
            KeyRepositioning(parent, child, successor, childIndex, 1);
            return false;
        }
    }
    bool result;
    if (predecessor != nullptr)
    {
        Merge(parent, child, predecessor, childIndex, 0);
        result = true;
    }
    else if (successor != nullptr)
    {
        Merge(parent, child, successor, childIndex, 1);
        result = false;
    }
    return result;
}

template <typename TKey> void BTree<TKey>::RemoveFromLeaf(std::shared_ptr<Node> leaf, int keyIndex, bool rootCall)
{
    for (int i = keyIndex; i < leaf->size - 1; i++)
    {
        leaf->keys[i] = leaf->keys[i + 1];
    }
    leaf->size--;

    WriteNode(leaf);
}

template <typename TKey>
void BTree<TKey>::RemoveFromInternal(std::shared_ptr<Node> node, int keyIndex, bool rootCall)
{
    TKey key = node->keys[keyIndex];
    std::shared_ptr<Node> child;
    ReadNode(child, node->children[keyIndex + 1]);
    if (child->size == parameter - 1)
        if (EraseResolve(node, child, keyIndex + 1))
        {
            if (rootCall)
            {
                if (node->size == 0)
                {
                    rootIndex = node->children[0];
                    WriteNode(node);
                }
            }
            Erase(child, key);
            return;
        }
    TKey result = RemoveMin(child);
    node->keys[keyIndex] = result;
    if (rootCall)
    {
        if (node->size == 0)
        {
            rootIndex = node->children[0];
        }
    }
    WriteNode(node);
}

template <typename TKey> TKey BTree<TKey>::RemoveMin(std::shared_ptr<Node> node)
{
    if (node->leaf)
    {
        TKey result = node->keys[0];
        RemoveFromLeaf(node, 0, false);
        return result;
    }
    else
    {
        std::shared_ptr<Node> child;
        ReadNode(child, node->children[0]);
        if (child->size == parameter - 1)
            EraseResolve(node, child, 0);
        return RemoveMin(child);
    }
}

template <typename TKey> bool BTree<TKey>::Find(std::shared_ptr<Node> node, const TKey &key) const
{
    int keyIndex = std::upper_bound(node->keys, node->keys + node->size, key) - node->keys - 1;
    if (keyIndex != -1 && node->keys[keyIndex] == key)
        return true;
    if (node->leaf)
        return false;
    std::shared_ptr<Node> child;
    ReadNode(child, node->children[keyIndex + 1]);
    return Find(child, key);
}

template <typename TKey> void BTree<TKey>::Insert(std::shared_ptr<Node> node, const TKey &key)
{
    int keyIndex;
    if (node->leaf)
    {
        for (keyIndex = node->size - 1; keyIndex >= 0 && key < node->keys[keyIndex]; keyIndex--)
        {
            node->keys[keyIndex + 1] = node->keys[keyIndex];
        }
        node->keys[keyIndex + 1] = key;
        node->size++;
        WriteNode(node);
    }
    else
    {
        keyIndex = std::upper_bound(node->keys, node->keys + node->size, key) - node->keys - 1;
        int childIndex = keyIndex + 1;
        std::shared_ptr<Node> child;
        ReadNode(child, node->children[childIndex]);
        if (child->size == 2 * parameter - 1)
        {
            std::shared_ptr<Node> secondChild = Split(node, child, childIndex);
            if (key < node->keys[childIndex])
            {
                Insert(child, key);
            }
            else
            {
                Insert(secondChild, key);
            }
        }
        else
        {
            Insert(child, key);
        }
    }
}

template <typename TKey> void BTree<TKey>::Erase(std::shared_ptr<Node> node, const TKey &key)
{
    int keyIndex;
    keyIndex = std::upper_bound(node->keys, node->keys + node->size, key) - node->keys - 1;
    if (keyIndex != -1 && node->keys[keyIndex] == key)
    {

        if (node->leaf)
            RemoveFromLeaf(node, keyIndex, false);
        else
            RemoveFromInternal(node, keyIndex, false);
    }
    else if (!node->leaf)
    {
        std::shared_ptr<Node> child;
        ReadNode(child, node->children[keyIndex + 1]);
        if (child->size == parameter - 1)
            EraseResolve(node, child, keyIndex + 1);
        Erase(child, key);
    }
}

template <typename TKey> bool BTree<TKey>::Find(TKey key) const
{
    std::shared_ptr<Node> root;
    ReadNode(root, rootIndex);
    return Find(root, key);
}

template <typename TKey> void BTree<TKey>::Insert(TKey key)
{
    std::shared_ptr<Node> root;
    ReadNode(root, rootIndex);
    if (root->size == 2 * parameter - 1)
    {
        std::shared_ptr<Node> newRoot = nullptr;
        CreateNode(newRoot);
        rootIndex = newRoot->index;
        newRoot->leaf = false;
        newRoot->children[0] = root->index;
        std::shared_ptr<Node> secondChild = Split(newRoot, root, 0);
        Insert(newRoot, key);
    }
    else
        Insert(root, key);
    BackupTree();
}

template <typename TKey> void BTree<TKey>::Erase(TKey key)
{
    std::shared_ptr<Node> root;
    ReadNode(root, rootIndex);
    int keyIndex;
    keyIndex = std::upper_bound(root->keys, root->keys + root->size, key) - root->keys - 1;
    if (keyIndex != -1 && root->keys[keyIndex] == key)
    {
        if (root->leaf)
        {
            RemoveFromLeaf(root, keyIndex, true);
        }
        else
        {
            RemoveFromInternal(root, keyIndex, true);
        }
    }
    else if (!root->leaf)
    {
        std::shared_ptr<Node> child;
        ReadNode(child, root->children[keyIndex + 1]);
        if (child->size == parameter - 1)
        {
            EraseResolve(root, child, keyIndex + 1);
            if (root->size == 0)
            {
                rootIndex = root->children[0];
                WriteNode(root);
            }
        }
        Erase(child, key);
    }
    BackupTree();
}

template <typename TKey> void BTree<TKey>::Print() const
{
    std::shared_ptr<Node> root;
    ReadNode(root, rootIndex);
    Print(root, 0);
}

template <typename TKey> void BTree<TKey>::Print(std::shared_ptr<Node> node, int level) const
{
    std::cout << "level: " << level << " {";
    for (int i = 0; i < node->size; ++i)
    {
        std::cout << node->keys[i];
        if (i + 1 != node->size)
        {
            std::cout << ", ";
        }
    }
    std::cout << "}\n";
    if (node->leaf)
    {
        return;
    }
    for (int i = 0; i <= node->size; ++i)
    {
        std::shared_ptr<Node> child;
        ReadNode(child, node->children[i]);
        Print(child, level + 1);
    }
}

#endif
