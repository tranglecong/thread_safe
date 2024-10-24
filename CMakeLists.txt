project(trlc_thread_safe)

set(TRLC_THREAD_SAFE_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/thread_safe")
file(GLOB_RECURSE TRLC_THREAD_SAFE_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/thread_safe/*.hpp")
file(GLOB_RECURSE TRLC_THREAD_SAFE_SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(thread_safe ${TRLC_THREAD_SAFE_SOURCES_FILES} ${TRLC_THREAD_SAFE_HEADER_FILES})
add_library(trlc::thread_safe ALIAS thread_safe)
target_link_libraries(thread_safe PRIVATE trlc::common)

target_include_directories(thread_safe PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${TRLC_THREAD_SAFE_HEADER_PATH}
)


if(TRLC_BUILD_TESTS)
    add_subdirectory(tests)
endif()

if(TRLC_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()

if(NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(TRLC_ENUM_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/")
    install(TARGETS thread_safe
        EXPORT trlcTargets # Export this target
        DESTINATION ${TRLC_LIBRARY_INSTALL_DIR} # Install location for libraries
        INCLUDES DESTINATION ${TRLC_INC_INSTALL_DIR} # Install location for headers
    )
    install(DIRECTORY "${TRLC_THREAD_SAFE_HEADER_PATH}" # source directory
        DESTINATION ${TRLC_INC_INSTALL_DIR} # target directory
        FILES_MATCHING # install only matched files
        PATTERN "*.hpp" # select header files
    )
endif()