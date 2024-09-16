if(NOT TARGET QtZeroConf)

  # Module subdirectory
  set(QTZEROCONF_DIR "${PROJECT_SOURCE_DIR}/3rdparty/QtZeroConf")

  # set build as share library
  set(BUILD_SHARED_LIBS ON)

  # Module subdirectory
  add_subdirectory("${QTZEROCONF_DIR}" QtZeroConf)

if(MSVC)
  # 拷贝输出文件到应用
  file(GLOB OUTPUTS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_BUILD_TYPE}/QtZeroConf.*)
  file(COPY ${OUTPUTS}
    DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dde-cooperation/${CMAKE_BUILD_TYPE})
  message("   >>> copy QtZeroConf output libraries:  ${OUTPUTS}")
endif()

endif()
