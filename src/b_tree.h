#ifndef COURSE_PROJECT_B_TREE_H
#define COURSE_PROJECT_B_TREE_H

#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <queue>
#include <string>
#include <vector>

template <typename TKey, typename Hash = std::hash<TKey>> class BTree
{
  protected:
    bool binary;
    int parameter;
    int rootIndex;
    int lastIndex;
    std::string storageLocation;
    Hash hasher;
    struct Node
    {
        int size{};
        bool leaf{};
        int index{};
        int *keys = nullptr;
        int *children = nullptr;
        TKey *values = nullptr;
        ~Node()
        {
            delete[] keys;
            delete[] children;
            delete[] values;
        }
    };
    [[nodiscard]] std::string GetTempFileNameKeys(const std::string &storageLocation, int index) const;
    [[nodiscard]] std::string GetTempFileNameValues(const std::string &storageLocation, int index) const;
    void ReadNodeKeys(std::shared_ptr<Node> &node, int index) const;
    void ReadNodeValues(std::shared_ptr<Node> &node, int index) const;
    void WriteNode(std::shared_ptr<Node> node);
    void CreateNode(std::shared_ptr<Node> &node);
    void Insert(std::shared_ptr<Node> node, const TKey &value);
    void RemoveFromLeaf(std::shared_ptr<Node> leaf, int keyIndex, bool rootCall);
    void RemoveFromInternal(std::shared_ptr<Node> node, int keyIndex, bool rootCall);
    std::pair<int, TKey> RemoveMin(std::shared_ptr<Node> node);
    void Erase(std::shared_ptr<Node> node, const TKey &value);
    bool Find(std::shared_ptr<Node> node, const TKey &value) const;
    std::shared_ptr<Node> Split(std::shared_ptr<Node> parent, std::shared_ptr<Node> childPtr, int child);
    void Merge(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart, std::shared_ptr<Node> secondPart,
               int firstPartIndex, bool direction);
    void KeyRepositioning(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart,
                          std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction);
    bool EraseResolve(std::shared_ptr<Node> parent, std::shared_ptr<Node> child, int childIndex);
    void BackupTree() const;
    void Print(std::shared_ptr<Node> node, int level) const;

  public:
    BTree(int parameter, bool binary, const std::string &storageLocation);
    void Insert(TKey value);
    void Erase(TKey value);
    bool Find(TKey value) const;
    void Print() const;
    ~BTree();
};

template <typename TKey, typename Hash>
BTree<TKey, Hash>::BTree(int parameter, bool binary, const std::string &storageLocation)
    : lastIndex(0), rootIndex(0), binary(binary), parameter(parameter), hasher(Hash())
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
        this->storageLocation = storageLocation;
        std::shared_ptr<Node> root = nullptr;
        CreateNode(root);
        WriteNode(root);
        BackupTree();
    }
}

template <typename TKey, typename Hash> BTree<TKey, Hash>::~BTree()
{
    BackupTree();
}

template <typename TKey, typename Hash>
std::string BTree<TKey, Hash>::GetTempFileNameKeys(const std::string &storageLocation_, int index) const
{
    return (storageLocation_ + "/node-k" + std::to_string(index) + ".bin");
}

template <typename TKey, typename Hash>
std::string BTree<TKey, Hash>::GetTempFileNameValues(const std::string &storageLocation_, int index) const
{
    return (storageLocation_ + "/node-v" + std::to_string(index) + ".bin");
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::BackupTree() const
{
    FILE *output;
    fopen_s(&output, (storageLocation + "/treeParams.bin").c_str(), "wb");
    fwrite(&parameter, sizeof(int), 1, output);
    fwrite(&rootIndex, sizeof(int), 1, output);
    fwrite(&lastIndex, sizeof(int), 1, output);
    fwrite(&binary, sizeof(bool), 1, output);
    fclose(output);
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::ReadNodeKeys(std::shared_ptr<Node> &node, int index) const
{
    node = std::shared_ptr<Node>(new Node());
    if (binary)
    {
        FILE *fileIn;
        if (int x = fopen_s(&fileIn, GetTempFileNameKeys(storageLocation, index).c_str(), "rb"))
            return;
        node->keys = new int[2 * parameter - 1];
        node->index = index;
        fread(&node->size, sizeof(int), 1, fileIn);
        fread(&node->leaf, sizeof(bool), 1, fileIn);
        fread(node->keys, sizeof(int), node->size, fileIn);
        if (!node->leaf)
        {
            node->children = new int[2 * parameter];
            fread(node->children, sizeof(int), node->size + 1, fileIn);
        }
        fclose(fileIn);
    }
    else
    {
        std::ifstream fileIn(GetTempFileNameKeys(storageLocation, index));
        fileIn >> node->size;
        fileIn >> node->leaf;
        node->index = index;
        node->keys = new int[2 * parameter - 1];
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

template <typename TKey, typename Hash> void BTree<TKey, Hash>::ReadNodeValues(std::shared_ptr<Node> &node, int index) const
{
    if (binary)
    {
        FILE *fileIn;
        if (int x = fopen_s(&fileIn, GetTempFileNameValues(storageLocation, index).c_str(), "rb"))
            return;
        node->values = new TKey[2 * parameter - 1];
        fread(node->values, sizeof(TKey), node->size, fileIn);
        fclose(fileIn);
    }
    else
    {
        std::ifstream fileIn(GetTempFileNameValues(storageLocation, index));
        node->values = new TKey[2 * parameter - 1];
        for (int i = 0; i < node->size; i++)
            fileIn >> node->values[i];
        fileIn.close();
    }
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::WriteNode(std::shared_ptr<Node> node)
{
    if (node->size == 0 && node->index != rootIndex)
    {
        remove(GetTempFileNameKeys(storageLocation, node->index).c_str());
        remove(GetTempFileNameValues(storageLocation, node->index).c_str());
        return;
    }
    if (binary)
    {
        FILE *fileOut;
        fopen_s(&fileOut, GetTempFileNameKeys(storageLocation, node->index).c_str(), "wb");
        fwrite(&node->size, sizeof(int), 1, fileOut);
        fwrite(&node->leaf, sizeof(bool), 1, fileOut);
        fwrite(node->keys, sizeof(int), node->size, fileOut);
        if (!node->leaf)
            fwrite(node->children, sizeof(int), node->size + 1, fileOut);
        fclose(fileOut);
        fopen_s(&fileOut, GetTempFileNameValues(storageLocation, node->index).c_str(), "wb");
        fwrite(node->values, sizeof(TKey), node->size, fileOut);
        fclose(fileOut);
    }
    else
    {
        std::ofstream fileOut(GetTempFileNameKeys(storageLocation, node->index));
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
        fileOut.open(GetTempFileNameValues(storageLocation, node->index));
        for (int i = 0; i < node->size; i++)
            fileOut << node->values[i] << " ";
        fileOut.close();
    }
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::CreateNode(std::shared_ptr<Node> &node)
{
    node = std::shared_ptr<Node>(new Node());
    node->size = 0;
    node->index = lastIndex++;
    node->leaf = true;
    node->keys = new int[2 * parameter - 1];
    node->children = new int[2 * parameter];
    node->values = new TKey[2 * parameter - 1];
}

template <typename TKey, typename Hash>
std::shared_ptr<typename BTree<TKey, Hash>::Node> BTree<TKey, Hash>::Split(std::shared_ptr<Node> parent,
                                                                           std::shared_ptr<Node> childPtr, int child)
{
    std::shared_ptr<Node> firstPart, secondPart = nullptr;
    firstPart = childPtr;
    CreateNode(secondPart);
    secondPart->leaf = firstPart->leaf;
    for (int keyIndex = 0; keyIndex < parameter - 1; keyIndex++)
    {
        secondPart->keys[keyIndex] = firstPart->keys[keyIndex + parameter];
        secondPart->values[keyIndex] = firstPart->values[keyIndex + parameter];
    }
    if (!firstPart->leaf)
        for (int childIndex = 0; childIndex < parameter; childIndex++)
            secondPart->children[childIndex] = firstPart->children[childIndex + parameter];
    firstPart->size = parameter - 1;
    secondPart->size = parameter - 1;
    for (int keyIndex = parent->size; keyIndex > child; keyIndex--)
    {
        parent->keys[keyIndex] = parent->keys[keyIndex - 1];
        parent->values[keyIndex] = parent->keys[keyIndex - 1];
    }
    parent->keys[child] = firstPart->keys[parameter - 1];
    parent->values[child] = firstPart->values[parameter - 1];
    for (int childIndex = parent->size + 1; childIndex > child + 1; childIndex--)
        parent->children[childIndex] = parent->children[childIndex - 1];
    parent->children[child + 1] = secondPart->index;
    parent->size++;
    WriteNode(parent);
    WriteNode(firstPart);
    WriteNode(secondPart);
    return secondPart;
}

template <typename TKey, typename Hash>
void BTree<TKey, Hash>::Merge(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart,
                              std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction)
{
    if (direction)
    {
        firstPart->keys[firstPart->size] = parent->keys[firstPartIndex];
        firstPart->values[firstPart->size] = parent->values[firstPartIndex];
        for (int i = 0; i < secondPart->size; i++)
        {
            firstPart->keys[firstPart->size + i + 1] = secondPart->keys[i];
            firstPart->values[firstPart->size + i + 1] = secondPart->values[i];
            if (!firstPart->leaf)
                firstPart->children[firstPart->size + i + 1] = secondPart->children[i];
        }
        if (!firstPart->leaf)
            firstPart->children[firstPart->size + secondPart->size + 1] = secondPart->children[secondPart->size];
        for (int i = firstPartIndex; i < parent->size - 1; i++)
        {
            parent->keys[i] = parent->keys[i + 1];
            parent->values[i] = parent->values[i + 1];
            parent->children[i + 1] = parent->children[i + 2];
        }
    }
    else
    {
        for (int i = 0; i < firstPart->size; i++)
        {
            firstPart->keys[i + secondPart->size + 1] = firstPart->keys[i];
            firstPart->values[i + secondPart->size + 1] = firstPart->values[i];
            if (!firstPart->leaf)
                firstPart->children[i + secondPart->size + 1] = firstPart->children[i];
        }
        if (!firstPart->leaf)
            firstPart->children[firstPart->size + secondPart->size + 1] = firstPart->children[firstPart->size];
        for (int i = 0; i < secondPart->size; i++)
        {
            firstPart->keys[i] = secondPart->keys[i];
            firstPart->values[i] = secondPart->values[i];
            if (!firstPart->leaf)
                firstPart->children[i] = secondPart->children[i];
        }
        firstPart->keys[firstPart->size] = parent->keys[firstPartIndex - 1];
        firstPart->values[firstPart->size] = parent->values[firstPartIndex - 1];
        if (!firstPart->leaf)
            firstPart->children[firstPart->size] = secondPart->children[secondPart->size];
        for (int i = firstPartIndex - 1; i < parent->size - 1; i++)
        {
            parent->keys[i] = parent->keys[i + 1];
            parent->values[i] = parent->values[i + 1];
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

template <typename TKey, typename Hash>
void BTree<TKey, Hash>::KeyRepositioning(std::shared_ptr<Node> parent, std::shared_ptr<Node> firstPart,
                                         std::shared_ptr<Node> secondPart, int firstPartIndex, bool direction)
{
    if (direction)
    {
        firstPart->keys[firstPart->size] = parent->keys[firstPartIndex];
        firstPart->values[firstPart->size] = parent->values[firstPartIndex];
        if (!firstPart->leaf)
            firstPart->children[firstPart->size + 1] = secondPart->children[0];
        firstPart->size++;
        parent->keys[firstPartIndex] = secondPart->keys[0];
        parent->values[firstPartIndex] = secondPart->values[0];
        for (int i = 1; i < secondPart->size; i++)
        {
            secondPart->keys[i - 1] = secondPart->keys[i];
            secondPart->values[i - 1] = secondPart->values[i];
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
            firstPart->values[i] = firstPart->values[i - 1];
            if (!firstPart->leaf)
                firstPart->children[i + 1] = firstPart->children[i];
        }
        if (!firstPart->leaf)
        {
            firstPart->children[1] = firstPart->children[0];
            firstPart->children[0] = secondPart->children[secondPart->size];
        }
        firstPart->keys[0] = parent->keys[firstPartIndex - 1];
        firstPart->values[0] = parent->values[firstPartIndex - 1];
        firstPart->size++;
        parent->keys[firstPartIndex - 1] = secondPart->keys[secondPart->size - 1];
        parent->values[firstPartIndex - 1] = secondPart->values[secondPart->size - 1];
        secondPart->size--;
    }
    WriteNode(parent);
    WriteNode(firstPart);
    WriteNode(secondPart);
}

template <typename TKey, typename Hash>
bool BTree<TKey, Hash>::EraseResolve(std::shared_ptr<Node> parent, std::shared_ptr<Node> child, int childIndex)
{
    std::shared_ptr<Node> predecessor = nullptr, successor = nullptr;
    if (childIndex)
    {
        ReadNodeKeys(predecessor, parent->children[childIndex - 1]);
        ReadNodeValues(predecessor, parent->children[childIndex - 1]);
        if (predecessor->size != parameter - 1)
        {
            KeyRepositioning(parent, child, predecessor, childIndex, 0);
            return true;
        }
    }
    if (childIndex != parent->size)
    {
        ReadNodeKeys(successor, parent->children[childIndex + 1]);
        ReadNodeValues(successor, parent->children[childIndex + 1]);
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

template <typename TKey, typename Hash>
void BTree<TKey, Hash>::RemoveFromLeaf(std::shared_ptr<Node> leaf, int keyIndex, bool rootCall)
{
    for (int i = keyIndex; i < leaf->size - 1; i++)
    {
        leaf->keys[i] = leaf->keys[i + 1];
        leaf->values[i] = leaf->values[i + 1];
    }
    leaf->size--;

    WriteNode(leaf);
}

template <typename TKey, typename Hash>
void BTree<TKey, Hash>::RemoveFromInternal(std::shared_ptr<Node> node, int keyIndex, bool rootCall)
{
    int key = node->keys[keyIndex];
    std::shared_ptr<Node> child;
    ReadNodeKeys(child, node->children[keyIndex + 1]);
    ReadNodeValues(child, node->children[keyIndex + 1]);
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
    auto result = RemoveMin(child);
    node->keys[keyIndex] = result.first;
    node->values[keyIndex] = result.second;
    if (rootCall)
    {
        if (node->size == 0)
        {
            rootIndex = node->children[0];
        }
    }
    WriteNode(node);
}

template <typename TKey, typename Hash> std::pair<int, TKey> BTree<TKey, Hash>::RemoveMin(std::shared_ptr<Node> node)
{
    if (node->leaf)
    {
        int result = node->keys[0];
        TKey value = node->values[0];
        RemoveFromLeaf(node, 0, false);
        return {result, value};
    }
    else
    {
        std::shared_ptr<Node> child;
        ReadNodeKeys(child, node->children[0]);
        ReadNodeValues(child, node->children[0]);
        if (child->size == parameter - 1)
            EraseResolve(node, child, 0);
        return RemoveMin(child);
    }
}

template <typename TKey, typename Hash> bool BTree<TKey, Hash>::Find(std::shared_ptr<Node> node, const TKey &value) const
{
    int keyIndex = std::upper_bound(node->keys, node->keys + node->size, hasher(value)) - node->keys - 1;
    if (keyIndex != -1 && node->keys[keyIndex] == hasher(value))
        return true;
    if (node->leaf)
        return false;
    std::shared_ptr<Node> child;
    ReadNodeKeys(child, node->children[keyIndex + 1]);
    return Find(child, hasher(value));
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::Insert(std::shared_ptr<Node> node, const TKey &value)
{
    int keyIndex;
    if (node->leaf)
    {
        ReadNodeValues(node, node->index);
        for (keyIndex = node->size - 1; keyIndex >= 0 && hasher(value) < node->keys[keyIndex]; keyIndex--)
        {
            node->keys[keyIndex + 1] = node->keys[keyIndex];
            node->values[keyIndex + 1] = node->values[keyIndex];
        }
        node->keys[keyIndex + 1] = hasher(value);
        node->values[keyIndex + 1] = value;
        node->size++;
        WriteNode(node);
    }
    else
    {
        keyIndex = std::upper_bound(node->keys, node->keys + node->size, hasher(value)) - node->keys - 1;
        int childIndex = keyIndex + 1;
        std::shared_ptr<Node> child;
        ReadNodeKeys(child, node->children[childIndex]);
        if (child->size == 2 * parameter - 1)
        {
            ReadNodeValues(node, node->index);
            ReadNodeValues(child, node->children[childIndex]);
            std::shared_ptr<Node> secondChild = Split(node, child, childIndex);
            if (hasher(value) < node->keys[childIndex])
            {
                Insert(child, value);
            }
            else
            {
                Insert(secondChild, value);
            }
        }
        else
        {
            Insert(child, value);
        }
    }
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::Erase(std::shared_ptr<Node> node, const TKey &value)
{
    int keyIndex;
    keyIndex = std::upper_bound(node->keys, node->keys + node->size, hasher(value)) - node->keys - 1;
    if (keyIndex != -1 && node->keys[keyIndex] == hasher(value))
    {
        ReadNodeValues(node, node->index);
        if (node->leaf)
            RemoveFromLeaf(node, keyIndex, false);
        else
            RemoveFromInternal(node, keyIndex, false);
    }
    else if (!node->leaf)
    {
        std::shared_ptr<Node> child;
        ReadNodeKeys(child, node->children[keyIndex + 1]);
        ReadNodeValues(child, node->children[keyIndex + 1]);
        if (child->size == parameter - 1)
            EraseResolve(node, child, keyIndex + 1);
        Erase(child, value);
    }
}

template <typename TKey, typename Hash> bool BTree<TKey, Hash>::Find(TKey value) const
{
    std::shared_ptr<Node> root;
    ReadNodeKeys(root, rootIndex);
    return Find(root, value);
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::Insert(TKey value)
{
    std::shared_ptr<Node> root;
    ReadNodeKeys(root, rootIndex);
    if (root->size == 2 * parameter - 1)
    {
        ReadNodeValues(root, rootIndex);
        std::shared_ptr<Node> newRoot = nullptr;
        CreateNode(newRoot);
        rootIndex = newRoot->index;
        newRoot->leaf = false;
        newRoot->children[0] = root->index;
        std::shared_ptr<Node> secondChild = Split(newRoot, root, 0);
        Insert(newRoot, value);
    }
    else
        Insert(root, value);
    BackupTree();
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::Erase(TKey value)
{
    std::shared_ptr<Node> root;
    ReadNodeKeys(root, rootIndex);
    int keyIndex;
    keyIndex = std::upper_bound(root->keys, root->keys + root->size, hasher(value)) - root->keys - 1;
    if (keyIndex != -1 && root->keys[keyIndex] == hasher(value))
    {
        ReadNodeValues(root, rootIndex);
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
        ReadNodeKeys(child, root->children[keyIndex + 1]);
        ReadNodeValues(child, root->children[keyIndex + 1]);
        if (child->size == parameter - 1)
        {
            ReadNodeValues(root, rootIndex);
            EraseResolve(root, child, keyIndex + 1);
            if (root->size == 0)
            {
                rootIndex = root->children[0];
                WriteNode(root);
            }
        }
        Erase(child, value);
    }
    BackupTree();
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::Print() const
{
    std::shared_ptr<Node> root;
    ReadNodeKeys(root, rootIndex);
    Print(root, 0);
}

template <typename TKey, typename Hash> void BTree<TKey, Hash>::Print(std::shared_ptr<Node> node, int level) const
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
        ReadNodeKeys(child, node->children[i]);
        Print(child, level + 1);
    }
}

#endif
