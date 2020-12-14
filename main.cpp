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

const int kTableRow = 50622;
const int kTableCols = 31622;
const int kParam = 160;
const int N = 31622 * 31622;
const int ROOT_PARAMETER = 37;
const std::string storageLocation = "../../storage-b-tree-gen";

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

void TestBTree(const std::string &);

inline int GenerateTree(int l, int r, int& num)
{
    int result = num;
    FILE* output;
    fopen_s(&output, (storageLocation + "/node-k" + std::to_string(num) + ".bin").c_str(), "wb");
    num++;
    if (r - l + 1 >= kParam - 1 && r - l + 1 <= 2 * kParam - 1)
    {
        int size = r - l + 1;
        bool leaf = true;
        fwrite(&size, sizeof(int), 1, output);
        fwrite(&leaf, sizeof(bool), 1, output);
        for (int i = l; i <= r; i++)
            fwrite(&i, sizeof(int), 1, output);
        fclose(output);
        fopen_s(&output, (storageLocation + "/node-v" + std::to_string(result) + ".bin").c_str(), "wb");
        for (int i = l; i <= r; i++)
        {
            TableItem item(GetRand(1, 100000), 1, 1);
            fwrite(&item, sizeof(TableItem), 1, output);
        }
        fclose(output);
        return result;
    }
    int parameter;
    if (l == 0 && r == N - 1)
        parameter = ROOT_PARAMETER;
    else
        parameter = 300;
    int childSize = (r - l + 1 - (parameter - 1)) / parameter;
    int additionalChilds = (r - l + 1 - (parameter - 1)) % parameter;
    int cnt = 0;
    int size = parameter - 1;
    bool leaf = false;
    fwrite(&size, sizeof(int), 1, output);
    fwrite(&leaf, sizeof(bool), 1, output);
    for (int i = l + childSize + (cnt < additionalChilds); i < r; i += childSize + 1 + (cnt < additionalChilds))
    {
        fwrite(&i, sizeof(int), 1, output);
        cnt++;
    }
    cnt = 0;
    for (int i = l + childSize + (cnt < additionalChilds); i < r; i += childSize + 1 + (cnt < additionalChilds))
    {
        int subresult = GenerateTree(i - childSize - (cnt < additionalChilds), i - 1, num);
        fwrite(&subresult, sizeof(int), 1, output);
        cnt++;
    }
    int subresult = GenerateTree(r - childSize + 1, r, num);
    fwrite(&subresult, sizeof(int), 1, output);
    fclose(output);
    fopen_s(&output, (storageLocation + "/node-v" + std::to_string(result) + ".bin").c_str(), "wb");
    cnt = 0;
    for (int i = l + childSize + (cnt < additionalChilds); i < r; i += childSize + 1 + (cnt < additionalChilds))
    {
        TableItem item(GetRand(1, 100000), 1, 1);
        fwrite(&item, sizeof(TableItem), 1, output);
        cnt++;
    }
    fclose(output);
    return result;
}

void GenerateBTree()
{
    FILE *output;
    int rootIndex = 0;
    int lastIndex = 0;
    bool binary = true;
    fopen_s(&output, (storageLocation + "/treeParams.bin").c_str(), "wb");
    fwrite(&kParam, sizeof(int), 1, output);
    fwrite(&rootIndex, sizeof(int), 1, output);
    fwrite(&lastIndex, sizeof(int), 1, output);
    fwrite(&binary, sizeof(bool), 1, output);
    fclose(output);
    GenerateTree(0, N - 1, lastIndex);
}

int main()
{
    TestBTree(storageLocation);
//    GenerateBTree();
    return 0;
}

void TestBTree(const std::string &storageLocation)
{
    BTree<TableItem> bTree(kParam, true, storageLocation);
    long double prevTime = 0;
    std::vector<long double> insertTime, eraseTime, getTime;
    long double sum = 0;
    for (int i = 0; i < 1000; ++i)
    {
        bTree.Get(GetRand(1, 31621), GetRand(1, 31621));
        getTime.emplace_back(clock() / (long double)CLOCKS_PER_SEC - prevTime);
        prevTime = clock() / (long double)CLOCKS_PER_SEC;
    }
    sum = 0;
    for (const auto &i : getTime)
    {
        sum += i;
    }
    std::cout << "1000 get: " << sum << " sec.\n";
    std::cout << "avg get: " << sum / getTime.size() << " sec.\n\n";
    for (int i = 0; i < 1000; ++i)
    {
        bTree.Erase(TableItem(4, GetRand(1, 31621), GetRand(1, 31621)));
        eraseTime.emplace_back(clock() / (long double)CLOCKS_PER_SEC - prevTime);
        prevTime = clock() / (long double)CLOCKS_PER_SEC;
    }
    sum = 0;
    for (const auto &i : eraseTime)
    {
        sum += i;
    }
    std::cout << "1000 erase: " << sum << " sec.\n";
    std::cout << "avg erase: " << sum / eraseTime.size() << " sec.\n";

//    int cntInsert = 0;
//    long double prev = 0;
//    for (int i = 0; i < kTableRow; ++i)
//    {
//        for (int j = 0; j < kTableCols; ++j)
//        {
//            bTree.Insert(TableItem(GetRand(1, 100), i, j));
//            cntInsert++;
//        }
//        std::cout << cntInsert << "\n";
//        std::cout << clock() / (long double)CLOCKS_PER_SEC - prev << " sec.\n";
//        prev = clock() / (long double)CLOCKS_PER_SEC;
//    }
//    std::cerr << clock() / (long double)CLOCKS_PER_SEC << " sec.\n";
//    std::cout << cntInsert << " items inserted\n";
//    int cntErase = 0;
//    for (int i = 0; i < kTableRow; ++i)
//    {
//        for (int j = 0; j < kTableCols; ++j)
//        {
//            TableItem temp = TableItem(GetRand(1, 100), i, j);
//            if (GetRand(1, 5) == 1 && bTree.Find(temp))
//            {
//                bTree.Erase(temp);
//                cntErase++;
//            }
//        }
//    }
//    std::cout << cntErase << " items erased\n";
//    int cntFind = 0;
//    for (int i = 0; i < kTableRow; ++i)
//    {
//        for (int j = 0; j < kTableCols; ++j)
//        {
//            cntFind += bTree.Find(TableItem(GetRand(1, 100), i, j));
//        }
//    }
//    std::cout << cntFind << " items found\n";
//    int cntGet = 0;
//    for (int i = 0; i < kTableRow; ++i)
//    {
//        for (int j = 0; j < kTableCols; ++j)
//        {
//            auto value = bTree.Get(i, j);
//            cntGet += value.has_value();
//        }
//    }
//    std::cout << cntGet << " items got\n";
    std::cerr << clock() / (long double)CLOCKS_PER_SEC << " sec.\n";
}