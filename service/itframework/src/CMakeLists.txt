add_subdirectory(ksendbugmail)

add_library(KF5XmlGui)
add_library(KF5::XmlGui ALIAS KF5XmlGui)

if (HAVE_GLOBALACCEL)
    target_link_libraries(KF5XmlGui PRIVATE KF5::GlobalAccel)
endif ()
if(WIN32)
    target_link_libraries(KF5XmlGui PRIVATE secur32) # GetUserNameEx()
endif()

configure_file(config-xmlgui.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-xmlgui.h )

target_sources(KF5XmlGui PRIVATE
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
if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
    target_sources(KF5XmlGui PRIVATE
        kmainwindowiface.cpp
    )
endif()

# add the resource file
target_sources(KF5XmlGui PRIVATE
    kxmlgui.qrc
)

ecm_qt_declare_logging_category(KF5XmlGui
    HEADER debug.h
    IDENTIFIER DEBUG_KXMLGUI
    CATEGORY_NAME kf.xmlgui
    OLD_CATEGORY_NAMES kf5.kxmlgui
    DESCRIPTION "KXmlGui"
    EXPORT KXMLGUI
)

ki18n_wrap_ui(KF5XmlGui
    kshortcutsdialog.ui
    kshortcutwidget.ui
)

ecm_generate_export_header(KF5XmlGui
    BASE_NAME KXmlGui
    GROUP_BASE_NAME KF
    VERSION ${KF_VERSION}
    DEPRECATED_BASE_VERSION 0
    DEPRECATION_VERSIONS 4.0 4.1 5.0 5.75 5.83 5.84
    EXCLUDE_DEPRECATED_BEFORE_AND_AT ${EXCLUDE_DEPRECATED_BEFORE_AND_AT}
)

target_include_directories(KF5XmlGui INTERFACE "$<INSTALL_INTERFACE:${KDE_INSTALL_INCLUDEDIR_KF}/KXmlGui>")

target_link_libraries(KF5XmlGui
PUBLIC
    Qt${QT_MAJOR_VERSION}::Xml      #To parse the configuration (QDomDocument etc)
    Qt${QT_MAJOR_VERSION}::Widgets  #QWidget is used everywhere
    KF5::ConfigCore     #Reading config for ToolbarIcons, Shortcut Schemes...
    KF5::ConfigWidgets  #KStandardAction, KToggleAction...
PRIVATE
    Qt${QT_MAJOR_VERSION}::Network        #QNetworkAccessManager in kaboutapplicationpersonmodel_p
    Qt${QT_MAJOR_VERSION}::PrintSupport   #QPrinter in kshortcutseditor
    Qt${QT_MAJOR_VERSION}::CorePrivate    #QSystemLocale in initializeLanguages
    KF5::CoreAddons     #KAboutData
    KF5::GuiAddons
    KF5::WidgetsAddons
    KF5::ItemViews           #KWidgetItemDelegate in KAboutApplicationPersonListDelegate
    KF5::I18n                #i18n and i18nc in many places
    KF5::IconThemes         #KIconLoader and KIconThemes in KToolBar
)
if (TARGET Qt${QT_MAJOR_VERSION}::DBus)
  target_link_libraries(KF5XmlGui PUBLIC Qt${QT_MAJOR_VERSION}::DBus) #QDBus connect to signal in KToolBar
endif()

set_target_properties(KF5XmlGui PROPERTIES VERSION   ${KXMLGUI_VERSION}
                                           SOVERSION ${KXMLGUI_SOVERSION}
                                           EXPORT_NAME XmlGui
)

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

install(TARGETS KF5XmlGui EXPORT KF5XmlGuiTargets ${KF_INSTALL_TARGETS_DEFAULT_ARGS})

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/kxmlgui_export.h
  ${KXmlGui_HEADERS}
  DESTINATION ${KDE_INSTALL_INCLUDEDIR_KF}/KXmlGui COMPONENT Devel
)

# install this file to be compatible, it is bundled in the resource file, too
install( FILES ui_standards.rc  DESTINATION  ${KDE_INSTALL_CONFDIR}/ui )

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
        KF5XmlGui_QCH
        NAME KXmlGui
        BASE_NAME KF5XmlGui
        VERSION ${KF_VERSION}
        ORG_DOMAIN org.kde
        SOURCES # using only public headers, to cover only public API
            ${KXmlGui_HEADERS}
        MD_MAINPAGE "${CMAKE_SOURCE_DIR}/README.md"
        IMAGE_DIRS "${CMAKE_SOURCE_DIR}/docs/pics"
        LINK_QCHS
            Qt5Xml_QCH
            Qt5DBus_QCH
            Qt5Widgets_QCH
            KF5Config_QCH
            KF5ConfigWidgets_QCH
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

include(ECMGeneratePriFile)
ecm_generate_pri_file(BASE_NAME KXmlGui LIB_NAME KF5XmlGui DEPS "dbus xml widgets KConfigCore KConfigWidgets" FILENAME_VAR PRI_FILENAME INCLUDE_INSTALL_DIR ${KDE_INSTALL_INCLUDEDIR_KF}/KXmlGui)
install(FILES ${PRI_FILENAME} DESTINATION ${ECM_MKSPECS_INSTALL_DIR})
