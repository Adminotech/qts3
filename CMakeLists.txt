
# Solution

project (qts3)

cmake_minimum_required (VERSION 2.8)

set (PROJECT_DIR ${PROJECT_BINARY_DIR})
set (INCLUDE_DIR ${PROJECT_DIR}/include)
set (CMAKE_CONFIGURATION_TYPES "Release;RelWithDebInfo;Debug" CACHE STRING "Configurations" FORCE)

option(QTS3_BUILD_TESTER "Build command line test utility." OFF)

# Dependencies

find_package (Qt4 4.6.0 REQUIRED COMPONENTS QtCore QtNetwork QtXml)
set (QT_INCLUDE_DIRS ${QT_INCLUDE_DIR} ${QT_QTCORE_INCLUDE_DIR} ${QT_QTNETWORK_INCLUDE_DIR} ${QT_QTXML_INCLUDE_DIR})
set (QT_LIBRARIES ${QT_QTCORE_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY})

# Projects

add_subdirectory (src/qts3)
if (QTS3_BUILD_TESTER)
    add_subdirectory (src/qts3tester)
endif()
