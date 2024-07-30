set(xf86gammacfg_SRCS xf86gammacfg.cpp)

add_executable(xf86gammacfg ${xf86gammacfg_SRCS})

install(TARGETS xf86gammacfg ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

