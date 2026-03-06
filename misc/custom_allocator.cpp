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

// Alias for your code
template<typename T>
using AlignedVector = std::vector<T, AlignedAllocator<T>>;

// ==========================================
// 2. The Physics Kernel
// ==========================================
void solve_stencil(const double* __restrict__ in, double* __restrict__ out, size_t n) {
    for (size_t i = 1; i < n - 1; ++i) {
        out[i] = 0.5 * (in[i - 1] + in[i + 1]);
    }
}

// ==========================================
// 3. The Test Runner
// ==========================================
template <typename VectorType>
void run_test(const std::string& name, size_t N, size_t iterations) {
    VectorType in(N, 1.0);
    VectorType out(N, 0.0);

    // Warmup
    solve_stencil(in.data(), out.data(), N);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t k = 0; k < iterations; ++k) {
        solve_stencil(in.data(), out.data(), N);
        if (N > 1) in[1] = out[N/2]; // Dummy write
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    size_t address = reinterpret_cast<size_t>(in.data());
    size_t alignment_mod = address % 64;

    std::cout << "------------------------------------------------\n";
    std::cout << "Test: " << name << "\n";
    std::cout << "Start Address Mod 64: " << alignment_mod << " (0 means 64-byte aligned)\n";
    std::cout << "Time: " << elapsed.count() << "s\n";
    std::cout << "Bandwidth: " << (double(N * 8 * 2 * iterations) / 1e9) / elapsed.count() << " GB/s\n";
}

class MyClass {
    public:
    int aaa;
        MyClass(int aha){
            aaa = aha;
        }
};

int main() {
    const size_t N = 100'000'000; 
    const size_t ITERATIONS = 1;

    std::cout << "Benchmarking Stencil Operation (Size: " << N << " doubles)\n";

    MyClass();

    //run_test<AlignedVector<double>>("Super Aligned Vector", N, ITERATIONS);
    //run_test<std::vector<double>>("Standard std::vector", N, ITERATIONS);

    return 0;
}