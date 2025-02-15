add_executable(ksendbugmail)

target_sources(ksendbugmail PRIVATE
    main.cpp
    smtp.cpp
)

ecm_mark_nongui_executable(ksendbugmail)

target_link_libraries(ksendbugmail
   Qt${QT_MAJOR_VERSION}::Widgets
   Qt${QT_MAJOR_VERSION}::Network
   KF5::ConfigCore # KEmailSettings
   KF5::I18n
)
if(WIN32)
    target_link_libraries(ksendbugmail secur32) # GetUserNameEx()
endif()

install(TARGETS ksendbugmail DESTINATION ${KDE_INSTALL_LIBEXECDIR_KF} )

