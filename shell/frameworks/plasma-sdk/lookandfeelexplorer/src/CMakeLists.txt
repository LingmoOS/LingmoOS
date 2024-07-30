set(lookandfeelexplorer_SRCS
    main.cpp
    lnflogic.cpp
    lnflistmodel.cpp
)

add_executable(lookandfeelexplorer ${lookandfeelexplorer_SRCS})
target_compile_definitions(lookandfeelexplorer PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")

#find_package(ActiveApp REQUIRED)

target_link_libraries(lookandfeelexplorer
    Qt::Gui
    Qt::Quick
    Qt::Widgets
    Qt::DBus
    KF6::Archive
    KF6::I18n
    KF6::Package
    KF6::KIOCore
    KF6::KIOWidgets
    Plasma::Plasma
    Plasma::PlasmaQuick
)

install(TARGETS lookandfeelexplorer ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
