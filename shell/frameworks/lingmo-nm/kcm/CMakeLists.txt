#KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"lingmonetworkmanagement-kcm\")

kcoreaddons_add_plugin(kcm_networkmanagement INSTALL_NAMESPACE "lingmo/kcms/systemsettings_qwidgets")

kcmutils_generate_desktop_file(kcm_networkmanagement)
target_sources(kcm_networkmanagement PRIVATE
    kcm.cpp
    kcm.h
)
ecm_qt_declare_logging_category(kcm_networkmanagement HEADER lingmo_nm_kcm.h IDENTIFIER LINGMO_NM_KCM_LOG CATEGORY_NAME org.kde.lingmo.nm.kcm DESCRIPTION "Lingmo NM (kcm)" EXPORT LINGMONM)

target_link_libraries(kcm_networkmanagement
    lingmonm_internal
    lingmonm_editor
    KF6::KCMUtils
    KF6::I18n
    KF6::Service
    Qt::Quick
    Qt::QuickWidgets
)

########### install files ###############

install( FILES
    qml/AddConnectionDialog.qml
    qml/ConfigurationDialog.qml
    qml/ConnectionItem.qml
    qml/ListItem.qml
    qml/main.qml

    DESTINATION ${KDE_INSTALL_DATADIR}/kcm_networkmanagement/qml)
