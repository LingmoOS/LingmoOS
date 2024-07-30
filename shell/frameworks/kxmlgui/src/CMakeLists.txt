add_library(KF6XmlGui)
add_library(KF6::XmlGui ALIAS KF6XmlGui)

set_target_properties(KF6XmlGui PROPERTIES
    VERSION     ${KXMLGUI_VERSION}
    SOVERSION   ${KXMLGUI_SOVERSION}
    EXPORT_NAME XmlGui
)

if (HAVE_GLOBALACCEL)
    target_link_libraries(KF6XmlGui PRIVATE KF6::GlobalAccel)
endif ()
if(WIN32)
    target_link_libraries(KF6XmlGui PRIVATE secur32) # GetUserNameEx()
endif()

configure_file(config-xmlgui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xmlgui.h )

target_sources(KF6XmlGui PRIVATE
  kaboutapplicationdialog.cpp
  kaboutapplicationcomponentlistdelegate_p.cpp
  kaboutapplicationcomponentmodel_p.cpp
  kaboutapplicationlistview_p.cpp
  kaboutapplicationpersonlistdelegate_p.cpp
  kaboutapplicationpersonmodel_p.cpp
  kaboutkdedialog_p.cpp
  kaboutplugindialog.cpp
  kabstractaboutdialog_p.cpp
  kactioncategory.cpp
  kactioncollection.cpp
  kactionconflictdetector.cpp
  kbugreport.cpp
  kedittoolbar.cpp
  khelpmenu.cpp
  kkeysequencewidget.cpp
  klicensedialog_p.cpp
  kmainwindow.cpp
  kmenumenuhandler_p.cpp
  kshortcuteditwidget.cpp
  kshortcutschemeseditor.cpp
  kshortcutschemeshelper.cpp
  kshortcutsdialog.cpp
  kshortcutseditor.cpp
  kshortcutseditordelegate.cpp
  kshortcutseditoritem.cpp
  kshortcutwidget.cpp
  kswitchlanguagedialog_p.cpp
  ktoggletoolbaraction.cpp
  ktoolbar.cpp
  ktoolbarhandler.cpp
  ktoolbarhelper.cpp
  ktooltiphelper.cpp
  kxmlguibuilder.cpp
  kxmlguiclient.cpp
  kxmlguifactory.cpp
  kxmlguifactory_p.cpp
  kxmlguiversionhandler.cpp
  kxmlguiwindow.cpp
  kundoactions.cpp
  kcheckaccelerators.cpp
)
if (HAVE_QTDBUS)
    target_sources(KF6XmlGui PRIVATE kmainwindowiface.cpp)
    target_compile_definitions(KF6XmlGui PRIVATE WITH_QTDBUS)
endif()

# add the resource file
target_sources(KF6XmlGui PRIVATE
    kxmlgui.qrc
)

ecm_qt_declare_logging_category(KF6XmlGui
    HEADER debug.h
    IDENTIFIER DEBUG_KXMLGUI
    CATEGORY_NAME kf.xmlgui
    OLD_CATEGORY_NAMES kf5.kxmlgui
    DESCRIPTION "KXmlGui"
    EXPORT KXMLGUI
)

ki18n_wrap_ui(KF6XmlGui
    kshortcutsdialog.ui
    kshortcutwidget.ui
)

ecm_generate_export_header(KF6XmlGui
    BASE_NAME KXmlGui
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    USE_VERSION_HEADER
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF6XmlGui INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KXmlGui>")

target_link_libraries(KF6XmlGui
PUBLIC
    Qt6::Xml      #To parse the configuration (QDomDocument etc)
    Qt6::Widgets  #QWidget is used everywhere
    KF6::ConfigCore     #Reading config for ToolbarIcons, Shortcut Schemes...
    KF6::ConfigWidgets  #KStandardAction, KToggleAction...
PRIVATE
    Qt6::Network        #QNetworkAccessManager in kaboutapplicationpersonmodel_p
    Qt6::PrintSupport   #QPrinter in kshortcutseditor
    Qt6::CorePrivate    #QSystemLocale in initializeLanguages
    KF6::CoreAddons     #KAboutData
    KF6::GuiAddons
    KF6::WidgetsAddons
    KF6::ItemViews           #KWidgetItemDelegate in KAboutApplicationPersonListDelegate
    KF6::I18n                #i18n and i18nc in many places
    KF6::IconThemes         #KIconLoader and KIconThemes in KToolBar
    KF6::IconWidgets        #KIconDialog
)
if (HAVE_QTDBUS)
  target_link_libraries(KF6XmlGui PUBLIC Qt6::DBus) #QDBus connect to signal in KToolBar
endif()

ecm_generate_headers(KXmlGui_HEADERS
  HEADER_NAMES
  KAboutApplicationDialog
  KAboutPluginDialog
  KActionCategory
  KActionCollection
  KBugReport
  KEditToolBar
  KHelpMenu
  KKeySequenceWidget
  KMainWindow
  KShortcutsDialog
  KShortcutsEditor
  KShortcutWidget
  KToggleToolBarAction
  KToolBar
  KToolTipHelper
  KXMLGUIBuilder
  KXMLGUIClient
  KXMLGUIFactory
  KXmlGuiWindow
  KUndoActions

  REQUIRED_HEADERS KXmlGui_HEADERS
)

install(TARGETS KF6XmlGui EXPORT KF6XmlGuiTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_export.h
  ${KXmlGui_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KXmlGui COMPONENT Devel
)

if(BUILD_DESIGNERPLUGIN)
    add_subdirectory(designer)
endif()

ecm_qt_install_logging_categories(
    EXPORT KXMLGUI
    FILE kxmlgui.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

if (BUILD_QCH)
    ecm_add_qch(
        KF6XmlGui_QCH
        NAME KXmlGui
        BASE_NAME KF6XmlGui
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KXmlGui_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt6Xml_QCH
            Qt6DBus_QCH
            Qt6Widgets_QCH
            KF6Config_QCH
            KF6ConfigWidgets_QCH
        INCLUDE_DIRS
            ${CMAKE_CURRENT_BINARY_DIR}
        BLANK_MACROS
            KXMLGUI_EXPORT
            KXMLGUI_DEPRECATED_EXPORT
            KXMLGUI_DEPRECATED
            "KXMLGUI_DEPRECATED_VERSION(x, y, t)"
            "KXMLGUI_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
            "KXMLGUI_ENUMERATOR_DEPRECATED_VERSION(x, y, t)"
            "KXMLGUI_ENUMERATOR_DEPRECATED_VERSION_BELATED(x, y, xt, yt, t)"
        TAGFILE_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        QCH_INSTALL_DESTINATION ${KDE_INSTALL_QTQCHDIR}
        COMPONENT Devel
    )
endif()

