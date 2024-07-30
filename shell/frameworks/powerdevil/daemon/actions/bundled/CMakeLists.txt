function(add_powerdevil_bundled_action _name)
    # Build the plugin
    kcoreaddons_add_plugin(powerdevil_${_name}action SOURCES ${_name}.cpp INSTALL_NAMESPACE "powerdevil/action")
    target_link_libraries(powerdevil_${_name}action
                        powerdevil_log_static
                        powerdevilcore
                        KF6::CoreAddons
                        KF6::I18n
                        ${ARGN})
endfunction(add_powerdevil_bundled_action _name)

function(target_dbus_files _target)
    set(dbus_SRCS)
    qt_add_dbus_adaptor(dbus_SRCS ${ARGN})
    target_sources(${_target} PRIVATE ${dbus_SRCS})
endfunction()

add_powerdevil_bundled_action(brightnesscontrol KF6::GlobalAccel)
target_dbus_files(powerdevil_brightnesscontrolaction  org.kde.Solid.PowerManagement.Actions.BrightnessControl.xml
                     brightnesscontrol.h PowerDevil::BundledActions::BrightnessControl)

add_powerdevil_bundled_action(keyboardbrightnesscontrol)
target_dbus_files(powerdevil_keyboardbrightnesscontrolaction org.kde.Solid.PowerManagement.Actions.KeyboardBrightnessControl.xml
                     keyboardbrightnesscontrol.h PowerDevil::BundledActions::KeyboardBrightnessControl)

add_powerdevil_bundled_action(dimdisplay)
add_powerdevil_bundled_action(runscript KF6::KIOCore KF6::KIOWidgets)

add_powerdevil_bundled_action(suspendsession KF6::KIOCore KF6::KIOWidgets)
target_dbus_files(powerdevil_suspendsessionaction org.kde.Solid.PowerManagement.Actions.SuspendSession.xml
                     suspendsession.h PowerDevil::BundledActions::SuspendSession)

add_powerdevil_bundled_action(dpms)
target_link_libraries(powerdevil_dpmsaction KF6::ScreenDpms)

set(handlebuttons_SRCS handlebuttonevents.cpp)
qt_add_dbus_adaptor(handlebuttons_SRCS org.kde.Solid.PowerManagement.Actions.HandleButtonEvents.xml handlebuttonevents.h PowerDevil::BundledActions::HandleButtonEvents)

add_powerdevil_bundled_action(powerprofile)
target_dbus_files(powerdevil_powerprofileaction org.kde.Solid.PowerManagement.Actions.PowerProfile.xml powerprofile.h PowerDevil::BundledActions::PowerProfile)
set(powerprofile_dbus_SRCS)
qt_add_dbus_interface(powerprofile_dbus_SRCS org.freedesktop.DBus.Properties.xml properties_interface)
qt_add_dbus_interface(powerprofile_dbus_SRCS net.hadess.PowerProfiles.xml power_profiles_interface)
target_sources(powerdevil_powerprofileaction PRIVATE ${powerprofile_dbus_SRCS} ${handlebuttons_SRCS})

kcoreaddons_add_plugin(powerdevil_handlebuttoneventsaction SOURCES powerdevilhandlebuttonevents_plugin.cpp ${handlebuttons_SRCS} INSTALL_NAMESPACE "powerdevil/action")
target_link_libraries(powerdevil_handlebuttoneventsaction PRIVATE powerdevilcore)
