#include "src/b_plus_tree.h"
#include "src/b_tree.h"
#include <ctime>
#include <random>

struct TableItem
{
    int value;
    int i, j;

    TableItem() = default;
    ~TableItem() = default;
    TableItem(int value, int i, int j) : value(value), i(i), j(j){};
    TableItem &operator=(const TableItem &other)
    {
        if (this != &other)
        {
            value = other.value;
            i = other.i;
            j = other.j;
        }
        return *this;
    }
    friend bool operator==(const TableItem &lhs, const TableItem &rhs)
    {
        return lhs.value == rhs.value && lhs.i == rhs.i && lhs.j == rhs.j;
    }
    friend bool operator!=(const TableItem &lhs, const TableItem &rhs)
    {
        return !(lhs == rhs);
    }
    friend std::ostream &operator<<(std::ostream &out, const TableItem &item)
    {
        out << item.value << ' ' << item.i << ' ' << item.j;
        return out;
    }
    friend std::istream &operator>>(std::istream &in, TableItem &item)
    {
        in >> item.value >> item.i >> item.j;
        return in;
    }
};

const int kTableRow = 100;
const int kTableCols = 100;
const int kParam = 100;

namespace std
{
template <> struct hash<TableItem>
{
    size_t operator()(const TableItem &k) const
    {
        return kTableCols * k.i + k.j;
    }
};
} // namespace std

int GetRand(int l, int r)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(l, r);

    return distrib(gen);
}

void TestBTree()
{
    BTree<TableItem> bTree(kParam, true, "../storage-b-tree");
    int cntInsert = 0;
    for (int i = 0; i < kTableRow; ++i)
    {
        for (int j = 0; j < kTableCols; ++j)
        {
            if (GetRand(1, 5) == 1)
            {
                bTree.Insert(TableItem(GetRand(1, 100), i, j));
                cntInsert++;
            }
        }
    }
    std::cout << cntInsert << " items inserted\n";
    int cntErase = 0;
    for (int i = 0; i < kTableRow; ++i)
    {
        for (int j = 0; j < kTableCols; ++j)
        {
            TableItem temp = TableItem(GetRand(1, 100), i, j);
            if (GetRand(1, 5) == 1 && bTree.Find(temp))
            {
                bTree.Erase(temp);
                cntErase++;
            }
        }
    }
    std::cout << cntErase << " items erased\n";
    int cntFind = 0;
    for (int i = 0; i < kTableRow; ++i)
    {
        for (int j = 0; j < kTableCols; ++j)
        {
            cntFind += bTree.Find(TableItem(GetRand(1, 100), i, j));
        }
    }
    std::cout << cntFind << " items found\n";
    int cntGet = 0;
    for (int i = 0; i < kTableRow; ++i)
    {
        for (int j = 0; j < kTableCols; ++j)
        {
            auto value = bTree.Get(i, j);
            cntGet += value.has_value();
        }
    }
    std::cout << cntGet << " items got\n";
    std::cerr << clock() / (long double)CLOCKS_PER_SEC << " sec.\n";
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
