# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"deepin-kcm_kwinrules\")
add_definitions(-DKCMRULES)

include_directories(../../)

set(kwinrules_SRCS
    ../../rulebooksettings.cpp
    ../../rules.cpp
    ../../utils/common.cpp
    ../../virtualdesktopsdbustypes.cpp
    kwinsrc.cpp
    optionsmodel.cpp
    ruleitem.cpp
    rulesmodel.cpp
    rulebookmodel.cpp
)

kconfig_add_kcfg_files(kwinrules_SRCS ../../rulesettings.kcfgc)
kconfig_add_kcfg_files(kwinrules_SRCS ../../rulebooksettingsbase.kcfgc)

add_library(KWinRulesObjects STATIC ${kwinrules_SRCS})
set_property(TARGET KWinRulesObjects PROPERTY POSITION_INDEPENDENT_CODE ON)

set(kwin_deepin-kcm_rules_XCB_LIBS
    XCB::CURSOR
    XCB::XCB
    XCB::XFIXES
)

set(deepin-kcm_libs
    Qt${QT_MAJOR_VERSION}::Quick

    KF5::KCMUtils
    KF5::I18n
    KF5::QuickAddons
    KF5::WindowSystem
    KF5::XmlGui
)

if (KWIN_BUILD_ACTIVITIES)
    set(deepin-kcm_libs ${deepin-kcm_libs} KF5::Activities)
endif()
target_link_libraries(KWinRulesObjects ${deepin-kcm_libs} ${kwin_deepin-kcm_rules_XCB_LIBS})

add_executable(kwin_rules_dialog main.cpp)
set_target_properties(kwin_rules_dialog PROPERTIES
    OUTPUT_NAME deepin-kwin_rules_dialog
)
target_link_libraries(kwin_rules_dialog KWinRulesObjects)
target_compile_definitions(kwin_rules_dialog PRIVATE KCMRULES_PLUGIN="plasma/kcms/systemsettings/deepin-kcm_kwinrules")
install(TARGETS kwin_rules_dialog DESTINATION ${KDE_INSTALL_LIBEXECDIR})

kcoreaddons_add_plugin(deepin-kcm_kwinrules SOURCES kcmrules.cpp INSTALL_NAMESPACE "plasma/kcms/systemsettings")
target_link_libraries(deepin-kcm_kwinrules KWinRulesObjects)

kpackage_install_package(package deepin-kcm_kwinrules kcms)