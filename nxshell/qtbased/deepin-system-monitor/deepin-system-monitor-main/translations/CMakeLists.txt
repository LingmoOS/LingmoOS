file(GLOB APP_TS_FILES LIST_DIRECTORIES false *.ts)
qt5_add_translation(APP_QM_FILES ${APP_TS_FILES})
add_custom_target(QMFILES ALL DEPENDS ${APP_QM_FILES})
install(FILES ${APP_QM_FILES} DESTINATION share/${CMAKE_PROJECT_NAME}/translations)

add_subdirectory(desktop)
add_subdirectory(policy)
