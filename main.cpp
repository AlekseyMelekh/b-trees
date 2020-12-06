#include <iostream>

#include "src/b_tree.h"

int main()
{
    BTree<int> bTree(3);
    std::vector<int> input = {10, 20, 35, 40, 65, 2, 4, 5, 34, 3, 4, 50, 70, 80, 90, 1235, 325, 34, 2334, 324234, 10, 10};
    for (const auto &i : input)
    {
        bTree.Insert(i);
    }
    std::vector<int> another_input = {120, 2134, 43, 3535};
    for (const auto &i : another_input)
    {
        bTree.Insert(i);
    }
    bTree.Print();
    return 0;
}
