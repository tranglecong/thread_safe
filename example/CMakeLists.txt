# Example CMakeLists.txt

# Define the list of example
set(EXAMPLE_SOURCES
    queue_example.cpp
    thread_example.cpp
    wait_example.cpp
    variable_example.cpp
)

# Loop through each example source and create the corresponding executable
foreach(EXAMPLE_SOURCE ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SOURCE} NAME_WE)
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SOURCE})
    target_link_libraries(${EXAMPLE_NAME} PRIVATE trlc::threadsafe)
endforeach()