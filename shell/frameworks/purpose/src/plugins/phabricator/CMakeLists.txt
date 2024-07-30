find_program(ARCANIST arc)
if(NOT ARCANIST)
    message(WARNING "The phabricator plugin depends on having the 'arc' script available in the PATH")
else()
    message(STATUS "The 'arc' script was found as ${ARCANIST}")
endif()

add_definitions(-DTRANSLATION_DOMAIN=\"purpose6_phabricator\")

add_subdirectory(icons)

set(PhabricatorHelper_SRCS phabricatorjobs.cpp)
ecm_qt_declare_logging_category(PhabricatorHelper_SRCS
    HEADER debug.h
    IDENTIFIER PLUGIN_PHABRICATOR
    CATEGORY_NAME kf.purpose.plugins.phabricator
    OLD_CATEGORY_NAMES kdevplatform.plugins.phabricator
    DEFAULT_SEVERITY Debug
    DESCRIPTION "purpose (phabricator plugin)"
    EXPORT PURPOSE
)

add_library(PhabricatorHelpers STATIC ${PhabricatorHelper_SRCS})
set_target_properties(PhabricatorHelpers PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_link_libraries(PhabricatorHelpers KF6::CoreAddons KF6::I18n)

add_executable(testphabricator tests/testphabricator.cpp)
ecm_mark_nongui_executable(testphabricator)
target_link_libraries(testphabricator PhabricatorHelpers Qt6::Core)

add_share_plugin(phabricatorplugin phabricatorplugin.cpp)
target_link_libraries(phabricatorplugin Qt6::Widgets PhabricatorHelpers)

add_library(phabricatorquickplugin quick/phabricatorquickplugin.cpp quick/difflistmodel.cpp quick/phabricatorrc.cpp)
target_link_libraries(phabricatorquickplugin Qt6::Qml Qt6::Gui PhabricatorHelpers)

install(TARGETS phabricatorquickplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/purpose/phabricator)
install(FILES quick/qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/purpose/phabricator)
