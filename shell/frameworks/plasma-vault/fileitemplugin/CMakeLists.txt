kcoreaddons_add_plugin(plasmavaultfileitemaction
    SOURCES
        plasmavaultfileitemaction.cpp
        plasmavaultfileitemaction.h
    INSTALL_NAMESPACE "kf6/kfileitemaction"
)

target_link_libraries(plasmavaultfileitemaction
    KF6::I18n
    KF6::KIOWidgets
)
