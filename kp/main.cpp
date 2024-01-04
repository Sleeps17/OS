#include <iostream>
#include <vector>
#include <chrono>
#include "freeListsAllocator.hpp"
#include "binaryAllocator.hpp"
#include "TestBin.hpp"

using Byte = char;

int main() {

    std::vector<size_t> sizes = {1, 2, 4, 8, 16, 32, 64, 128, 256};
    const size_t attemps = 1000;

    BinaryAllocator::Allocator a(1024*1024);
    FreeListsAllocator::Allocator b(1024*1024);

    auto start = std::chrono::steady_clock::now();
    for(auto& size : sizes) {
        for(int i = 0; i < attemps; ++i) {
            auto ptr = a.allocate(size);
        }
    }
    auto stop = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Binary Allocator:\n";
    std::cout << duration.count() << "mcs" << std::endl;
    std::cout << "Peak Allocated: " << a.peakAlloced << std::endl;

    start = std::chrono::steady_clock::now();
    for(auto& size : sizes) {
        for(int i = 0; i < attemps; ++i) {
            auto ptr = b.allocate(size);
        }
    }
    stop = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Free Lists Allocator:\n";
    std::cout << duration.count() << "mcs" << std::endl;
    std::cout << "Peak Allocated: " << b.peakAlloced << std::endl;

    auto alloc = bin_alloc_create(1024*1024);
    start = std::chrono::steady_clock::now();
    for(auto& size : sizes) {
        for(int i = 0; i < attemps; ++i) {
            auto ptr = bin_alloc_allocate(alloc, size);
        }
    }
    stop = std::chrono::steady_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - start);
    std::cout << "Bin Allocator:\n";
    std::cout << duration.count() << "mcs" << std::endl;
}