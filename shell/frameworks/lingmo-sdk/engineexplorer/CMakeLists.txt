set(lingmoengineexplorer_SRCS
    engineexplorer.cpp
    ktreeviewsearchline.cpp
    main.cpp
    serviceviewer.cpp
    modelviewer.cpp
)

ki18n_wrap_ui(lingmoengineexplorer_SRCS engineexplorer.ui serviceviewer.ui)

add_executable(lingmoengineexplorer ${lingmoengineexplorer_SRCS})
target_compile_definitions(lingmoengineexplorer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(lingmoengineexplorer
    KF6::I18n
    KF6::Completion
    Lingmo::Lingmo5Support
    KF6::I18n
    KF6::ConfigCore
    KF6::ConfigWidgets
)


install(TARGETS lingmoengineexplorer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(PROGRAMS org.kde.lingmoengineexplorer.desktop  DESTINATION ${KDE_INSTALL_APPDIR})
install(FILES org.kde.lingmoengineexplorer.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})

kdoctools_create_manpage (man-lingmoengineexplorer.1.docbook 1 INSTALL_DESTINATION ${KDE_INSTALL_MANDIR})
