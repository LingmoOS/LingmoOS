set (KDED_BALOOSEARCH_SRCS
     baloosearchmodule.cpp
)

add_library(baloosearchmodule MODULE ${KDED_BALOOSEARCH_SRCS})

target_link_libraries(baloosearchmodule
    KF6::CoreAddons
    KF6::DBusAddons
    KF6::KIOCore
)

install(TARGETS baloosearchmodule DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kded)
