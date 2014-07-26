
# Project

set (TARGET_NAME qts3)

file (GLOB CPP_FILES *.cpp)
file (GLOB H_FILES *.h ${INCLUDE_DIR}/${TARGET_NAME}/*.h )

QT4_WRAP_CPP (MOC_SRCS ${H_FILES})

add_definitions (-DQTS3_LIBRARY)

include_directories (${INCLUDE_DIR}/${TARGET_NAME} ${QT_INCLUDE_DIRS})

add_library (${TARGET_NAME} SHARED ${CPP_FILES} ${H_FILES} ${MOC_SRCS})

target_link_libraries (${TARGET_NAME} ${QT_LIBRARIES})

# Output
set (DYNAMIC_DIR bin)
if (NOT MSVC)
	set (DYNAMIC_DIR lib)
endif()

set_target_properties (${TARGET_NAME} PROPERTIES DEBUG_POSTFIX d) 
set_target_properties (${TARGET_NAME} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/lib)
set_target_properties (${TARGET_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PROJECT_DIR}/${DYNAMIC_DIR})
set_target_properties (${TARGET_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)

if (MSVC)
    file(GLOB MOCS_TO_SOURCE_GROUP *.cxx __/__/include/qts3/*.cxx)
    source_group("CMake Moc" FILES ${MOCS_TO_SOURCE_GROUP})
endif()
