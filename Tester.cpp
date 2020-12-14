#include <time.h>
#include "../ExternalMemoryAlgorithms/BTreeMap.h"
#include "../ExternalMemoryAlgorithms/BTreeSet.h"
#include "../ExternalMemoryAlgorithms/Karatsuba.h"
#include "../ExternalMemoryAlgorithms/Conversion.h"
#include "../ExternalMemoryAlgorithms/ExternalSort.h"
#include <iostream>
#include <memory>

const std::string BTREE_DIRECTORY = "C:/External Memory/MyTest/";
const std::string SORT_DIRECTORY = "C:/External Memory/MyTestSort/";
const std::string KARATSUBA_DIRECTORY = "C:/External Memory/MyTestKaratsuba/";
const int PARAMETER = 160;
const int N = 1e7;
const int ROOT_PARAMETER = 37;
inline int RAND(int n) {
	return abs((rand() << 15) + rand()) % n;
}
inline int generateTree(int l, int r, int& num)
{
	int result = num;
	FILE* output;
	fopen_s(&output, (BTREE_DIRECTORY + "node" + std::to_string(num) + ".bin").c_str(), "wb");
	num++;
	if (r - l + 1 >= PARAMETER - 1 && r - l + 1 <= 2 * PARAMETER - 1)
	{
		int size = r - l + 1;
		bool leaf = true;
		fwrite(&size, sizeof(int), 1, output);
		fwrite(&leaf, sizeof(bool), 1, output);
		for (int i = l; i <= r; i++)
			fwrite(&i, sizeof(int), 1, output);
		for (int i = l + 5; i <= r + 5; i++)
			fwrite(&i, sizeof(int), 1, output);
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
	for (int i = l + childSize + (cnt < additionalChilds) + 5; i < r + 5; i += childSize + 1 + (cnt < additionalChilds))
	{
		fwrite(&i, sizeof(int), 1, output);
		cnt++;
	}
	cnt = 0;
	for (int i = l + childSize + (cnt < additionalChilds); i < r; i += childSize + 1 + (cnt < additionalChilds))
	{
		int subresult = generateTree(i - childSize - (cnt < additionalChilds), i - 1, num);
		fwrite(&subresult, sizeof(int), 1, output);
		cnt++;	
	}
	int subresult = generateTree(r - childSize + 1, r, num);
	fwrite(&subresult, sizeof(int), 1, output);
	fclose(output);
	return result;
}
void generateFile(std::string filePath, bool writeLength)
{
	FILE* file;
	fopen_s(&file, filePath.c_str(), "wb");
	BufferedIntWriter bw(file, 1024);
	if (writeLength)
		bw.writeNext(N);
	for (int i = 0; i < N; i++)
	{
		bw.writeNext(RAND(10));
	}
}
bool checkFile(std::string filePath)
{
	FILE* file;
	fopen_s(&file, filePath.c_str(), "rb");
	BufferedIntReader br(file, 1024);
	int last, cur;
	last = -1;
	for (int i = 0; i < N; i++)
	{
		cur = br.next();
		if (cur < last)
		{
			std::cout << cur << " " << last << "\n";
			return false;
		}
		else
			last = cur;
	}
	fclose(file);
	return true;
}
void testKaratsuba()
{
	clock_t tStart = clock();
	multiply(KARATSUBA_DIRECTORY + "input.bin", KARATSUBA_DIRECTORY + "input2.bin", KARATSUBA_DIRECTORY + "output.bin", KARATSUBA_DIRECTORY, 1e5);
	printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
}
void testBTree()
{
	clock_t tStart = clock();
	BTreeMap<int, int> bm(PARAMETER, 1, BTREE_DIRECTORY);
	for (int i = 0; i < 1000; i++)
	{
		bm.find(RAND(1e8));
	}
	printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
}
void testSort()
{
	clock_t tStart = clock();
	externalSort(SORT_DIRECTORY + "input.bin", SORT_DIRECTORY + "output.bin", SORT_DIRECTORY, 1e8);
	printf("Time taken: %.2fs\n", (double)(clock() - tStart) / CLOCKS_PER_SEC);
}
int main()
{
	std::ios_base::sync_with_stdio(0);
	srand(time(NULL));
	FILE* out;
	testBTree();
	//testSort();
	//testKaratsuba();
}