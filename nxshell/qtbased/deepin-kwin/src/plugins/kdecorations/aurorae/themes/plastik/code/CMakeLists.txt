set(plastik_plugin_SRCS
    plastikbutton.cpp
    plastikplugin.cpp
)

add_library(plastikplugin SHARED ${plastik_plugin_SRCS})
set_target_properties(plastikplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org/deepin/kwin/decorations/plastik")
target_link_libraries(plastikplugin Qt${QT_MAJOR_VERSION}::Core Qt${QT_MAJOR_VERSION}::Quick KF5::ConfigWidgets)
install(TARGETS plastikplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/decorations/plastik)
install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/decorations/plastik)
file(COPY qmldir DESTINATION ${CMAKE_BINARY_DIR}/bin/org/deepin/kwin/decorations/plastik)
