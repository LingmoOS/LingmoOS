add_executable(kstart kstart.cpp)
target_compile_definitions(kstart PRIVATE -DPROJECT_VERSION="${PROJECT_VERSION}")
target_link_libraries(kstart
    Qt::Gui
    KF6::I18n
    KF6::KIOGui
    KF6::Service
    KF6::WindowSystem)

if(WITH_X11)
	target_link_libraries(kstart Qt::GuiPrivate)
endif()

install_compat_symlink(kstart)
install(TARGETS kstart DESTINATION ${KDE_INSTALL_FULL_BINDIR})
