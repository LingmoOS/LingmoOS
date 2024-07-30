

set(plasmathemeexplorer_SRCS
    main.cpp
    thememodel.cpp
    themelistmodel.cpp
    coloreditor.cpp
)

add_executable(plasmathemeexplorer ${plasmathemeexplorer_SRCS})
target_compile_definitions(plasmathemeexplorer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

#find_package(ActiveApp REQUIRED)

target_link_libraries(plasmathemeexplorer
 Qt::Gui
 Qt::Quick
 Qt::Widgets
 Qt::Xml
 KF6::Archive
 KF6::I18n
 KF6::KIOCore
 KF6::KIOWidgets
 KF6::Package
 Plasma::PlasmaQuick
)

install(TARGETS plasmathemeexplorer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.plasma.themeexplorer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
