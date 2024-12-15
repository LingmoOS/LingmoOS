configure_file(
  ocean-fakewm.service.in
  ocean-fakewm.service
  ONLY
)

set(SYSTEMD_SERVICE_FILES
  ${CMAKE_CURRENT_BINARY_DIR}/ocean-fakewm.service
)

install(FILES ${SYSTEMD_SERVICE_FILES} DESTINATION lib/systemd/user/)