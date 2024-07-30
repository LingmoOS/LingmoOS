add_definitions(-DTRANSLATION_DOMAIN=\"iconexplorer_editorplugin\")

include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(iconexplorerplugin_PART_SRCS
    iconexplorerplugin.cpp
)

add_library (iconexplorerplugin MODULE ${iconexplorerplugin_PART_SRCS})

target_link_libraries(iconexplorerplugin
    KF6::TextEditor
    KF6::IconThemes
    KF6::I18n
)

install(TARGETS iconexplorerplugin  DESTINATION ${KDE_INSTALL_PLUGINDIR}/ktexteditor)

