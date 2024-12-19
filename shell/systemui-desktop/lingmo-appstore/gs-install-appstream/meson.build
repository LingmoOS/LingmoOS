executable(
  'lingmo-appstore-install-appstream',
  sources : [
    'gs-install-appstream.c',
  ],
  dependencies : [
    gio_unix,
    liblingmosoftware_dep,
    libxmlb,
  ],
  include_directories : [
    include_directories('..'),
    include_directories('../lib'),
  ],
  c_args : ['-DG_LOG_DOMAIN="GsInstallAppstream"'],
  install : true,
  install_dir : get_option('libexecdir'),
  install_rpath : gs_private_libdir,
)
