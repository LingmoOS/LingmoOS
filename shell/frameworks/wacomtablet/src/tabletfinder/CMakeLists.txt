
set( tabletfinder_SRCS
     main.cpp
     dialog.cpp
     hwbuttondialog.cpp
     dialog.h
     hwbuttondialog.h
)

ki18n_wrap_ui( tabletfinder_SRCS
                   dialog.ui
                   hwbuttondialog.ui
)

add_executable(kde_wacom_tabletfinder ${tabletfinder_SRCS})

target_link_libraries( kde_wacom_tabletfinder
                       wacom_common
)

install(TARGETS kde_wacom_tabletfinder DESTINATION ${KDE_INSTALL_BINDIR} )
install( FILES kde_wacom_tabletfinder.desktop  DESTINATION  ${KDE_INSTALL_APPDIR} )
