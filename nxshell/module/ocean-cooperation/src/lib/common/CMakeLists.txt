include_directories("${CMAKE_SOURCE_DIR}/3rdparty/CppServer/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/CppServer/modules/CppLogging/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/CppServer/modules/CppCommon/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/CppServer/modules/asio/asio/include")
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/CppServer/modules/CppCommon/modules/fmt/include")

include_directories("${CMAKE_SOURCE_DIR}/src")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/proto")

# # CMake module path
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/3rdparty/CppServer/cmake")

# protocol library
add_subdirectory("proto")
 
# network communication session
add_subdirectory("session")

# the file sync server and client
add_subdirectory("httpweb")

add_subdirectory("manager")
