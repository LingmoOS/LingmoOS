# KI18N Translation Domain for this library
add_definitions(-DTRANSLATION_DOMAIN=\"kcm_energyinfo\")

set(kcm_energyinfo_SRCS
    batterymodel.cpp
    batterymodel.h
    kcm.cpp
    statisticsprovider.cpp
    statisticsprovider.h
)

kcmutils_add_qml_kcm(kcm_energyinfo SOURCES ${kcm_energyinfo_SRCS} INSTALL_NAMESPACE "lingmo/kcms")

target_link_libraries(kcm_energyinfo PRIVATE
  Qt::DBus
  Qt::Widgets
  KF6::KIOWidgets
  KF6::CoreAddons
  KF6::KCMUtils
  KF6::I18n
  KF6::Solid
  KF6::KCMUtilsQuick
)
