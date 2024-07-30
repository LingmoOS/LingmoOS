include(ECMMarkNonGuiExecutable)

# We need to add a '6' so that kde3/kde4/kf5 apps running kbuildsycoca don't run this one.
add_executable(kbuildsycoca6)

# Mark it as non-gui so we won't create an app bundle on Mac OS X
ecm_mark_nongui_executable(kbuildsycoca6)

target_sources(kbuildsycoca6 PRIVATE
   kbuildsycoca_main.cpp
)

target_link_libraries(kbuildsycoca6
   KF6::Service
   KF6::CoreAddons # KAboutData
   KF6::I18n
   Qt6::Xml
)
install(TARGETS kbuildsycoca6 ${KF_INSTALL_TARGETS_DEFAULT_ARGS} )
