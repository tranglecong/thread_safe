# Tests CMakeLists.txt
# Locate GTest
include(FetchContent)
# Fetch GoogleTest if it is not found on the system
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Download and build GTest
FetchContent_MakeAvailable(googletest)

# Define the list of tests
set(TEST_SOURCES
  thread_safe_queue_test.cpp
  thread_safe_variable_test.cpp
  thread_safe_thread_test.cpp
  thread_safe_wait_test.cpp
)

# Loop through each test source and create the corresponding executable
foreach(TEST_SOURCE ${TEST_SOURCES})
  get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)

  add_executable(${TEST_NAME} ${TEST_SOURCE})
  target_link_libraries(${TEST_NAME} PRIVATE thread_safe GTest::gtest GTest::gtest_main)
  target_include_directories(${TEST_NAME} PRIVATE
    ${THREAD_SAFE_SOURCE_DIR}/include/thread_safe)
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
  set_tests_properties(${TEST_NAME} PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endforeach()