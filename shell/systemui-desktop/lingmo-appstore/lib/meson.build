cargs = ['-DG_LOG_DOMAIN="Gs"']
cargs += ['-DLOCALPLUGINDIR=""']

liblingmosoftware_public_headers = [
  'lingmo-appstore.h',
  'gs-app.h',
  'gs-app-collation.h',
  'gs-app-list.h',
  'gs-app-permissions.h',
  'gs-app-query.h',
  'gs-appstream.h',
  'gs-category.h',
  'gs-category-manager.h',
  'gs-desktop-data.h',
  'gs-download-utils.h',
  'gs-external-appstream-utils.h',
  'gs-icon.h',
  'gs-icon-downloader.h',
  'gs-ioprio.h',
  'gs-job-manager.h',
  'gs-key-colors.h',
  'gs-metered.h',
  'gs-odrs-provider.h',
  'gs-os-release.h',
  'gs-plugin.h',
  'gs-plugin-event.h',
  'gs-plugin-helpers.h',
  'gs-plugin-job.h',
  'gs-plugin-job-list-apps.h',
  'gs-plugin-job-list-categories.h',
  'gs-plugin-job-list-distro-upgrades.h',
  'gs-plugin-job-manage-repository.h',
  'gs-plugin-job-refine.h',
  'gs-plugin-job-refresh-metadata.h',
  'gs-plugin-job-update-apps.h',
  'gs-plugin-loader.h',
  'gs-plugin-loader-sync.h',
  'gs-plugin-types.h',
  'gs-plugin-vfuncs.h',
  'gs-remote-icon.h',
  'gs-rewrite-resources.h',
  'gs-test.h',
  'gs-utils.h',
  'gs-worker-thread.h',
]

install_headers(liblingmosoftware_public_headers,
  subdir : 'lingmo-appstore'
)

librarydeps = [
  appstream,
  gio_unix,
  glib,
  gmodule,
  gtk,
  json_glib,
  libm,
  libsoup,
  libsysprof_capture_dep,
  libxmlb,
]

if get_option('mogwai')
  librarydeps += mogwai_schedule_client
endif

if get_option('polkit')
  librarydeps += polkit
endif

gs_build_ident_h = vcs_tag(
     fallback: meson.project_version(),
        input: 'gs-build-ident.h.in',
       output: 'gs-build-ident.h',
)

liblingmosoftware_enums = lingmo.mkenums_simple('gs-enums',
  sources : liblingmosoftware_public_headers,
  install_header : true,
  install_dir : join_paths(get_option('includedir'), 'lingmo-appstore'),
)

liblingmosoftware_include_directories = [
  include_directories('..'),
]

liblingmosoftware = library(
  'lingmosoftware',
  sources : [
    'gs-app.c',
    'gs-app-list.c',
    'gs-app-permissions.c',
    'gs-app-query.c',
    'gs-appstream.c',
    'gs-category.c',
    'gs-category-manager.c',
    'gs-debug.c',
    'gs-desktop-data.c',
    'gs-download-utils.c',
    'gs-external-appstream-utils.c',
    'gs-fedora-third-party.c',
    'gs-icon.c',
    'gs-icon-downloader.c',
    'gs-ioprio.c',
    'gs-ioprio.h',
    'gs-job-manager.c',
    'gs-key-colors.c',
    'gs-metered.c',
    'gs-odrs-provider.c',
    'gs-os-release.c',
    'gs-plugin.c',
    'gs-plugin-event.c',
    'gs-plugin-helpers.c',
    'gs-plugin-job.c',
    'gs-plugin-job-list-apps.c',
    'gs-plugin-job-list-categories.c',
    'gs-plugin-job-list-distro-upgrades.c',
    'gs-plugin-job-manage-repository.c',
    'gs-plugin-job-refine.c',
    'gs-plugin-job-refresh-metadata.c',
    'gs-plugin-job-update-apps.c',
    'gs-plugin-loader.c',
    'gs-plugin-loader-sync.c',
    'gs-profiler.h',
    'gs-remote-icon.c',
    'gs-rewrite-resources.c',
    'gs-test.c',
    'gs-utils.c',
    'gs-worker-thread.c',
  ] + liblingmosoftware_enums + [gs_build_ident_h],
  soversion: gs_plugin_api_version,
  include_directories : liblingmosoftware_include_directories,
  dependencies : librarydeps,
  c_args : cargs,
  install: true,
  install_dir: gs_private_libdir,
)

liblingmosoftware_dep = declare_dependency(link_with : liblingmosoftware,
  sources : liblingmosoftware_enums,
  include_directories : liblingmosoftware_include_directories,
  dependencies: librarydeps,
)

pkg = import('pkgconfig')

pkg.generate(
    liblingmosoftware,
    description : 'LINGMO Appstore is a software center for LINGMO',
    filebase : 'lingmo-appstore',
    name : 'lingmo-appstore',
    subdirs : 'lingmo-appstore',
    variables : [
      'gs_private_libdir=${libdir}/lingmo-appstore',
      'plugindir=${gs_private_libdir}/plugins-' + gs_plugin_api_version,
      'soupapiversion=' + libsoupapiversion,
    ],
    install_dir : join_paths(get_option('prefix'), get_option('libdir'), 'pkgconfig'),  # or it defaults to gs_private_libdir, which is wrong
)

executable(
  'lingmo-appstore-cmd',
  sources : [
    'gs-cmd.c',
  ],
  include_directories : [
    include_directories('..'),
  ],
  dependencies : [
    appstream,
    gio_unix,
    glib,
    gmodule,
    gtk,
    json_glib,
    liblingmosoftware_dep,
    libm,
    libsoup,
  ],
  c_args : cargs,
  install : true,
  install_dir : get_option('libexecdir'),
  install_rpath : gs_private_libdir,
)

if get_option('tests')
  cargs += ['-DTESTDATADIR="' + join_paths(meson.current_source_dir(), '..', 'data') + '"']
  e = executable(
    'gs-self-test',
    compiled_schemas,
    sources : [
      'gs-self-test.c'
    ],
    include_directories : [
      include_directories('..'),
    ],
    dependencies : [
      appstream,
      gio_unix,
      glib,
      gmodule,
      gtk,
      json_glib,
      liblingmosoftware_dep,
      libm,
      libsoup
    ],
    c_args : cargs
  )
  test('gs-self-test-lib', e, suite: ['lib'], env: test_env, timeout : 120)
endif

subdir('tools')
