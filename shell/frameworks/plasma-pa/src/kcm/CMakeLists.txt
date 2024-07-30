kcmutils_add_qml_kcm(kcm_pulseaudio SOURCES module.cpp)
target_link_libraries(kcm_pulseaudio PRIVATE
    Qt::Quick
    KF6::CoreAddons
    KF6::KCMUtilsQuick
)

