# Define the executable target for our tests. Let's call it "run_open_loop_tests".
# It will be built from the main.cpp file in this directory.
add_executable(run_open_loop_tests main.cpp date_time.h)

# Link our test executable against the "open_loop" static library.
# CMake handles everything from here:
# - It finds the compiled .lib or .a file.
# - It adds the necessary include directories from the library (e.g., the root "include" folder).
# - It applies the C++17 standard to this executable as well.
target_link_libraries(run_open_loop_tests PRIVATE open_loop)