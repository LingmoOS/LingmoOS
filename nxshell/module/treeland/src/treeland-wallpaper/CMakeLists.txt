find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS WaylandClient Quick Core Gui)
find_package(Dtk6 REQUIRED COMPONENTS Declarative)
find_package(PkgConfig)
pkg_get_variable(WAYLAND_PROTOCOLS wayland-protocols pkgdatadir)

qt_add_executable(treeland-wallpaper
    main.cpp
)

qt_generate_wayland_protocol_client_sources(treeland-wallpaper
    FILES
        ${CMAKE_SOURCE_DIR}/src/modules/personalization/protocols/treeland-personalization-manager-v1.xml
)

set(QML_FILES
    main.qml
    Wallpaper.qml
    WallpaperCard.qml
    WallpaperSetting.qml
    PropertyItemDelegate.qml
)

qt_add_qml_module(treeland-wallpaper
    URI Wallpaper
    VERSION "1.0"
    SOURCES
    personalizationmangerclient.cpp
    personalizationmangerclient.h
    wallpapercardmodel.cpp
    wallpapercardmodel.h
    QML_FILES ${QML_FILES}
    SOURCES imagehelper.h imagehelper.cpp
)

set(TRANSLATED_FILES)

if ((${QT_VERSION_MAJOR} EQUAL 6) AND (${QT_VERSION_MINOR} LESS 7))
    qt_add_lupdate(treeland-wallpaper
        TS_FILES
            translations/wallpaper.zh_CN.ts
            translations/wallpaper.en_US.ts
        NO_GLOBAL_TARGET
    )

    qt_add_lrelease(treeland-wallpaper
        TS_FILES
            translations/wallpaper.zh_CN.ts
            translations/wallpaper.en_US.ts
        QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
    )
elseif ((${QT_VERSION_MAJOR} EQUAL 6) AND (${QT_VERSION_MINOR} GREATER_EQUAL 7))
    qt_add_lupdate(
        SOURCE_TARGETS treeland-wallpaper
        TS_FILES
            translations/wallpaper.zh_CN.ts
            translations/wallpaper.en_US.ts
        NO_GLOBAL_TARGET
    )

    qt_add_lrelease(
        TS_FILES
            translations/wallpaper.zh_CN.ts
            translations/wallpaper.en_US.ts
        QM_FILES_OUTPUT_VARIABLE TRANSLATED_FILES
    )
endif()

install(FILES ${TRANSLATED_FILES} DESTINATION ${COMPONENTS_TRANSLATION_DIR})

target_link_libraries(treeland-wallpaper
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Gui
        Qt::WaylandClient
        Dtk6::Declarative
)

install(TARGETS treeland-wallpaper RUNTIME DESTINATION "${CMAKE_INSTALL_LIBEXECDIR}")
