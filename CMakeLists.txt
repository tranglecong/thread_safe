project(trlc_thread_safe)

set(TRLC_THREAD_SAFE_HEADER_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/thread_safe")
file(GLOB_RECURSE TRLC_THREAD_SAFE_HEADER_FILES "${CMAKE_CURRENT_SOURCE_DIR}/include/thread_safe/*.hpp")
file(GLOB_RECURSE TRLC_THREAD_SAFE_SOURCES_FILES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")

add_library(thread_safe ${TRLC_THREAD_SAFE_SOURCES_FILES} ${TRLC_THREAD_SAFE_HEADER_FILES})
add_library(trlc::thread_safe ALIAS thread_safe)
target_link_libraries(thread_safe PRIVATE trlc::common)

target_include_directories(thread_safe PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>"
    "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>"
    PRIVATE ${TRLC_THREAD_SAFE_HEADER_PATH}
)


if(TRLC_BUILD_TESTS)
    enable_testing()
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


# add_library(thread_safe ${THREAD_SAFE_HEADER_FILES} ${THREAD_SAFE_SOURCES_FILES})
# target_include_directories(thread_safe PUBLIC $<BUILD_INTERFACE:${THREAD_SAFE_SOURCE_DIR}/include/> # For client in subdirectory
#     $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}> # For generated files in build mode
#     $<INSTALL_INTERFACE:${THREAD_SAFE_INC_INSTALL_DIR}> # For client in install mode
#     PRIVATE ${THREAD_SAFE_HEADER_PATH}) # For source files

# target_compile_definitions(thread_safe PUBLIC
#     THREAD_SAFE=1
#     THREAD_SAFE_VERSION_MAJOR=${THREAD_SAFE_VERSION_MAJOR}
#     THREAD_SAFE_VERSION_MINOR=${THREAD_SAFE_VERSION_MINOR}
#     THREAD_SAFE_VERSION_PATCH=${THREAD_SAFE_VERSION_PATCH})

# set_target_properties(thread_safe PROPERTIES
#     OUTPUT_NAME "thread_safe-${THREAD_SAFE_VERSION}"
#     POSITION_INDEPENDENT_CODE ON)

# install(TARGETS thread_safe EXPORT thread_safeTargets
#     RUNTIME DESTINATION ${THREAD_SAFE_RUNTIME_INSTALL_DIR}
#     LIBRARY DESTINATION ${THREAD_SAFE_LIBRARY_INSTALL_DIR}
#     ARCHIVE DESTINATION ${THREAD_SAFE_ARCHIVE_INSTALL_DIR}
#     FRAMEWORK DESTINATION ${THREAD_SAFE_FRAMEWORK_INSTALL_DIR})

# # Write/install version file
# include(CMakePackageConfigHelpers)
# set(THREAD_SAFE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/cmake/thread_safe-config-version.cmake")
# write_basic_package_version_file(${THREAD_SAFE_VERSION_FILE}
#     VERSION ${THREAD_SAFE_VERSION}
#     COMPATIBILITY AnyNewerVersion)

# install(FILES ${THREAD_SAFE_HEADER_FILES} DESTINATION ${THREAD_SAFE_INC_INSTALL_DIR}/thread_safe)
# install(FILES ${THREAD_SAFE_VERSION_FILE} DESTINATION ${THREAD_SAFE_CMAKE_CONFIG_INSTALL_DIR})