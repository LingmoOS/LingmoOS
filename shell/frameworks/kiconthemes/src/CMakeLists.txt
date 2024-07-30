add_subdirectory(tools/kiconfinder)
if (KICONTHEMES_USE_QTQUICK)
    add_subdirectory(qml)
endif()
if (APPLE)
    add_subdirectory(tools/ksvg2icns)
endif()

add_subdirectory(widgets)

add_library(KF6IconThemes)
add_library(KF6::IconThemes ALIAS KF6IconThemes)

set_target_properties(KF6IconThemes PROPERTIES
    VERSION     ${KICONTHEMES_VERSION}
    SOVERSION   ${KICONTHEMES_SOVERSION}
    EXPORT_NAME IconThemes
)

target_sources(KF6IconThemes PRIVATE
    kiconcolors.cpp
    kiconcolors.h
    kiconeffect.cpp
    kiconeffect.h
    kiconengine.cpp
    kiconengine.h
    kiconengineplugin.cpp
    kiconloader.cpp
    kiconloader.h
    kicontheme.cpp
    kicontheme.h
    kquickiconprovider.h

    hicolor.qrc
    )

ecm_qt_declare_logging_category(KF6IconThemes
    HEADER debug.h
    IDENTIFIER KICONTHEMES
    CATEGORY_NAME kf.iconthemes
    OLD_CATEGORY_NAMES kf5.kiconthemes
    DESCRIPTION "KIconThemes"
    EXPORT KICONTHEMES
)

ecm_generate_export_header(KF6IconThemes
    BASE_NAME KIconThemes
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 5.0 6.5
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6IconThemes INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KIconThemes>")

target_link_libraries(KF6IconThemes
  PUBLIC
    Qt6::Gui
  PRIVATE
    Qt6::GuiPrivate
    Qt6::Svg
    KF6::Archive       # for KCompressionDevice
    KF6::I18n          # for KLocalizedString::localizedFilePath in KIconTheme
    KF6::ColorScheme
)
if (HAVE_QTDBUS)
  target_compile_definitions(KF6IconThemes PRIVATE WITH_QTDBUS)
  target_link_libraries(KF6IconThemes PRIVATE Qt6::DBus)
endif()

if(TARGET KF6::BreezeIcons)
    target_link_libraries(KF6IconThemes
            PRIVATE
            KF6::BreezeIcons
    )
endif ()

ecm_generate_headers(KIconThemes_HEADERS
  HEADER_NAMES
  KIconColors
  KIconEffect
  KIconLoader
  KIconTheme
  KIconEngine
  KQuickIconProvider

  REQUIRED_HEADERS KIconThemes_HEADERS
)

install(TARGETS KF6IconThemes EXPORT KF6IconThemesTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kiconthemes_export.h
  ${KIconThemes_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KIconThemes COMPONENT Devel
)

if(BUILD_QCH)
    ecm_add_qch(
        KF6IconThemes_QCH
        NAME KIconThemes
        BASE_NAME KF6IconThemes
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KIconThemes_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Widgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KICONTHEMES_EXPORT
            KICONTHEMES_DEPRECATED
            KICONTHEMES_DEPRECATED_EXPORT
            "KICONTHEMES_DEPRECATED_VERSION(x, y, t)"
            "KICONTHEMES_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KICONTHEMES_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KICONTHEMES_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

add_library(KIconEnginePlugin MODULE kiconengineplugin.cpp)

target_link_libraries(KIconEnginePlugin
    PRIVATE
        Qt6::Gui
        KF6::IconThemes
)

# install in an extra directory we will add to the plugin path later just for this one engine
# we can overwrite the system one with that trick
install(TARGETS KIconEnginePlugin DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/kiconthemes6/iconengines)

ecm_qt_install_logging_categories(
    EXPORT KICONTHEMES
    FILE kiconthemes.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
