add_definitions(-DTRANSLATION_DOMAIN=\"%{APPNAMELC}part\")

add_library(%{APPNAMELC}part MODULE)

target_sources(%{APPNAMELC}part PRIVATE
    %{APPNAMELC}part.cpp
    # handled by CMake's AUTORCC, as set by KDECMakeSettings
    %{APPNAMELC}part.qrc
)

target_link_libraries(%{APPNAMELC}part
    KF6::I18n
    KF6::Parts
)

install(TARGETS %{APPNAMELC}part  DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/parts)
