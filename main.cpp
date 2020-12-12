#include "src/b_plus_tree.h"
#include "src/b_tree.h"
#include <ctime>

void TestBTree()
{
    BTree<int> bTree(1000, true, "../storage-b-tree");
    long double prev = 0;
    for (int i = 0; i < 100'000; ++i)
    {
        bTree.Insert(i);
        if (i % 1'000 == 0)
        {
            std::cout << i << '\n';
            long double cur = (long double)(std::clock()) / CLOCKS_PER_SEC;
            std::cerr << cur - prev << "sec\n";
            prev = cur;
        }
    }
    bTree.Print();
}

void TestBPlusTree()
{
    BPlusTree<int> bPlusTree(2, true, "../storage-b-plus-tree");
}

int main()
{
    TestBTree();
//    TestBPlusTree();

    return 0;
}
