set(TARGET_NAME dtk-settings)
set(BIN_NAME ${TARGET_NAME}${DTK_VERSION_MAJOR})

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Xml)

if(${QT_VERSION_MAJOR} STREQUAL "5")
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(QGSettings REQUIRED IMPORTED_TARGET gsettings-qt)
endif()

add_executable(${BIN_NAME}
  main.cpp
)
target_link_libraries(
  ${BIN_NAME} PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Xml
  ${LIB_NAME}
)

if(${QT_VERSION_MAJOR} STREQUAL "5")
target_link_libraries(
  ${BIN_NAME} PRIVATE
  PkgConfig::QGSettings
)
endif()

target_include_directories( ${BIN_NAME} PUBLIC
  ../../include/util/
  ../../include/dci/
  ../../include/log/
  ../../include/base/
  ../../include/global/
  ../../include/DtkCore/
  ../../include/settings/
  ../../include/filesystem/
  ../../include/
)
set_target_properties(${BIN_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
install(TARGETS ${BIN_NAME} DESTINATION "${TOOL_INSTALL_DIR}")
