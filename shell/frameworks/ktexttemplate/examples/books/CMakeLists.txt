cmake_minimum_required(VERSION 3.5)

project(books_ktexttemplate)

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

find_package(Qt6Widgets REQUIRED)
find_package(Qt6Sql REQUIRED)

add_executable(books
  main.cpp
  bookwindow.cpp
  bookwrapper.cpp
  bookdelegate.cpp
  books.qrc
)

target_link_libraries(books
  KF6::TextTemplate
  Qt6::Widgets
  Qt6::Sql
)
