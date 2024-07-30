
add_subdirectory(systemmonitor)

# Systemmonitor presets
lingmo_install_package(coreusage org.kde.lingmo.systemmonitor.cpucore)
lingmo_install_package(cpu org.kde.lingmo.systemmonitor.cpu)
lingmo_install_package(memory org.kde.lingmo.systemmonitor.memory)
lingmo_install_package(diskusage org.kde.lingmo.systemmonitor.diskusage)
lingmo_install_package(diskactivity org.kde.lingmo.systemmonitor.diskactivity)
lingmo_install_package(net org.kde.lingmo.systemmonitor.net)
