// FileSort.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS 1

#include <cstdio>
#include <vector>
#include <cassert>
#include <algorithm>
#include <string>
#include <format>
#include <set>

const size_t MEMORY_LIMIT = 64; // 64 Kbytes

int file_sort(const char* infile, const char* outfile)
{
    if (!infile || !outfile) return - 1;
    FILE* in = fopen(infile, "r");
    if (!in)return -2;
    
    std::string tmp_in;
    int tmp_idx = 0;

    std::vector<int> nums;
    nums.reserve(MEMORY_LIMIT / sizeof(int));
    int x;
    while (1 == fscanf(in, "%d", &x))
    {
        nums.push_back(x);
        if (nums.size() >= MEMORY_LIMIT / sizeof(int))
        {
            std::sort(nums.begin(), nums.end());
            tmp_in = std::format("{}_tmp_{}", infile, ++tmp_idx);
            FILE* in_x = fopen(tmp_in.c_str(), "w");
            for (int x : nums) fprintf(in_x, "%d ", x);
            fclose(in_x);
            nums.clear();
        }
    }
    if (nums.size() > 0)
    {
        std::sort(nums.begin(), nums.end());
        tmp_in = std::format("{}_tmp_{}", infile, ++tmp_idx);
        FILE* in_x = fopen(tmp_in.c_str(), "w");
        for (int x : nums) fprintf(in_x, "%d ", x);
        fclose(in_x);
        nums.clear();
    }

    std::vector<int>{}.swap(nums);
    struct tag
    {
        FILE* f;
        int x;
        bool operator < (const tag& t) const { return x < t.x; }
    };

    std::multiset<tag> st;
    for (int i = 1; i <= tmp_idx; ++i)
    {
        tmp_in = std::format("{}_tmp_{}", infile, i);

        tag g;
        g.f = fopen(tmp_in.c_str(), "r");
        if (1 == fscanf(g.f, "%d", &g.x))
        {
            st.insert(g);
        }
        else {
            assert(false);
        }
    }

    FILE* out = fopen(outfile, "w");
    while (!st.empty())
    {
        auto beg = st.begin();
        tag g = *beg;
        st.erase(beg);
        fprintf(out, "%d ", g.x);
        if (1 == fscanf(g.f, "%d", &g.x))
        {
            st.insert(g);
        }
        else {
            fclose(g.f);
        }
    }

    for (int i = 1; i <= tmp_idx; ++i)
    {
        tmp_in = std::format("{}_tmp_{}", infile, i);
        ::remove(tmp_in.c_str());
    }
    fclose(out);

    return 0;
}

int main(int argc, char* argv[])
{

    if (argc != 3) {
        puts("ERROR: <program> <in-file> <out-file>");
        return -1;
    }

    return file_sort(argv[1], argv[2]);
  //  file_sort("input.txt", "output.txt");
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
