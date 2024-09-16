########### decoration ###############
include_directories(
    ./lib
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set(kwin5_aurorae_PART_SRCS
    aurorae.cpp
    decorationoptions.cpp
    lib/auroraetheme.cpp
    lib/themeconfig.cpp
)

add_library(kwin5_aurorae MODULE ${kwin5_aurorae_PART_SRCS})
set_target_properties(kwin5_aurorae PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org.kde.kdecoration2/"
    OUTPUT_NAME deepin-kwin5_aurorae
)

target_link_libraries(kwin5_aurorae
    KDecoration2::KDecoration
    kwineffects
    KF5::ConfigWidgets
    KF5::I18n
    KF5::Package
    KF5::WindowSystem
    Qt${QT_MAJOR_VERSION}::Quick
    Qt${QT_MAJOR_VERSION}::UiTools
)

install(TARGETS kwin5_aurorae DESTINATION ${KDE_INSTALL_PLUGINDIR}/org.kde.kdecoration2)

set(decoration_plugin_SRCS
    colorhelper.cpp
    decorationoptions.cpp
    decorationplugin.cpp
)

add_library(decorationplugin SHARED ${decoration_plugin_SRCS})
set_target_properties(decorationplugin PROPERTIES LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/org/deepin/kwin/decoration/")
target_link_libraries(decorationplugin
    KDecoration2::KDecoration
    KF5::ConfigWidgets
    Qt${QT_MAJOR_VERSION}::Quick
)
install(TARGETS decorationplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/decoration)

configure_file(deepin-aurorae.knsrc.cmake ${CMAKE_CURRENT_BINARY_DIR}/deepin-aurorae.knsrc)

########### install files ###############

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/deepin-aurorae.knsrc DESTINATION ${KDE_INSTALL_KNSRCDIR})
install(
    FILES
        qml/AppMenuButton.qml
        qml/AuroraeButton.qml
        qml/AuroraeButtonGroup.qml
        qml/AuroraeMaximizeButton.qml
        qml/Decoration.qml
        qml/DecorationButton.qml
        qml/MenuButton.qml
        qml/aurorae.qml
    DESTINATION
        ${KDE_INSTALL_DATADIR}/deepin-kwin/aurorae)
set(QMLFILES
    qml/AppMenuButton.qml
    qml/ButtonGroup.qml
    qml/Decoration.qml
    qml/DecorationButton.qml
    qml/MenuButton.qml
    qml/qmldir
)
install(FILES ${QMLFILES} DESTINATION ${KDE_INSTALL_QMLDIR}/org/deepin/kwin/decoration)
file(COPY ${QMLFILES} DESTINATION ${CMAKE_BINARY_DIR}/bin/org/deepin/kwin/decoration/)
install(FILES deepin-kwindecoration.desktop DESTINATION ${KDE_INSTALL_KSERVICETYPESDIR})
