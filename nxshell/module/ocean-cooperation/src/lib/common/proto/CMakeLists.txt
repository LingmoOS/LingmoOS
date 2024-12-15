cmake_minimum_required(VERSION 3.13)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
project(proto)

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/3rdparty/CppServer/cmake")
# Compiler features
include(SetCompilerFeatures)
if (NOT ANDROID)
include(SetCompilerWarnings)
endif()
include(SetPlatformFeatures)
include(SystemInformation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Proto library
file(GLOB_RECURSE PROTO_HEADER_FILES "*.h")
file(GLOB_RECURSE PROTO_INLINE_FILES "*.inl")
file(GLOB_RECURSE PROTO_SOURCE_FILES "*.cpp")
add_library(proto STATIC ${PROTO_HEADER_FILES} ${PROTO_INLINE_FILES} ${PROTO_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "${PEDANTIC_COMPILE_FLAGS}" FOLDER "proto")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(${PROJECT_NAME} PUBLIC cppcommon uuid)

if(FPIC)
    set_target_properties(${PROJECT_NAME} PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()


#install library file
#install(TARGETS
#    ${PROJECT_NAME}
 #   LIBRARY
 #   DESTINATION
 #   ${LIB_INSTALL_DIR}
#)
