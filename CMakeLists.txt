cmake_minimum_required(VERSION 3.1)
project(MigrationTool VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(FILES ${CMAKE_SOURCE_DIR}/migration_tool.cpp
	      ${CMAKE_SOURCE_DIR}/keyUtils.cpp
)
SET(HEADER_FILES
	K12AndKeyUtil.h
	keyUtils.h
	m256.h
)
ADD_EXECUTABLE(qubic-stateFile-migration-QVAULT-SC--tool ${FILES})