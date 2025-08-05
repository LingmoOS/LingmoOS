project(rime-octagram)
cmake_minimum_required(VERSION 3.10)

option(BUILD_TOOLS "Build tools" ON)

aux_source_directory(src octagram_src)

add_library(rime-octagram-objs OBJECT ${octagram_src})
if(BUILD_SHARED_LIBS)
  set_target_properties(rime-octagram-objs
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON)
endif()

set(plugin_name rime-octagram PARENT_SCOPE)
set(plugin_objs $<TARGET_OBJECTS:rime-octagram-objs> PARENT_SCOPE)
set(plugin_deps ${rime_library} PARENT_SCOPE)
set(plugin_modules "octagram" PARENT_SCOPE)

if(BUILD_TOOLS)
  add_subdirectory(tools)
endif()
