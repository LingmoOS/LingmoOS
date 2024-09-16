if(NOT TARGET CuteIPC)

  # Module subdirectory
  set(CUTEIPC_DIR "${PROJECT_SOURCE_DIR}/3rdparty/CuteIPC")

  # set build as share library
  if(MSVC)
    set(BUILD_SHARED_LIBS OFF)
  else()
    set(BUILD_SHARED_LIBS ON)
  endif()

  # Module subdirectory
  add_subdirectory("${CUTEIPC_DIR}" CuteIPC)
  include_directories(${CUTEIPC_DIR}/include)

if(MSVC)
  # 拷贝输出文件到应用
  file(GLOB OUTPUTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/CuteIPC.*)
  file(COPY ${OUTPUTS}
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dde-cooperation/${CMAKE_BUILD_TYPE})
  message("   >>> copy CuteIPC output libraries:  ${OUTPUTS}")
endif()

endif()
