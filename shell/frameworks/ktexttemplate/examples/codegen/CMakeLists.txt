cmake_minimum_required(VERSION 3.5)

project(codegen_ktexttemplate)

find_package(KF6TextTemplate REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

get_property(KTextTemplate_PLUGIN_DIR TARGET KF6::defaulttags PROPERTY LOCATION)
get_filename_component(KTextTemplate_PLUGIN_DIR ${KTextTemplate_PLUGIN_DIR} PATH)
get_filename_component(KTextTemplate_PLUGIN_DIR ${KTextTemplate_PLUGIN_DIR} PATH)
get_filename_component(KTextTemplate_PLUGIN_DIR ${KTextTemplate_PLUGIN_DIR} PATH)

configure_file(ktexttemplate_paths.h.cmake ${PROJECT_BINARY_DIR}/ktexttemplate_paths.h)

qt6_add_resources(
  codegen_example_RCS_SRCS
  custom_tags.qrc
  OPTIONS -root "/plugins/kf6/ktexttemplate/"
)

add_executable(codegen
  codegen.qrc
  comboboxdelegate.cpp
  main.cpp
  methodmodel.cpp
  codegentableview.cpp
  designwidget.cpp
  propertytablewidget.cpp
  mainwindow.cpp
  ${codegen_example_RCS_SRCS}
)
target_compile_definitions(codegen PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0)

find_package(Qt6Widgets REQUIRED)

target_link_libraries(
  codegen
  KF6::TextTemplate
  Qt6::Widgets
)
