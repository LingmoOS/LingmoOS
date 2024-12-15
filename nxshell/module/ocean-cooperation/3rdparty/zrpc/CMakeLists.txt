cmake_minimum_required(VERSION 3.1)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(zrpc)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/include)

include(protobuf-generate)

# do not find, check system installed protobuf package
if(NOT EXISTS "${PROTOBUF_DIR}")
    find_package(Protobuf REQUIRED)
    if (${PROTOBUF_FOUND})
        message("protobuf found")
    else ()
        message(FATAL_ERROR "Cannot find Protobuf")
    endif ()

    include_directories(${Protobuf_INCLUDE_DIRS})
    set(PROTOBUF_LIB
        ${Protobuf_LIBRARY}
    )
# use the local customize protobuf package
else ()
    include_directories(${PROTOBUF_DIR}/src)
    set(PROTOBUF_LIB
        protobuf::libprotobuf
    )
endif()

FILE(GLOB SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*/*.cpp"
    )

add_library(${PROJECT_NAME} SHARED 
    ${SRC_FILES}
)
set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(${PROJECT_NAME} ${PROTOBUF_LIB} co)
add_compile_definitions(${PROJECT_NAME} EXPORT_ZRPC_API)

#install library file
install(TARGETS
    ${PROJECT_NAME}
    LIBRARY
    DESTINATION
    ${LIB_INSTALL_DIR}
)

# add_subdirectory(sample)

