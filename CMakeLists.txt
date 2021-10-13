# Project
cmake_minimum_required(VERSION 3.13)
project(CAT)

# Programming languages to use
enable_language(C CXX)

# Find and link with LLVM
find_package(LLVM 12 REQUIRED CONFIG)
message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wall")

set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

# Link with LLVM
include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
add_definitions(
-D__STDC_LIMIT_MACROS
-D__STDC_CONSTANT_MACROS
)
link_directories(${LLVM_LIBRARY_DIRS})

# Prepare the pass to be included in the source tree
list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

# Pass
add_subdirectory(src)

# Install
install(PROGRAMS bin/cat-c DESTINATION bin)

###########
# DOXYGEN #
###########
find_package(Doxygen)
if (DOXYGEN_FOUND)
    set(DOXYGEN_GENERATE_HTML NO)
    set(DOXYGEN_GENERATE_MAN NO)
    set(DOXYGEN_GENERATE_TAGFILE ${CMAKE_BINARY_DIR}/llvm.tag)
    doxygen_add_docs(
        llvm-doxygen
        ${LLVM_INCLUDE_DIRS}
        COMMENT "Generate documentation for llvm"
    )

    unset(DOXYGEN_GENERATE_TAGFILE)
    set(DOXYGEN_GENERATE_HTML YES)
    set(DOXYGEN_GENERATE_MAN YES)
    set(DOXYGEN_TAGFILES "${CMAKE_SOURCE_DIR}/tagfiles/llvm.tag=https://llvm.org/doxygen/" "${CMAKE_SOURCE_DIR}/tagfiles/libstdc++.tag=http://gcc.gnu.org/onlinedocs/libstdc++/latest-doxygen")
    set(DOXYGEN_JAVADOC_AUTOBRIEF YES)
    doxygen_add_docs(
        doxygen
        src
        COMMENT "Generate documentation for cat-c"
    )
endif(DOXYGEN_FOUND)
