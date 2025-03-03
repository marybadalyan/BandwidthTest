#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <immintrin.h> // AVX2
#include <memory>
#include "kaizen.h"

std::atomic<bool> running(true);
const size_t BUFFER_SIZE = 1ULL * 1024ULL * 1024ULL * 1024ULL; // 1 GB
const size_t ITERATIONS = 20;

// Custom allocator for 32-byte alignment
// Custom allocator for 32-byte alignment
template<typename T>
struct AlignedAllocator {
    using value_type = T;

    AlignedAllocator() = default;

    template <typename U>
    AlignedAllocator(const AlignedAllocator<U>&) {}

    T* allocate(std::size_t n) {
        void* ptr = _mm_malloc(n * sizeof(T), 32); // 32-byte aligned malloc
        if (!ptr) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t) {
        _mm_free(ptr);
    }

    template <typename U>
    struct rebind {
        using other = AlignedAllocator<U>;
    };
};


std::vector<uint64_t, AlignedAllocator<uint64_t>> buffer(BUFFER_SIZE / sizeof(uint64_t), 0);

void memory_stress_test(size_t thread_id, size_t thread_count, uint64_t& bytes_processed) {
    size_t chunk_size = buffer.size() / thread_count;
    size_t start = thread_id * chunk_size;
    size_t end = (thread_id == thread_count - 1) ? buffer.size() : start + chunk_size;

    // Ensure end aligns with AVX2 step size
    end = start + ((end - start) / 16) * 16; // Round down to multiple of 16

    bytes_processed = 0;
    __m256i pattern = _mm256_set1_epi64x(0xDEADBEEF);

    for (size_t iter = 0; iter < ITERATIONS && running; ++iter) {
        for (size_t i = start; i < end; i += 16) { // Adjusted bounds
            __m256i data1 = _mm256_load_si256(reinterpret_cast<__m256i*>(&buffer[i]));
            __m256i data2 = _mm256_load_si256(reinterpret_cast<__m256i*>(&buffer[i + 4]));
            __m256i data3 = _mm256_load_si256(reinterpret_cast<__m256i*>(&buffer[i + 8]));
            __m256i data4 = _mm256_load_si256(reinterpret_cast<__m256i*>(&buffer[i + 12]));
            _mm256_stream_si256(reinterpret_cast<__m256i*>(&buffer[i]), pattern);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(&buffer[i + 4]), data1);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(&buffer[i + 8]), data2);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(&buffer[i + 12]), data3);
            bytes_processed += sizeof(uint64_t) * 32;
        }
    }
}

int main(int argc, char* argv[]) {
    // Parse command-line arguments with Kaizen
    zen::cmd_args args(argv, argc);
    size_t thread_count = args.accept("--threads").count_accepted();
    if (thread_count < 1) {
        std::cerr << "Error: Thread count must be at least 1. Using 1 instead.\n";
        thread_count = 1;
    }

    std::fill(buffer.begin(), buffer.end(), 1);

    std::vector<std::thread> threads;
    std::vector<uint64_t> bytes_per_thread(thread_count, 0);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < thread_count; ++i) {
        threads.emplace_back(memory_stress_test, i, thread_count, std::ref(bytes_per_thread[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    uint64_t total_bytes = std::accumulate(bytes_per_thread.begin(), bytes_per_thread.end(), 0ULL);
    double total_gb = total_bytes / (1024.0 * 1024.0 * 1024.0);
    double throughput = total_gb / elapsed.count();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Performance Metrics  |    Value        |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Buffer Size:         | " << std::setw(8) << std::fixed << std::setprecision(2) << BUFFER_SIZE / (1024.0 * 1024.0 * 1024.0)  << " GB     |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Thread Count:        | " << std::setw(8) << thread_count << "        |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Bytes processed:     | " << std::setw(8) << std::fixed << std::setprecision(2) << total_gb    << " GB     |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Elapsed Time:        | " << std::setw(8) << std::fixed << std::setprecision(2) << elapsed.count() << " s      |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Throughput:          | " << std::setw(8) << std::fixed << std::setprecision(2) << throughput << " GB/s   |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "All threads completed.\n";
    return 0;
}