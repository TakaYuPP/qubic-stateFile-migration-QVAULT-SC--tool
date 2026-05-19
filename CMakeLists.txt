cmake_minimum_required(VERSION 3.15)
project(MigrationTool VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(SOURCES
	${CMAKE_SOURCE_DIR}/migration_tool.cpp
)

set(HEADERS
	${CMAKE_SOURCE_DIR}/m256.h
	${CMAKE_SOURCE_DIR}/test_utils.h
	${CMAKE_SOURCE_DIR}/identity_k12.h
)

add_executable(qubic-stateFile-migration-tool ${SOURCES} ${HEADERS})

target_include_directories(qubic-stateFile-migration-tool PRIVATE
	${CMAKE_SOURCE_DIR}
)


if (NOT MSVC)
       target_compile_options(qubic-stateFile-migration-tool PRIVATE -mavx2 -mbmi -mlzcnt)
endif()