set(org.kde.desktop_SRCS
    animationspeedprovider.cpp
    plasmadesktoptheme.cpp
    kirigamiplasmafactory.cpp
    plasmadesktopunits.cpp
)

add_library(org.kde.desktop MODULE ${org.kde.desktop_SRCS})

target_link_libraries(org.kde.desktop
    PUBLIC
        Qt6::Core
        KF6::KirigamiPlatform
    PRIVATE
        Qt6::Qml
        Qt6::Quick
        KF6::ConfigCore
        KF6::ColorScheme
        KF6::IconThemes
)

if (HAVE_DBUS)
    target_link_libraries(org.kde.desktop
        PRIVATE
        Qt6::DBus
    )
    target_compile_definitions(org.kde.desktop PRIVATE -DHAVE_QTDBUS=1)
else()
    target_compile_definitions(org.kde.desktop PRIVATE -DHAVE_QTDBUS=0)
endif()

if(WIN32)
    target_link_libraries(org.kde.desktop PRIVATE user32)
endif()

install(TARGETS org.kde.desktop DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kirigami/platform)

add_subdirectory(autotests)
