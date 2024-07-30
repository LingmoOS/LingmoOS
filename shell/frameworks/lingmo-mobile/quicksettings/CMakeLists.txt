# SPDX-FileCopyrightText: 2022 Devin Lin <devin@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later

lingmo_install_package(airplanemode org.kde.lingmo.quicksetting.airplanemode quicksettings)
lingmo_install_package(audio org.kde.lingmo.quicksetting.audio quicksettings)
lingmo_install_package(battery org.kde.lingmo.quicksetting.battery quicksettings)
lingmo_install_package(bluetooth org.kde.lingmo.quicksetting.bluetooth quicksettings)
lingmo_install_package(caffeine org.kde.lingmo.quicksetting.caffeine quicksettings)
lingmo_install_package(docked org.kde.lingmo.quicksettings.docked quicksettings)
lingmo_install_package(donotdisturb org.kde.lingmo.quicksetting.donotdisturb quicksettings)
lingmo_install_package(keyboardtoggle org.kde.lingmo.quicksetting.keyboardtoggle quicksettings)
lingmo_install_package(mobiledata org.kde.lingmo.quicksetting.mobiledata quicksettings)
lingmo_install_package(settingsapp org.kde.lingmo.quicksetting.settingsapp quicksettings)
lingmo_install_package(wifi org.kde.lingmo.quicksetting.wifi quicksettings)
lingmo_install_package(hotspot org.kde.lingmo.quicksetting.hotspot quicksettings)
add_subdirectory(flashlight)
add_subdirectory(nightcolor)
add_subdirectory(powermenu)
# TODO: Disable screen recording quick setting, as it does not work properly
add_subdirectory(record)
add_subdirectory(screenshot)
add_subdirectory(screenrotation)
