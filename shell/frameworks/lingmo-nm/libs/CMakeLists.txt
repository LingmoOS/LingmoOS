add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement-libs\")




add_library(lingmonm_internal SHARED)
generate_export_header(lingmonm_internal)

ecm_qt_declare_logging_category(lingmonm_internal HEADER lingmo_nm_libs.h IDENTIFIER LINGMO_NM_LIBS_LOG CATEGORY_NAME org.kde.lingmo.nm.libs DESCRIPTION "Lingmo NM (libs)" EXPORT LINGMONM)
target_sources(lingmonm_internal PRIVATE
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

target_include_directories(lingmonm_internal PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/models)

target_link_libraries(lingmonm_internal
PUBLIC
    KF6::NetworkManagerQt
    lingmonm_editor
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

target_link_libraries(lingmonm_internal PUBLIC KF6::ModemManagerQt)

ecm_add_qml_module(lingmonm_internal URI org.kde.lingmo.networkmanagement GENERATE_PLUGIN_SOURCE)
ecm_finalize_qml_module(lingmonm_internal)

install(TARGETS lingmonm_internal ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

add_subdirectory(editor)
