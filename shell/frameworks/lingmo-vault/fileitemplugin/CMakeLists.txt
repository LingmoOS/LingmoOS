kcoreaddons_add_plugin(lingmovaultfileitemaction
    SOURCES
        lingmovaultfileitemaction.cpp
        lingmovaultfileitemaction.h
    INSTALL_NAMESPACE "kf6/kfileitemaction"
)

target_link_libraries(lingmovaultfileitemaction
    KF6::I18n
    KF6::KIOWidgets
)
