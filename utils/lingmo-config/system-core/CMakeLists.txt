cmake_minimum_required(VERSION 3.5)
project(lingmo-system)

file(GLOB_RECURSE SRCS_R "sources/*")
foreach(filepath ${SRCS_R})
	string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}/" "" filename ${filepath})
	list(APPEND BASESYSTEM ${filename})
endforeach()
install(FILES ${BASESYSTEM} DESTINATION /usr/system/)
