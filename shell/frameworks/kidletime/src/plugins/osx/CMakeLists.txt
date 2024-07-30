add_library(KF6IdleTimeOsxPlugin MODULE)

target_sources(KF6IdleTimeOsxPlugin PRIVATE
    macpoller.cpp
)

target_link_libraries(KF6IdleTimeOsxPlugin
    KF6IdleTime
    "-framework CoreFoundation -framework Carbon"
)

install(
    TARGETS
        KF6IdleTimeOsxPlugin
    DESTINATION
        ${KDE_INSTALL_PLUGINDIR}/kf6/kidletime/
)
