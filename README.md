# BandwidthTest - Memory Stress Test for Measuring Throughput

## Description
`BandwidthTest` is a C++ program designed to stress-test memory by performing high-frequency read-write operations with multiple threads. The program maximizes memory bus saturation to assess the memory bandwidth performance of the system. It utilizes AVX2 instructions for fast, vectorized memory access and measures the throughput in GB/s.

The program uses a 1 GB buffer and runs with a configurable number of threads. The main objective is to test memory throughput under heavy load, providing real-time statistics such as total bytes processed, elapsed time, and throughput in GB/s.

## Features
- **Memory stress testing** with maximum read-write operations.
- Uses **AVX2** SIMD instructions for high throughput.
- Supports **multi-threading** for parallel memory access and improved performance.
- Real-time performance metrics including **memory throughput** in **GB/s**.
- Configurable thread count via command-line arguments.
## Requirements
- C++17 compatible compiler
- AVX2-enabled processor
- CMake (for building the project)
- Using [Kaizen](https://github.com/heinsaar/kaizen) library for parsing command-line arguments for thread count. See [example](https://github.com/heinsaar/kaizen/blob/master/Examples.md#program-arguments)
  ```
  int main(int argc, char* argv[]) {
    // Parse command-line arguments with Kaizen
    zen::cmd_args args(argv, argc);
    size_t thread_count = args.accept("--threads").count_accepted();
  ```
## Build Instructions

1. **Clone the repository**:
    ```bash
    git clone https://github.com/marybadalyan/BandwidthTest
    ```

2. **Go into the repository**:
    ```bash
    cd BandwidthTest
    ```

3. **Generate the build files**:
    ```bash
    cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
    ```

4. **Build the project**:
    ```bash
    cmake --build build --config Release
    ```

5. **Run the executable** generated in the build directory:
    ```bash
    ./build/BandwidthTest
    ```

### Platform-Specific Compiler Flags:
The project uses AVX2 instructions to maximize memory throughput. The `CMakeLists.txt` includes platform-specific flags:
- **Windows**: `/arch:AVX2`
- **Linux**: `-mavx -mavx2 -march=native`
- **macOS**: `-mavx -march=native` (AVX2 may not be available)

## Usage
Once compiled, run the program to start the memory stress test:

```bash
./BandwidthTest --threads [num] // num as in int or ```std::thread::hardware.councurrency```
```

The program will:
- Display the number of threads being used.
- Output the total size of the buffer (1 GB).
- Measure memory throughput and provide the following statistics:
    - **Buffer Size** (in GB)
    - **Thread Count** 
    - **Bytes processed** (in GB)
    - **Elapsed time** (in seconds)
    - **Throughput** (in GB/s)

You can modify the number of threads or the number of iterations in the `main.cpp` file to adjust the test parameters.

## Example Output:
```
+----------------------+-----------------+
| Performance Metrics  |    Value        |
+----------------------+-----------------+
| Buffer Size:         |     1.00 GB     |
+----------------------+-----------------+
| Thread Count:        |        4        |
+----------------------+-----------------+
| Bytes processed:     |    40.00 GB     |
+----------------------+-----------------+
| Elapsed Time:        |     1.04 s      |
+----------------------+-----------------+
| Throughput:          |    38.34 GB/s   |
+----------------------+-----------------+
All threads completed.
```

## Code Overview:
### Main Test:
- The memory is divided into chunks based on the number of threads, and each thread performs read and write operations using AVX2.
- The test runs for a predefined number of iterations (`ITERATIONS`), and each iteration involves reading 128 bytes and writing back to the memory in an optimized manner using AVX2.
- The throughput is calculated by measuring the total data processed and dividing it by the elapsed time.

### CMake Configuration:
- The `CMakeLists.txt` file configures platform-specific flags for compiler optimizations.
- It ensures that AVX2 support is enabled for all platforms, including Windows, Linux, and macOS.

## Customization:
- **Buffer size**: Adjust the `BUFFER_SIZE` variable to change the size of the test buffer.
- **Thread count**: Set the number of threads for parallel processing by modifying the `THREAD_COUNT` variable.
- **Iterations**: Adjust the `ITERATIONS` variable to control the number of test iterations (higher values will stress the system for longer periods).
