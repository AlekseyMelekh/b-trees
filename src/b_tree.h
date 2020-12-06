#ifndef COURSE_PROJECT_B_TREE_H
#define COURSE_PROJECT_B_TREE_H

#include <iostream>
#include <memory>
#include <vector>

template <typename T> struct BNode
{
    bool isLeaf;
    size_t size;
    std::vector<T> keys;
    std::vector<std::unique_ptr<BNode<T>>> childrens;

    BNode() = delete;
    explicit BNode(size_t t) : isLeaf(true), size(0)
    {
        keys.resize(t * 2 - 1);
        childrens.resize(t * 2);
    }
    ~BNode() = default;
};

template <typename T> class BTree
{
  public:
    explicit BTree(size_t minDegree) : minDegree_(minDegree), root_(nullptr)
    {
    }
    ~BTree() = default;
    void Insert(const T &value)
    {
        if (root_ == nullptr)
        {
            root_.reset(new BNode<T>(minDegree_));
        }

        if (root_->size == minDegree_ * 2 - 1)
        {
            auto newRoot = std::make_unique<BNode<T>>(minDegree_);
            auto oldRoot = std::move(root_);
            root_ = std::move(newRoot);
            root_->childrens[0] = std::move(oldRoot);
            root_->isLeaf = false;
            Split(root_, 0);
            Insert(root_, value);
        }
        else
        {
            Insert(root_, value);
        }
    }
    void Print()
    {
        Print(root_, 0);
    }

  private:
    void Insert(std::unique_ptr<BNode<T>> &node, const T &value)
    {
        int64_t index = static_cast<uint64_t>(node->size) - 1;
        if (node->isLeaf)
        {
            while (index >= 0 && value < node->keys[index])
            {
                node->keys[index + 1] = node->keys[index];
                index--;
            }
            node->keys[index + 1] = value;
            node->size++;
        }
        else
        {
            while (index >= 0 && value < node->keys[index])
            {
                index--;
            }
            index++;
            if (node->childrens[index]->size == minDegree_ * 2 - 1)
            {
                Split(node, index);
                index += (value > node->keys[index]);
                node->isLeaf = false;
            }
            Insert(node->childrens[index], value);
        }
    }
    void Split(std::unique_ptr<BNode<T>> &node, size_t index)
    {
        auto rightNewNode = std::make_unique<BNode<T>>(minDegree_);
        auto &leftNewNode = node->childrens[index];
        rightNewNode->isLeaf = leftNewNode->isLeaf;
        rightNewNode->size = minDegree_ - 1;
        for (size_t i = 0; i < minDegree_ - 1; ++i)
        {
            rightNewNode->keys[i] = leftNewNode->keys[i + minDegree_];
        }
        if (!leftNewNode->isLeaf)
        {
            for (size_t i = 0; i < minDegree_; ++i)
            {
                rightNewNode->childrens[i] = std::move(leftNewNode->childrens[i + minDegree_]);
            }
        }
        leftNewNode->size = minDegree_ - 1;
        for (size_t i = node->size; i >= index + 1; --i)
        {
            node->childrens[i + 1] = std::move(node->childrens[i]);
        }
        node->childrens[index + 1] = std::move(rightNewNode);
        for (int64_t i = node->size; i >= index; --i)
        {
            node->keys[i + 1] = node->keys[i];
        }
        node->keys[index] = leftNewNode->keys[minDegree_ - 1];
        node->size++;
    }
    void Print(std::unique_ptr<BNode<T>> &node, uint64_t level)
    {
        std::cout << "level: " << level << "   {";
        for (size_t i = 0; i < node->size; ++i)
        {
            std::cout << node->keys[i];
            if (node->size != i + 1)
            {
                std::cout << ", ";
            }
        }
        std::cout << "}\n";
        if (!node->isLeaf)
        {
            for (size_t i = 0; i <= node->size; ++i)
            {
                Print(node->childrens[i], level + 1);
            }
        }
    }

  private:
    size_t minDegree_;
    std::unique_ptr<BNode<T>> root_;
};

#endif
