// FileSort.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#define _CRT_SECURE_NO_WARNINGS 1

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <cassert>
#include <algorithm>
#include <string>
#include <set>

const size_t MEMORY_LIMIT = 64*1054; // 64 Kbytes

int file_sort(const char* infile, const char* outfile)
{
    if (!infile || !outfile) return - 1;
    FILE* in = fopen(infile, "r");
    if (!in)return -2;
    
    char* tmp_in;
    int tmp_idx = 0;
    int infile_length = strlen(infile);
    tmp_in = (char*)malloc(infile_length + 30);
    if (!tmp_in) return -3;
    memset(tmp_in, 0, infile_length + 30);

    strncpy(tmp_in, infile, infile_length);
    tmp_in[infile_length++] = '_';
    tmp_in[infile_length++] = 't';
    tmp_in[infile_length++] = 'm';
    tmp_in[infile_length++] = 'p';
    tmp_in[infile_length++] = '_';
    tmp_in[infile_length] = '\0';

    
    std::vector<int> nums;
    nums.reserve(MEMORY_LIMIT / sizeof(int));
    int x;
    while (1 == fscanf(in, "%d", &x))
    {
        nums.push_back(x);
        if (nums.size() >= MEMORY_LIMIT / sizeof(int))
        {
            std::sort(nums.begin(), nums.end());
            ++tmp_idx;
            int ix, idx;
            for (ix = infile_length, idx = tmp_idx; idx != 0; idx /= 10, ++ix)
            {
                tmp_in[ix] = idx % 10 + '0';
            }
            tmp_in[ix] = '\0';
            FILE* in_x = fopen(tmp_in, "w");
            for (int x : nums) fprintf(in_x, "%d ", x);
            fclose(in_x);
            nums.clear();
        }
    }
    if (nums.size() > 0)
    {
        std::sort(nums.begin(), nums.end());
        ++tmp_idx;
        int ix, idx;
        for (ix = infile_length, idx = tmp_idx; idx != 0; idx /= 10, ++ix)
        {
            tmp_in[ix] = idx % 10 + '0';
        }
        tmp_in[ix] = '\0';
        FILE* in_x = fopen(tmp_in, "w");
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
        //tmp_in = std::format("{}_tmp_{}", infile, i);
        int ix, idx;
        for (ix = infile_length, idx = i; idx != 0; idx /= 10, ++ix)
        {
            tmp_in[ix] = idx % 10 + '0';
        }
        tmp_in[ix] = '\0';

        tag g;
        g.f = fopen(tmp_in, "r");
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
        int ix, idx;
        for (ix = infile_length, idx = i; idx != 0; idx /= 10, ++ix)
        {
            tmp_in[ix] = idx % 10 + '0';
        }
        tmp_in[ix] = '\0';

        ::remove(tmp_in);
    }
    fclose(out);
    free(tmp_in);
    return 0;
}

int main(int argc, char* argv[])
{
#if 0
    if (argc != 3) {
        puts("ERROR: <program> <in-file> <out-file>");
        return -1;
    }

    return file_sort(argv[1], argv[2]);
#endif
      file_sort("input.txt", "output.txt");
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
