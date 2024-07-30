set(potd_%{APPNAMELC}_SRCS
    %{APPNAMELC}.cpp
)

add_library(plasma_potd_%{APPNAMELC} MODULE ${potd_%{APPNAMELC}_SRCS})
target_link_libraries(plasma_potd_%{APPNAMELC}
    Plasma::PotdProvider
    KF6::KIOCore
)

install(TARGETS plasma_potd_%{APPNAMELC} DESTINATION ${KDE_INSTALL_PLUGINDIR}/potd)
