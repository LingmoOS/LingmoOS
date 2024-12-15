# add common library directory to the include path
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/common)

# some common libraries
add_subdirectory(common)

# file transfer gui and business library
add_subdirectory(data-transfer)

# cooperation gui and business library
add_subdirectory(cooperation)
