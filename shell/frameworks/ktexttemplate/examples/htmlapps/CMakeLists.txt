cmake_minimum_required(VERSION 3.5)

project(htmlapps)

find_package(KF6TextTemplate REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)

get_property(KTextTemplate_PLUGIN_DIR TARGET KF6::defaulttags PROPERTY LOCATION)
get_filename_component(KTextTemplate_PLUGIN_DIR ${KTextTemplate_PLUGIN_DIR} PATH)
get_filename_component(KTextTemplate_PLUGIN_DIR ${KTextTemplate_PLUGIN_DIR} PATH)
get_filename_component(KTextTemplate_PLUGIN_DIR ${KTextTemplate_PLUGIN_DIR} PATH)

configure_file(ktexttemplate_paths.h.cmake ${PROJECT_BINARY_DIR}/ktexttemplate_paths.h)

add_library(customplugin MODULE
  customplugin.cpp
  rssfeed.cpp
)
ktexttemplate_adjust_plugin_name(customplugin)

find_package(Qt5 REQUIRED XmlPatterns Network)

target_link_libraries(customplugin
  KF6::TextTemplate
  Qt6::XmlPatterns
  Qt6::Network
)

add_executable(htmlapps
  main.cpp
  mainwindow.cpp
  localrequestmanager.cpp
  templatereply.cpp
)

find_package(Qt5WebKitWidgets REQUIRED)

target_link_libraries(htmlapps
  KF6::TextTemplate
  Qt6::WebKitWidgets
  )
