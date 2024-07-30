find_package(Qt6 ${REQUIRED_QT_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)

set(applicationitemapp_SRCS
    main.cpp
)

qt_add_resources(RESOURCES resources.qrc)

add_executable(applicationitemapp ${applicationitemapp_SRCS} ${RESOURCES})
target_link_libraries(applicationitemapp
    Qt6::Core
    Qt6::Qml
    Qt6::Quick
    Qt6::Svg
    Qt6::Widgets
)

install(TARGETS applicationitemapp ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})
