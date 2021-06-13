#include <algorithm>
#include <chrono>
#include <iostream>
#include <iterator>
#include <vector>

#include "Enumerable.h"

using cpplinq::Enumerable;

void TestBodyRvalue() {
    auto query = Enumerable<int>::Repeat(1, 10000)
        .SelectWithIndex([] (int x, int i) { return x * i; })
        .Select([] (int x) { return x * x; });
    query.begin();
}

void TestBodyLvalue() {
    auto query1 = Enumerable<int>::Repeat(1, 10000);
    auto query2 = query1.SelectWithIndex([] (int x, int i) { return x * i; });
    auto query3 = query2.Select([] (int x) { return x * x; });
    query3.begin();
}

void TestBodyStl1() {
    std::vector<int> query1(10000, 1);

    std::vector<int> query2(std::size(query1));
    {
        int i = 0;
        for (auto itr = std::begin(query1); itr != std::end(query1); ++itr, ++i) {
            auto x = *itr;
            query2[i] = x * i;
        }
    }

    std::vector<int> query3(std::size(query2));
    std::transform(std::begin(query2), std::end(query2), std::begin(query3), [] (int x) { return x * x; });
}

void TestBodyStl2() {
    std::vector<int> query1(10000, 1);

    std::vector<int> query2;
    {
        int i = 0;
        for (auto itr = std::begin(query1); itr != std::end(query1); ++itr, ++i) {
            auto x = *itr;
            query2.push_back(x * i);
        }
    }

    std::vector<int> query3;
    std::transform(std::begin(query2), std::end(query2), std::back_inserter(query3), [] (int x) { return x * x; });
}

void AnalyzePerformanceImpl(void(*testBody)()) {
    constexpr int N = 10000;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        testBody();
    }
    auto stop = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count() << " ms" << std::endl;
}

void AnalyzePerformance() {
    std::cout << "rvalue... ";
    AnalyzePerformanceImpl(&TestBodyRvalue);

    std::cout << "lvalue... ";
    AnalyzePerformanceImpl(&TestBodyLvalue);

    std::cout << "stl 1... ";
    AnalyzePerformanceImpl(&TestBodyStl1);

    std::cout << "stl 2... ";
    AnalyzePerformanceImpl(&TestBodyStl2);
}
