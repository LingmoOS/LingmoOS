########### next target ###############
include(ECMSetupVersion)
include(ECMSetupQtPluginMacroNames)

ecm_setup_version(${PROJECT_VERSION}
                  VARIABLE_PREFIX KWINEFFECTS
                  VERSION_HEADER "${CMAKE_CURRENT_BINARY_DIR}/kwineffects_version.h"
                  PACKAGE_VERSION_FILE "${CMAKE_CURRENT_BINARY_DIR}/DeepinKWinEffectsConfigVersion.cmake"
                  SOVERSION 14
)

ecm_setup_qtplugin_macro_names(
    JSON_ARG2
        "KWIN_EFFECT_FACTORY"
    JSON_ARG3
        "KWIN_EFFECT_FACTORY_ENABLED"
        "KWIN_EFFECT_FACTORY_SUPPORTED"
    JSON_ARG4
        "KWIN_EFFECT_FACTORY_SUPPORTED_ENABLED"
    CONFIG_CODE_VARIABLE
        PACKAGE_SETUP_AUTOMOC_VARIABLES
)

###  effects lib  ###
set(kwin_EFFECTSLIB_SRCS
    anidata.cpp
    kwinanimationeffect.cpp
    kwineffects.cpp
    kwineffectsex.cpp
    kwinoffscreeneffect.cpp
    kwinoffscreenquickview.cpp
    kwinquickeffect.cpp
    logging.cpp
    sharedqmlengine.cpp
)

add_library(kwineffects SHARED ${kwin_EFFECTSLIB_SRCS})
generate_export_header(kwineffects EXPORT_FILE_NAME kwineffects_export.h)
target_link_libraries(kwineffects
    PUBLIC
        KF5::ConfigCore
        KF5::CoreAddons
        KF5::WindowSystem
        XCB::XCB
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Quick
        KF5::I18n
        kwinglutils
)
if (QT_MAJOR_VERSION EQUAL "6")
    target_link_libraries(kwineffects PRIVATE Qt${QT_MAJOR_VERSION}::GuiPrivate)
endif()
if (KWIN_HAVE_XRENDER_COMPOSITING)
    target_link_libraries(kwineffects PRIVATE XCB::XFIXES)
endif()
set_target_properties(kwineffects PROPERTIES
    VERSION ${KWINEFFECTS_VERSION}
    SOVERSION ${KWINEFFECTS_SOVERSION}
    OUTPUT_NAME deepin-kwineffects
)

install(TARGETS kwineffects EXPORT DeepinKWinEffectsTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

# kwingl(es)utils library
set(kwin_GLUTILSLIB_SRCS
    kwinglplatform.cpp
    kwingltexture.cpp
    kwinglutils.cpp
    kwinglutils_funcs.cpp
    kwineglimagetexture.cpp
    logging.cpp
)

add_library(kwinglutils SHARED ${kwin_GLUTILSLIB_SRCS})
generate_export_header(kwinglutils BASE_NAME kwinglutils EXPORT_FILE_NAME kwinglutils_export.h)
target_link_libraries(kwinglutils PUBLIC XCB::XCB KF5::CoreAddons KF5::ConfigCore KF5::WindowSystem epoxy::epoxy)
set_target_properties(kwinglutils PROPERTIES
    VERSION ${KWINEFFECTS_VERSION}
    SOVERSION ${KWINEFFECTS_SOVERSION}
    OUTPUT_NAME deepin-kwinglutils
)

install(TARGETS kwinglutils EXPORT DeepinKWinEffectsTargets ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kwinconfig.h
    ${CMAKE_CURRENT_BINARY_DIR}/kwineffects_export.h
    ${CMAKE_CURRENT_BINARY_DIR}/kwinglutils_export.h
    kwinanimationeffect.h
    kwineffects.h
    kwineffectsex.h
    kwinglobals.h
    kwinglplatform.h
    kwingltexture.h
    kwinglutils.h
    kwinglutils_funcs.h
    kwinoffscreeneffect.h
    kwinoffscreenquickview.h
    kwinquickeffect.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/deepin-kwin COMPONENT Devel)

set(CMAKECONFIG_INSTALL_DIR "${KDE_INSTALL_CMAKEPACKAGEDIR}/DeepinKWinEffects")


install(FILES  "${CMAKE_CURRENT_BINARY_DIR}/DeepinKWinEffectsConfig.cmake"
               "${CMAKE_CURRENT_BINARY_DIR}/DeepinKWinEffectsConfigVersion.cmake"
        DESTINATION "${CMAKECONFIG_INSTALL_DIR}"
        COMPONENT Devel )

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/DeepinKWinEffectsConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/DeepinKWinEffectsConfig.cmake"
  INSTALL_DESTINATION  ${CMAKECONFIG_INSTALL_DIR}
)

install(EXPORT DeepinKWinEffectsTargets DESTINATION "${CMAKECONFIG_INSTALL_DIR}" FILE DeepinKWinEffectsTargets.cmake NAMESPACE DeepinKWinEffects:: )
