

set(lingmothemeexplorer_SRCS
    main.cpp
    thememodel.cpp
    themelistmodel.cpp
    coloreditor.cpp
)

add_executable(lingmothemeexplorer ${lingmothemeexplorer_SRCS})
target_compile_definitions(lingmothemeexplorer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

#find_package(ActiveApp REQUIRED)

target_link_libraries(lingmothemeexplorer
 Qt::Gui
 Qt::Quick
 Qt::Widgets
 Qt::Xml
 KF6::Archive
 KF6::I18n
 KF6::KIOCore
 KF6::KIOWidgets
 KF6::Package
 Lingmo::LingmoQuick
)

install(TARGETS lingmothemeexplorer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
install(FILES org.kde.lingmo.themeexplorer.desktop DESTINATION ${KDE_INSTALL_APPDIR})
