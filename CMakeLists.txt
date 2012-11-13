
# Solution

project (qts3)

cmake_minimum_required (VERSION 2.8)

set (CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

# Dependencies

find_package (Qt4 4.6.0 REQUIRED COMPONENTS QtCore QtNetwork QtXml)
set (QT_INCLUDE_DIRS ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})
set (QT_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY})

# Main project

set (PROJECT_DIR ${PROJECT_BINARY_DIR})
set (SOURCE_DIR ${PROJECT_DIR}/src)
set (INCLUDE_DIR ${PROJECT_DIR}/include)

# Project

set (TARGET_NAME qts3)

file (GLOB CPP_FILES ${SOURCE_DIR}/*.cpp)
file (GLOB H_FILES ${INCLUDE_DIR}/*.h)
QT4_WRAP_CPP (MOC_SRCS ${H_FILES})

add_definitions (-DQTS3_LIBRARY)

include_directories (${INCLUDE_DIR} ${QT_INCLUDE_DIRS})
add_library (${TARGET_NAME} SHARED ${CPP_FILES} ${H_FILES} ${MOC_SRCS})
target_link_libraries (${TARGET_NAME} ${QT_LIBRARIES})

set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d) 
set_target_properties (${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/lib)
set_target_properties (${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
set_target_properties (${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)

if (MSVC)
    file(GLOB MOCS_TO_SOURCE_GROUP ${INCLUDE_DIR}/*.cxx)
    source_group("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endif()

# Tester

set (TARGET_NAME qts3tester)

file (GLOB CPP_FILES QS3Tester.cpp)
file (GLOB H_FILES QS3Tester.h)
QT4_WRAP_CPP (MOC_SRCS ${H_FILES})

include_directories (${INCLUDE_DIR} ${QT_INCLUDE_DIRS})
link_directories (${PROJECT_DIR}/lib)
add_executable (${TARGET_NAME} ${CPP_FILES} ${H_FILES} ${MOC_SRCS})
target_link_libraries (${TARGET_NAME} ${QT_LIBRARIES} optimized qts3.lib debug qts3d.lib)

set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d) 
set_target_properties (${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/lib)
set_target_properties (${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
set_target_properties (${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)