set(kquickcontrolsprivate_SRCS
    keysequencehelper.cpp
    keysequencehelper.h
    keysequencevalidator.cpp
    keysequencevalidator.h
    kquickcontrolsprivateplugin.cpp
    kquickcontrolsprivateplugin.h
    translationcontext.cpp
    translationcontext.h
)

add_library(kquickcontrolsprivateplugin ${kquickcontrolsprivate_SRCS})

target_link_libraries(kquickcontrolsprivateplugin
    Qt6::Core
    Qt6::Quick
    Qt6::Qml
    KF6::I18n
    KF6::ConfigGui
    KF6::GuiAddons
)

target_include_directories(kquickcontrolsprivateplugin PRIVATE ${CMAKE_BINARY_DIR})

if (HAVE_KGLOBALACCEL)
    target_link_libraries(kquickcontrolsprivateplugin KF6::GlobalAccel)
endif()

if (NOT ANDROID)
    target_link_libraries(kquickcontrolsprivateplugin KF6::WidgetsAddons)
endif()

if(WIN32 AND BUILD_SHARED_LIBS)
    set_target_properties(kquickcontrolsprivateplugin PROPERTIES PREFIX "")
endif()

install(TARGETS kquickcontrolsprivateplugin DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/kquickcontrols)

install(FILES qmldir DESTINATION ${KDE_INSTALL_QMLDIR}/org/kde/private/kquickcontrols)
