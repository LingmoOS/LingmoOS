set(potd_%{APPNAMELC}_SRCS
    %{APPNAMELC}.cpp
)

add_library(lingmo_potd_%{APPNAMELC} MODULE ${potd_%{APPNAMELC}_SRCS})
target_link_libraries(lingmo_potd_%{APPNAMELC}
    Lingmo::PotdProvider
    KF6::KIOCore
)

install(TARGETS lingmo_potd_%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd)
