add_definitions(-DTRANSLATION_DOMAIN=\"plasmanetworkmanagement-libs\")




add_library(plasmanm_internal SHARED)
generate_export_header(plasmanm_internal)

ecm_qt_declare_logging_category(plasmanm_internal HEADER plasma_nm_libs.h IDENTIFIER PLASMA_NM_LIBS_LOG CATEGORY_NAME org.kde.plasma.nm.libs DESCRIPTION "Plasma NM (libs)" EXPORT PLASMANM)
target_sources(plasmanm_internal PRIVATE
    models/appletproxymodel.cpp
    models/creatableconnectionsmodel.cpp
    models/editorproxymodel.cpp
    models/kcmidentitymodel.cpp
    models/mobileproxymodel.cpp
    models/networkitemslist.cpp
    models/networkmodel.cpp
    models/networkmodelitem.cpp
    configurationproxy.cpp
    handler.cpp
    models/appletproxymodel.h
    models/creatableconnectionsmodel.h
    models/editorproxymodel.h
    models/kcmidentitymodel.h
    models/mobileproxymodel.h
    models/networkitemslist.h
    models/networkmodel.h
    models/networkmodelitem.h
    configurationproxy.h
    handler.h
    availabledevices.cpp
    connectionicon.cpp
    enabledconnections.cpp
    enums.cpp
    networkstatus.cpp
    wirelessstatus.cpp
    availabledevices.h
    connectionicon.h
    enabledconnections.h
    enums.h
    networkstatus.h
    wirelessstatus.h
)

target_include_directories(plasmanm_internal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/models)

target_link_libraries(plasmanm_internal
PUBLIC
    KF6::NetworkManagerQt
    plasmanm_editor
    PkgConfig::NETWORKMANAGER
    QCoro::Core
PRIVATE
    Qt6::Qml
    KF6::I18n
    KF6::Notifications
    KF6::Service
    KF6::Wallet
    KF6::WindowSystem
    KF6::KIOGui
    QCoro::DBus
)

target_link_libraries(plasmanm_internal PUBLIC KF6::ModemManagerQt)

ecm_add_qml_module(plasmanm_internal URI org.kde.plasma.networkmanagement GENERATE_PLUGIN_SOURCE)
ecm_finalize_qml_module(plasmanm_internal)

install(TARGETS plasmanm_internal ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(editor)
