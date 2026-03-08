#ifndef CUSTOM_ALLOCATOR
#define CUSTOM_ALLOCATOR 

#include <iostream>
#include <vector>
#include <chrono>
#include <new>
#include <mm_malloc.h>

// ==========================================
// 1. The Bulletproof Aligned Allocator
// ==========================================
template <typename T>
struct AlignedAllocator {
    using value_type = T;

    // 1. Boilerplate that MinGW/older GCC needs
    AlignedAllocator() noexcept {}
    template <class U> AlignedAllocator(const AlignedAllocator<U>&) noexcept {}

    // 2. Explicit Rebind (Helps compiler figure out types)
    template <class U> struct rebind { using other = AlignedAllocator<U>; };

    // 3. The Allocation Logic (Using Intel Intrinsics)
    T* allocate(size_t n) {
        size_t bytes = n * sizeof(T);
        // _mm_malloc(size, alignment) is standard for SIMD code
        void* ptr = _mm_malloc(bytes, 64); 
        if (!ptr) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }

    void deallocate(T* p, size_t) {
        _mm_free(p);
    }
};

// 4. Equality Operators (Required by some std::vector implementations)
template <class T, class U>
bool operator==(const AlignedAllocator<T>&, const AlignedAllocator<U>&) { return true; }

template <class T, class U>
bool operator!=(const AlignedAllocator<T>&, const AlignedAllocator<U>&) { return false; }

#endif 
