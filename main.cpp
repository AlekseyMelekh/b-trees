#include "src/b_tree.h"

int main()
{
    BTree<int> bTree(2, true, "../storage");

    std::vector<int> input = {11, 12, 13, 14, 1, 2, 3, 4, 5, 15, 16, 6, 7, 8, 9, 10};

    for (const auto &i : input)
    {
        bTree.Insert(i);
    }
    bTree.Print();
    std::cout << bTree.Find(11) << '\n';
    bTree.Erase(11);
    std::cout << bTree.Find(5) << '\n';
    std::cout << bTree.Find(11) << '\n';

    return 0;
}
