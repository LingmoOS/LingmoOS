set(oceandecoration_config_SRCS
    ../oceanexceptionlist.cpp
    oceanconfigwidget.cpp
    oceandetectwidget.cpp
    oceanexceptiondialog.cpp
    oceanexceptionlistwidget.cpp
    oceanexceptionmodel.cpp
    oceanitemmodel.cpp
)
ki18n_wrap_ui(oceandecoration_config_SRCS
   ui/oceanconfigurationui.ui
   ui/oceanexceptiondialog.ui
   ui/oceanexceptionlistwidget.ui
)

kcoreaddons_add_plugin(kcm_oceandecoration SOURCES kcm_oceandecoration.cpp ${oceandecoration_config_SRCS} INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}")
kconfig_add_kcfg_files(kcm_oceandecoration ../oceansettings.kcfgc)
target_include_directories(kcm_oceandecoration PRIVATE ${CMAKE_SOURCE_DIR}/kdecoration ${CMAKE_BINARY_DIR}/kdecoration/)
target_link_libraries(kcm_oceandecoration
    PUBLIC
        Qt6::Core
        Qt6::Gui
        Qt6::DBus
    PRIVATE
        KF6::ConfigCore
        KF6::CoreAddons
        KF6::GuiAddons
        KF6::I18n
        KF6::IconThemes
        KF6::KCMUtils
)
kcmutils_generate_desktop_file(kcm_oceandecoration)
