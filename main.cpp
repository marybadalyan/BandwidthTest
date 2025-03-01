#include <iostream>
#include <vector>
#include <thread>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <numeric>
#include <immintrin.h> // AVX2
#include <memory>

std::atomic<bool> running(true);

const size_t BUFFER_SIZE = 1ULL * 1024ULL * 1024ULL * 1024ULL; // 1 GB
const size_t ITERATIONS = 20;
const size_t THREAD_COUNT = 10; // or std::thread::hardware_concurrency()  for dynamic thread count

// Custom allocator for 32-byte alignment
template<typename T>
struct AlignedAllocator {
    using value_type = T;
    T* allocate(std::size_t n) {
        void* ptr = _mm_malloc(n * sizeof(T), 32); // 32-byte aligned malloc
        if (!ptr) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }
    void deallocate(T* ptr, std::size_t) {
        _mm_free(ptr);
    }
};

std::vector<uint64_t, AlignedAllocator<uint64_t>> buffer(BUFFER_SIZE / sizeof(uint64_t), 0);

void memory_stress_test(size_t thread_id, uint64_t& bytes_processed) {
    size_t chunk_size = buffer.size() / THREAD_COUNT;
    size_t start = thread_id * chunk_size;
    size_t end = (thread_id == THREAD_COUNT - 1) ? buffer.size() : start + chunk_size;

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

int main() {
    std::fill(buffer.begin(), buffer.end(), 1);

    std::vector<std::thread> threads;
    std::vector<uint64_t> bytes_per_thread(THREAD_COUNT, 0);

    auto start = std::chrono::high_resolution_clock::now();

    for (size_t i = 0; i < THREAD_COUNT; ++i) {
        threads.emplace_back(memory_stress_test, i, std::ref(bytes_per_thread[i]));
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
    std::cout << "| Thread Count:        | " << std::setw(8) << THREAD_COUNT << "        |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Total Data:          | " << std::setw(8) << std::fixed << std::setprecision(2) << total_gb    << " GB     |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Elapsed Time:        | " << std::setw(8) << std::fixed << std::setprecision(2) << elapsed.count() << " s      |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "| Throughput:          | " << std::setw(8) << std::fixed << std::setprecision(2) << throughput << " GB/s   |\n";
    std::cout << "+----------------------+-----------------+\n";
    std::cout << "All threads completed.\n";
    return 0;
}


