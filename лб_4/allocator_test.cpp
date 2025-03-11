#include <Windows.h>
#include <iostream>
#include <vector>
#include <chrono>
#include <random>

typedef struct Allocator Allocator;
typedef Allocator* (*allocator_create_func)(void*, size_t);
typedef void (*allocator_destroy_func)(Allocator*);
typedef void* (*allocator_alloc_func)(Allocator*, size_t);
typedef void (*allocator_free_func)(Allocator*, void*);

static Allocator* default_allocator_create(void* memory, size_t size) {
    return (Allocator*)1;
}

static void default_allocator_destroy(Allocator* alloc) {
}

static void* default_allocator_alloc(Allocator* alloc, size_t size) {
    return VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
}

static void default_allocator_free(Allocator* alloc, void* ptr) {
    VirtualFree(ptr, 0, MEM_RELEASE);
}

static allocator_create_func create_allocator = default_allocator_create;
static allocator_destroy_func destroy_allocator = default_allocator_destroy;
static allocator_alloc_func alloc_mem = default_allocator_alloc;
static allocator_free_func free_mem = default_allocator_free;

void test_allocator(Allocator* allocator, size_t total_size) {
    const int num_operations = 10000;
    std::vector<void*> blocks;
    blocks.reserve(num_operations);

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<size_t> size_dist(16, 256);
    std::bernoulli_distribution free_dist(0.3);

    auto start_alloc = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < num_operations; ++i) {
        size_t size = size_dist(rng);
        void* block = alloc_mem(allocator, size);
        if (block) {
            blocks.push_back(block);
            if (free_dist(rng) && !blocks.empty()) { 
                size_t idx = rng() % blocks.size();
                free_mem(allocator, blocks[idx]);
                blocks[idx] = blocks.back();
                blocks.pop_back();
            }
        }
    }
    auto end_alloc = std::chrono::high_resolution_clock::now();

    auto start_free = std::chrono::high_resolution_clock::now();
    for (void* block : blocks) {
        free_mem(allocator, block);
    }
    auto end_free = std::chrono::high_resolution_clock::now();

    auto alloc_time = std::chrono::duration_cast<std::chrono::microseconds>(end_alloc - start_alloc);
    auto free_time = std::chrono::duration_cast<std::chrono::microseconds>(end_free - start_free);

    std::cout << "Allocation time: " << alloc_time.count() << " μs\n";
    std::cout << "Free time: " << free_time.count() << " μs\n";
}

int main(int argc, char** argv) {
    HMODULE dll = nullptr;
    if (argc > 1) { 
        dll = LoadLibraryA(argv[1]);
        if (dll) {
            create_allocator = (allocator_create_func)GetProcAddress(dll, "allocator_create");
            destroy_allocator = (allocator_destroy_func)GetProcAddress(dll, "allocator_destroy");
            alloc_mem = (allocator_alloc_func)GetProcAddress(dll, "allocator_alloc");
            free_mem = (allocator_free_func)GetProcAddress(dll, "allocator_free");

            if (!create_allocator || !destroy_allocator || !alloc_mem || !free_mem) {
                std::cerr << "Failed to load functions from DLL\n";
                FreeLibrary(dll);
                dll = nullptr;
            }
        }
    }

    const size_t pool_size = 64 * 1024 * 1024;
    void* memory_pool = VirtualAlloc(nullptr, pool_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!memory_pool) {
        std::cerr << "Failed to allocate memory pool\n";
        return 1;
    }

    Allocator* allocator = create_allocator(memory_pool, pool_size);
    if (!allocator) {
        std::cerr << "Failed to create allocator\n";
        VirtualFree(memory_pool, 0, MEM_RELEASE);
        return 1;
    }

    std::cout << "Testing allocator...\n";
    test_allocator(allocator, pool_size);

    destroy_allocator(allocator);
    VirtualFree(memory_pool, 0, MEM_RELEASE);
    if (dll) FreeLibrary(dll);

    return 0;
}