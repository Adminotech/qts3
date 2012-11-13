
# Project

set (TARGET_NAME qts3tester)

file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h)

QT4_WRAP_CPP (MOC_SRCS ${H_FILES})

include_directories (${INCLUDE_DIR}/qts3 ${QT_INCLUDE_DIRS})
link_directories (${PROJECT_DIR}/lib)

add_executable (${TARGET_NAME} ${CPP_FILES} ${H_FILES} ${MOC_SRCS})

target_link_libraries (${TARGET_NAME} ${QT_LIBRARIES} optimized qts3 debug qts3d)

# Output

set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d) 
set_target_properties (${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/lib)
set_target_properties (${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
set_target_properties (${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)

if (MSVC)
    file(GLOB MOCS_TO_SOURCE_GROUP *.cxx)
    source_group("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endif()
