########### next target ###############

set( kcm_tablet_SRCS
     areaselectionwidget.cpp
     buttonactiondisplaywidget.cpp
     buttonactionselectiondialog.cpp
     buttonactionselectionwidget.cpp
     buttonactionselectorwidget.cpp
     buttonpagewidget.cpp
     calibrationdialog.cpp
     generalpagewidget.cpp
     kcmwacomtabletwidget.cpp
     keysequenceinputbutton.cpp
     keysequenceinputwidget.cpp
     pressurecurvewidget.cpp
     pressurecurvedialog.cpp
     styluspagewidget.cpp
     tabletareaselectioncontroller.cpp
     tabletareaselectiondialog.cpp
     tabletareaselectionview.cpp
     tabletareaselectionwidget.cpp
     tabletpagewidget.cpp
     touchpagewidget.cpp
     ../common/globalactions.cpp # FIXME: why?
     areaselectionwidget.h
     buttonactiondisplaywidget.h
     buttonactionselectiondialog.h
     buttonactionselectionwidget.h
     buttonactionselectorwidget.h
     buttonpagewidget.h
     calibrationdialog.h
     generalpagewidget.h
     kcmwacomtabletwidget.h
     keysequenceinputbutton.h
     keysequenceinputwidget.h
     pressurecurvewidget.h
     pressurecurvedialog.h
     styluspagewidget.h
     tabletareaselectioncontroller.h
     tabletareaselectiondialog.h
     tabletareaselectionview.h
     tabletareaselectionwidget.h
     tabletpagewidget.h
     touchpagewidget.h
)

ki18n_wrap_ui( kcm_tablet_SRCS
                   buttonactionselectionwidget.ui
                   buttonactionselectorwidget.ui
                   buttonpagewidget.ui
                   errorwidget.ui
                   generalpagewidget.ui
                   kcmwacomtabletwidget.ui
                   pressurecurvedialog.ui
                   styluspagewidget.ui
                   tabletareaselectionview.ui
                   tabletpagewidget.ui
                   touchpagewidget.ui
)

ecm_qt_declare_logging_category(
    kcm_tablet_SRCS
    HEADER "logging.h"
    DESCRIPTION "wacomtablet (kcm)"
    IDENTIFIER "KCM"
    CATEGORY_NAME "org.kde.wacomtablet.kcm"
    EXPORT WACOMTABLET
)

set(kcm_wacomtablet_PLUGIN_SRC
    kcmwacomtablet.cpp
    kcmwacomtablet.h
)

set(kcm_wacomtablet_LIBS
    wacom_common
    Qt::Core
    Qt::Widgets
    KF6::CoreAddons
    KF6::XmlGui
    KF6::WidgetsAddons
    KF6::WindowSystem
    ${X11_LIBRARIES}
    ${X11_Xinput_LIB}
    KF6::KCMUtils
    )

add_definitions(-DTRANSLATION_DOMAIN=\"wacomtablet\")
add_library(kcm_wacomtablet MODULE ${kcm_wacomtablet_PLUGIN_SRC} ${kcm_tablet_SRCS})
kcmutils_generate_desktop_file(kcm_wacomtablet)

target_link_libraries(kcm_wacomtablet ${kcm_wacomtablet_LIBS})

install(TARGETS kcm_wacomtablet DESTINATION ${KDE_INSTALL_PLUGINDIR}/plasma/kcms/systemsettings_qwidgets)
install(FILES org.kde.wacomtablet.metainfo.xml DESTINATION "${KDE_INSTALL_METAINFODIR}")

## LIBRARY FOR UNIT TESTS
if (BUILD_TESTING)
    ## build static library for unit testing
    add_library( kcm_testlib STATIC ${kcm_tablet_SRCS} )
    target_link_libraries( kcm_testlib ${kcm_wacomtablet_LIBS} )
    set_target_properties( kcm_testlib PROPERTIES COMPILE_FLAGS "-fPIC" )

endif (BUILD_TESTING)
