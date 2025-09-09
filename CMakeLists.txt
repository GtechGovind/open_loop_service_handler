# ====================================================================
# Project Definition & Metadata
# ====================================================================

# Set the minimum required version of CMake.
cmake_minimum_required(VERSION 3.15)

# Define the project with its name, version, and a brief description.
# This information is used by IDEs and packaging tools.
project(open_loop_service_handler
        VERSION             1.0.0
        DESCRIPTION         "A C++ library for parsing and serializing Open-Loop Transit Card data (CSA & OSA)."
        LANGUAGES           CXX
)

# Optional: Add developer information as a standard project variable.
set(PROJECT_DEVELOPER "Govind Yadav")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Developer: ${PROJECT_DEVELOPER}")

# ====================================================================
# Project-Wide Build Settings
# ====================================================================

# Set the C++ standard for the entire project for consistency.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ====================================================================
# Build the "open_loop" Static Library
# ====================================================================

# Define the library target named "open_loop".
# It is built as a STATIC library from the specified source file.
add_library(open_loop STATIC
        src/open_loop_service.cpp
)

# Specify the include directories for our library.
# The "PUBLIC" keyword is crucial. It ensures that any target linking
# to "open_loop" (like our test executable) will automatically inherit
# this include path, so it can find <open_loop/open_loop.h>.
target_include_directories(open_loop PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# ====================================================================
# Build the Test Executable
# ====================================================================

# This tells CMake to process the CMakeLists.txt file located in the "test" directory.
# This is added *after* the library is defined, as the tests depend on it.
add_subdirectory(test)