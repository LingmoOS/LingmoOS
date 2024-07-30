include(${CMAKE_CURRENT_SOURCE_DIR}/IncludeStaticDependencies.cmake)

add_subdirectory(platform)
add_subdirectory(primitives)
add_subdirectory(delegates)
add_subdirectory(dialogs)
add_subdirectory(layouts)

add_library(LingmoUI)
add_library(KF6::LingmoUI ALIAS LingmoUI)

if (WIN32)
    # LingmoUI apparently adds too many sources so on Windows we end
    # up running into command line length limits. So disable cache
    # generation on Windows for now.
    set(_extra_options NO_CACHEGEN)
endif()
if (BUILD_SHARED_LIBS)
    set(_extra_options ${_extra_options} NO_PLUGIN_OPTIONAL)
endif()
if (ANDROID)
    set(_extra_options ${_extra_options} OPTIONAL_IMPORTS org.kde.ocean)
endif()

# Module: org.kde.lingmoui.private

add_library(LingmoUIPrivate)
ecm_add_qml_module(LingmoUIPrivate
    URI "org.kde.lingmoui.private"
    GENERATE_PLUGIN_SOURCE
)

set_target_properties(LingmoUIPrivate PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUIPrivate"
)

target_sources(LingmoUIPrivate PRIVATE
    copyhelper.cpp
    copyhelper.h
    actionhelper.cpp
    actionhelper.h
)

include_static_dependencies(LingmoUIPrivate EXPORT LingmoUITargets)

target_link_libraries(LingmoUIPrivate PRIVATE Qt6::Gui)

ecm_finalize_qml_module(LingmoUIPrivate DESTINATION ${KDE_INSTALL_QMLDIR})

if (NOT BUILD_SHARED_LIBS)
    install(TARGETS LingmoUIPrivateplugin EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
endif()

install(TARGETS LingmoUIPrivate EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

# Module: org.kde.lingmoui

ecm_add_qml_module(LingmoUI URI "org.kde.lingmoui" VERSION 2.0
    CLASS_NAME LingmoUIPlugin
    DEPENDENCIES
    "QtQuick.Controls"
    "org.kde.lingmoui.private"
    IMPORTS
    "org.kde.lingmoui.platform"
    "org.kde.lingmoui.primitives"
    "org.kde.lingmoui.delegates"
    "org.kde.lingmoui.dialogs"
    "org.kde.lingmoui.layouts"
    ${_extra_options}
)

ecm_create_qm_loader(lingmoui_QM_LOADER liblingmoui6_qt)
target_sources(LingmoUI PRIVATE ${lingmoui_QM_LOADER})

ecm_qt_declare_logging_category(LingmoUI
    HEADER loggingcategory.h
    IDENTIFIER LingmoUILog
    CATEGORY_NAME kf.lingmoui
    DESCRIPTION "LingmoUI"
    DEFAULT_SEVERITY Warning
    EXPORT LINGMOUI
)

set_target_properties(LingmoUI PROPERTIES
    VERSION     ${PROJECT_VERSION}
    SOVERSION   6
    EXPORT_NAME "LingmoUI"
)

target_include_directories(LingmoUI PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/platform
    ${CMAKE_CURRENT_BINARY_DIR}/platform
)

target_sources(LingmoUI PRIVATE
    enums.h
    imagecolors.cpp
    imagecolors.h
    mnemonicattached.cpp
    mnemonicattached.h
    overlayzstackingattached.cpp
    overlayzstackingattached.h
    pagepool.cpp
    pagepool.h
    scenepositionattached.cpp
    scenepositionattached.h
    spellcheckattached.cpp
    spellcheckattached.h
    wheelhandler.cpp
    wheelhandler.h
)

target_sources(LingmoUIplugin PRIVATE
    lingmouiplugin.cpp
    lingmouiplugin.h
)

ecm_target_qml_sources(LingmoUI SOURCES
    controls/Action.qml
    controls/AbstractApplicationHeader.qml
    controls/AbstractApplicationWindow.qml
    controls/ApplicationWindow.qml
    controls/OverlayDrawer.qml
    controls/ContextDrawer.qml
    controls/GlobalDrawer.qml
    controls/Heading.qml
    controls/PageRow.qml
    controls/OverlaySheet.qml
    controls/Page.qml
    controls/ScrollablePage.qml
    controls/SwipeListItem.qml
    controls/Button.qml
    controls/ProgressBar.qml
    controls/Switch.qml
    controls/SwitchIndicator.qml
    controls/TabBar.qml
    controls/TabButton.qml
    controls/TextArea.qml
    controls/TextField.qml
    controls/ToolTip.qml
    controls/StackView.qml
    controls/Slider.qml
    controls/ScrollBar.qml
    controls/RadioIndicator.qml
    controls/RadioButton.qml
    controls/MenuItem.qml
    controls/RadioDelegate.qml
    controls/Menu.qml
    controls/Frame.qml
    controls/DialogButtonBox.qml
    controls/Dialog.qml
    controls/ComboBox.qml
    controls/Control.qml
    controls/CheckIndicator.qml
    controls/CheckBox.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.1 SOURCES
    controls/AbstractApplicationItem.qml
    controls/ApplicationItem.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.4 SOURCES
    controls/AbstractCard.qml
    controls/Card.qml
    controls/CardsListView.qml
    controls/CardsLayout.qml
    controls/InlineMessage.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.5 SOURCES
    controls/ListItemDragHandle.qml
    controls/ActionToolBar.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.6 SOURCES
    controls/AboutPage.qml
    controls/LinkButton.qml
    controls/UrlButton.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.7 SOURCES
    controls/ActionTextField.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.8 SOURCES
    controls/SearchField.qml
    controls/PasswordField.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.10 SOURCES
    controls/ListSectionHeader.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.11 SOURCES
    controls/PagePoolAction.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.12 SOURCES
    controls/PlaceholderMessage.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.14 SOURCES
    controls/FlexColumn.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.19 SOURCES
    controls/AboutItem.qml
    controls/NavigationTabBar.qml
    controls/NavigationTabButton.qml
    controls/Chip.qml
    controls/LoadingPlaceholder.qml
    controls/StandardItem.qml
    controls/Label.qml
    controls/RoundedItem.qml
)

ecm_target_qml_sources(LingmoUI VERSION 2.20 SOURCES
    controls/SelectableLabel.qml
    controls/InlineViewHeader.qml
    controls/ContextualHelpButton.qml
)

ecm_target_qml_sources(LingmoUI PRIVATE PATH private SOURCES
    controls/private/ActionIconGroup.qml
    controls/private/ActionMenuItem.qml
    controls/private/ActionsMenu.qml
    controls/private/BannerImage.qml
    controls/private/ContextDrawerActionItem.qml
    controls/private/DefaultCardBackground.qml
    controls/private/DefaultChipBackground.qml
    controls/private/DefaultPageTitleDelegate.qml
    controls/private/EdgeShadow.qml
    controls/private/GlobalDrawerActionItem.qml
    controls/private/MobileDialogLayer.qml
    controls/private/PrivateActionToolButton.qml
    controls/private/PullDownIndicator.qml
    controls/private/SwipeItemEventFilter.qml
)

ecm_target_qml_sources(LingmoUI PRIVATE PATH private/globaltoolbar SOURCES
    controls/private/globaltoolbar/AbstractPageHeader.qml
    controls/private/globaltoolbar/BreadcrumbControl.qml
    controls/private/globaltoolbar/PageRowGlobalToolBarStyleGroup.qml
    controls/private/globaltoolbar/PageRowGlobalToolBarUI.qml
    controls/private/globaltoolbar/TitlesPageHeader.qml
    controls/private/globaltoolbar/ToolBarPageHeader.qml
    controls/private/globaltoolbar/ToolBarPageFooter.qml
)

ecm_target_qml_sources(LingmoUI PRIVATE PATH templates SOURCES
    controls/templates/AbstractApplicationHeader.qml
    controls/templates/AbstractCard.qml
    controls/templates/Chip.qml
    controls/templates/InlineMessage.qml
    controls/templates/OverlayDrawer.qml
    controls/templates/OverlaySheet.qml
    controls/templates/SingletonHeaderSizeGroup.qml
    controls/templates/qmldir
)

ecm_target_qml_sources(LingmoUI PRIVATE PATH templates/private SOURCES
    controls/templates/private/BackButton.qml
    controls/templates/private/BorderPropertiesGroup.qml
    controls/templates/private/ContextIcon.qml
    controls/templates/private/DrawerHandle.qml
    controls/templates/private/ForwardButton.qml
    controls/templates/private/GenericDrawerIcon.qml
    controls/templates/private/IconPropertiesGroup.qml
    controls/templates/private/MenuIcon.qml
    controls/templates/private/PassiveNotificationsManager.qml
    controls/templates/private/qmldir
)

qt_target_qml_sources(LingmoUI RESOURCES
    styles/Material/InlineMessage.qml
    styles/Material/Theme.qml
)

if (DESKTOP_ENABLED)
    qt_target_qml_sources(LingmoUI RESOURCES
        styles/org.kde.desktop/AbstractApplicationHeader.qml
        styles/org.kde.desktop/Theme.qml
    )
endif()

include_static_dependencies(LingmoUI EXPORT LingmoUITargets)

target_link_libraries(LingmoUI
    PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    PRIVATE
    Qt6::Concurrent
    ${LingmoUI_EXTRA_LIBS}
)

if (HAVE_OpenMP)
    target_link_libraries(LingmoUI PRIVATE OpenMP::OpenMP_CXX)
endif()

if (NOT BUILD_SHARED_LIBS)
    # Ensure we install the plugin library file as that's required to link
    # against for static builds to work properly
    target_link_libraries(LingmoUIplugin
        PRIVATE
            LingmoUIPlatformplugin
            LingmoUIDelegatesplugin
            LingmoUIPrimitivesplugin
            LingmoUIDialogsplugin
            LingmoUILayoutsplugin
            LingmoUIPrivateplugin
    )
    install(TARGETS LingmoUIplugin EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
else()
    target_link_libraries(LingmoUI
        PUBLIC
            LingmoUIPlatform
        PRIVATE
            LingmoUIDelegates
            LingmoUIPrimitives
            LingmoUIDialogs
            LingmoUILayouts
            LingmoUIPrivate
    )
endif()

install(TARGETS LingmoUI EXPORT LingmoUITargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})
install(EXPORT LingmoUITargets
    DESTINATION ${KDE_INSTALL_CMAKEPACKAGEDIR}/KF6LingmoUI
    FILE KF6LingmoUITargets.cmake
    NAMESPACE KF6
)

ecm_finalize_qml_module(LingmoUI DESTINATION ${KDE_INSTALL_QMLDIR})

if (ANDROID)
    install(FILES LingmoUI-android-dependencies.xml
        DESTINATION ${KDE_INSTALL_LIBDIR}
        RENAME LingmoUI_${CMAKE_ANDROID_ARCH_ABI}-android-dependencies.xml
    )
endif()

ecm_qt_install_logging_categories(
    EXPORT LINGMOUI
    FILE lingmoui.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)
