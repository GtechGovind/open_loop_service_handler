# Open-Loop Transit Service Handler

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++ Standard](https://img.shields.io/badge/C++-17-blue.svg)](https://isocpp.org/std/the-standard)
[![CMake](https://img.shields.io/badge/CMake-3.15%2B-blue.svg)](https://cmake.org/)

A professional, type-safe, and robust C++17 library for parsing, manipulating, and serializing data structures for open-loop transit cards. This library provides a complete, object-oriented model for the 96-byte **Common Service Area (CSA)** and a flexible 96-byte **Operator Service Area (OSA)** as specified by NCMC and related standards.

The entire library is designed with a focus on data integrity, type safety, and ease of use. It abstracts away the complex bit-level details, allowing developers to work with clean, validated C++ objects.

## ✨ Features

-   **Type-Safe C++17 Design**: Utilizes modern C++ features like `enum class`, `std::optional`, `constexpr`, and `[[nodiscard]]` to prevent common errors.
-   **Robust Data Validation**: All setter methods perform strict range and format checking, throwing standard exceptions on invalid input to ensure data integrity.
-   **Symmetrical Serialization**: Guaranteed symmetrical `to_bytes()` and `parse()` operations ensure no data is lost or corrupted during serialization cycles.
-   **Object-Oriented Abstraction**: Models each distinct data block (General, Terminal, Validation, Log, History, Trip Pass) as a fully encapsulated class.
-   **Comprehensive Documentation**: The entire API is thoroughly documented using Doxygen-style comments directly in the header file.
-   **Modern CMake Build System**: Provides a clean, cross-platform build configuration for the static library and its comprehensive test suite.
-   **Extensively Tested**: Includes a full suite of unit tests covering functionality, error handling, stateful logic, and data integrity.

## ⚙️ Requirements

-   **CMake**: Version 3.15 or higher
-   **C++ Compiler**: A compiler with full C++17 support (e.g., GCC 7+, Clang 6+, MSVC 2017+).

## 🚀 Building the Project

The project is configured to build the core logic as a static library (`open_loop`) and a test executable (`run_open_loop_tests`).

### Command-Line Instructions

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your_username/open_loop_service_handler.git
    cd open_loop_service_handler
    ```

2.  **Configure the project with CMake:**
    ```bash
    mkdir build
    cd build
    cmake ..
    ```

3.  **Build the library and tests:**
    ```bash
    cmake --build .
    ```

The compiled static library (`libopen_loop_service.a` or `open_loop_service.lib`) and the test executable will be located in the `build/` directory.

### Building with CLion

Simply open the project's root directory in CLion. The IDE will automatically detect the `CMakeLists.txt` files and configure the project. You can then select the `run_open_loop_tests` target and click "Build" or "Run".

## ✅ Running the Tests

After a successful build, you can run the comprehensive test suite from the build directory:

```bash
cd build/test
./run_open_loop_tests
```

A successful run will show a report with all tests passing.

## 💡 Usage Example

The library is designed to be intuitive. Here is a basic example of creating, populating, serializing, and parsing a Common Service Area (`CSA`) object.

```cpp
#include <iostream>
#include <cassert>
#include "open_loop/open_loop.h"

using namespace open_loop;

int main() {
    // 1. Define the card's effective date in minutes since the Unix epoch.
    // This is a mandatory step for time-sensitive data.
    constexpr std::time_t card_effective_date = 28399680;

    // 2. Create the main CSA container.
    csa::container original_csa(card_effective_date);

    // 3. Populate the data using the high-level API.
    original_csa.get_general().set_version(1, 1, 0);
    original_csa.get_general().set_language(language_code::English);

    csa::terminal term;
    term.set_acquirer_id(20);
    term.set_operator_id(1024);
    term.set_terminal_id("AABBCC");
    original_csa.get_validation().set_terminal_info(term);
    original_csa.get_validation().set_fare_amount(1500);

    // 4. Serialize the entire 96-byte structure into a byte vector.
    std::vector<uint8_t> bytes_to_write = original_csa.to_bytes();
    assert(bytes_to_write.size() == 96);

    // 5. Parse the raw bytes back into a new object.
    //    The same effective date must be used to correctly interpret time offsets.
    csa::container parsed_csa = csa::container::parse(bytes_to_write, card_effective_date);

    // 6. Verify that the parsed data is identical to the original.
    assert(original_csa == parsed_csa);

    std::cout << "Successfully created, serialized, and parsed a CSA object!" << std::endl;
    std::cout << "\n--- Parsed CSA Object ---" << std::endl;
    std::cout << parsed_csa << std::endl;

    return 0;
}
```

## 📚 API Reference

The entire public API is documented within the `include/open_loop/open_loop.h` header file using Doxygen-style comments. For detailed information on specific classes, methods, and parameters, please refer directly to the source code.

You can also generate a full set of HTML documentation by running Doxygen on the project.

### Data Structure Overview

#### Common Service Area (CSA) - 96 Bytes

| Offset (Bytes) | Size (Bytes) | Description                  | Managed By            |
| :------------- | :----------- | :--------------------------- | :-------------------- |
| 0-1            | 2            | Version & Language Info      | `csa::general`        |
| 2-20           | 19           | Last Validation Record       | `csa::validation`     |
| 21-88          | 68           | Transaction History (4 logs) | `csa::history`        |
| 89-95          | 7            | Reserved for Future Use      | `std::array<uint8_t, 7>` |

#### Operator Service Area (OSA) - 96 Bytes

| Offset (Bytes) | Size (Bytes) | Description                       | Managed By                 |
| :------------- | :----------- | :-------------------------------- | :------------------------- |
| 0-6            | 7            | Version, Phone (BCD), etc.        | `osa::general`             |
| 7-19           | 13           | Last Validation Record            | `osa::transaction_record`  |
| 20-45          | 26           | Transaction History (2 records)   | `osa::history`             |
| 46-65          | 20           | Trip Pass Product Slot 1          | `osa::trip_pass`           |
| 66-85          | 20           | Trip Pass Product Slot 2          | `osa::trip_pass`           |
| 86-95          | 10           | Padding (zero-filled)             | N/A                        |


## 🤝 Contributing

Contributions are welcome! Please feel free to open an issue to report bugs or suggest features, or submit a pull request with your improvements.

## 📄 License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
