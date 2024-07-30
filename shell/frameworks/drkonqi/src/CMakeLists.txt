add_definitions(-DTRANSLATION_DOMAIN=\"drkonqi\")

include(CheckFunctionExists)

check_function_exists("strsignal" HAVE_STRSIGNAL)
check_function_exists("uname" HAVE_UNAME)

if(NOT DEBUG_PACKAGE_INSTALLER_NAME)
    set(DEBUG_PACKAGE_INSTALLER_NAME "installdbgsymbols.sh")
endif()

configure_file(config-drkonqi.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-drkonqi.h)

add_subdirectory(bugzillaintegration/libbugzilla)
add_subdirectory(data)
add_subdirectory(parser)

set(drkonqi_SRCS
    drkonqidialog.cpp
    statuswidget.cpp
    backtraceratingwidget.cpp
    backtracewidget.cpp
    backtracegenerator.cpp
    drkonqi.cpp
    drkonqibackends.cpp
    detachedprocessmonitor.cpp
    debugpackageinstaller.cpp
    systeminformation.cpp
    crashedapplication.cpp
    debugger.cpp
    debuggerlaunchers.cpp
    ptracer.cpp
    debuggermanager.cpp
    statusnotifier.cpp
    statusnotifier_activationclosetimer.cpp
    linuxprocmapsparser.cpp
    drkonqi_globals.cpp
    qmlextensions/platformmodel.cpp
    qmlextensions/reproducibilitymodel.cpp
    qmlextensions/credentialstore.cpp
    qmlextensions/commentmodel.cpp
    qmlextensions/qobjectlistmodel.cpp
    qmlextensions/doctore.cpp
    drkonqidialog.h
    statuswidget.h
    backtraceratingwidget.h
    backtracewidget.h
    backtracegenerator.h
    drkonqi.h
    drkonqibackends.h
    detachedprocessmonitor.h
    debugpackageinstaller.h
    systeminformation.h
    crashedapplication.h
    debugger.h
    debuggerlaunchers.h
    ptracer.h
    debuggermanager.h
    statusnotifier.h
    statusnotifier_activationclosetimer.h
    linuxprocmapsparser.h
    drkonqi_globals.h
    qmlextensions/platformmodel.h
    qmlextensions/reproducibilitymodel.h
    qmlextensions/credentialstore.h
    qmlextensions/commentmodel.h
    qmlextensions/qobjectlistmodel.h
    qmlextensions/doctore.h
)

ki18n_wrap_ui(drkonqi_SRCS
    ui/maindialog.ui
    ui/backtracewidget.ui
)

# if BACKTRACE_PARSER_DEBUG is enabled, it will show both the
# parsed and the unparsed backtrace in the backtrace widget.
# Comment this out for release.
#add_definitions(-DBACKTRACE_PARSER_DEBUG)

set(drkonqi_SRCS
    ${drkonqi_SRCS}
    bugzillaintegration/bugzillalib.cpp
    bugzillaintegration/reportinterface.cpp
    bugzillaintegration/productmapping.cpp
    bugzillaintegration/bugzillalib.h
    bugzillaintegration/reportinterface.h
    bugzillaintegration/productmapping.h
)

ecm_qt_declare_logging_category(
    drkonqi_SRCS
    HEADER drkonqi_debug.h
    IDENTIFIER DRKONQI_LOG
    CATEGORY_NAME org.kde.drkonqi
    DESCRIPTION "drkonqi"
    EXPORT DRKONQI
)

# transient static lib we can use to link autotests against
add_library(DrKonqiInternal STATIC ${drkonqi_SRCS})
kconfig_add_kcfg_files(DrKonqiInternal GENERATE_MOC settings.kcfgc)

target_link_libraries(
    DrKonqiInternal
    DrKonqiSentryInternal
    KF6::I18n
    KF6::CoreAddons
    KF6::ConfigGui
    KF6::JobWidgets
    KF6::KIOCore
    KF6::KIOGui
    KF6::Crash
    KF6::WindowSystem
    Qt::DBus
    Qt::Concurrent
    Qt::Qml
    KF6::WidgetsAddons
    KF6::Wallet
    KF6::Notifications # for status notifier
    KF6::IdleTime # hide status notifier only if user saw it
    KF6::SyntaxHighlighting # Backtrace Highlighting
    KF6::StatusNotifierItem
    drkonqi_backtrace_parser
    qbugzilla
)

if(Systemd_FOUND)
    add_subdirectory(coredump)

    target_sources(DrKonqiInternal PRIVATE coredumpbackend.cpp coredumpbackend.h)
    target_link_libraries(DrKonqiInternal drkonqi-coredump) # needed for logs extraction
    target_link_libraries(DrKonqiInternal drkonqi-coredumpexcavator)
    target_compile_definitions(DrKonqiInternal PRIVATE SYSTEMD_AVAILABLE)
endif()

add_executable(drkonqi main.cpp qml/qml.qrc)
ecm_mark_nongui_executable(drkonqi)
target_link_libraries(drkonqi DrKonqiInternal)

install(TARGETS drkonqi DESTINATION ${KDE_INSTALL_LIBEXECDIR})
configure_file(org.kde.drkonqi.desktop.cmake ${CMAKE_BINARY_DIR}/src/org.kde.drkonqi.desktop)
install(PROGRAMS ${CMAKE_BINARY_DIR}/src/org.kde.drkonqi.desktop DESTINATION ${KDE_INSTALL_APPDIR})

add_subdirectory(sentry)

if(BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(bugzillaintegration/libbugzilla/autotests)
endif()
