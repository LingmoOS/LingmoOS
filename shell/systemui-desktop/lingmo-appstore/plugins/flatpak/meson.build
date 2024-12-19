cargs = ['-DG_LOG_DOMAIN="GsPluginFlatpak"']
deps = [
  plugin_libs,
  flatpak,
  libxmlb,
  ostree,
]

if get_option('mogwai')
  deps += mogwai_schedule_client
endif

shared_module(
  'gs_plugin_flatpak',
  sources : [
    'gs-flatpak-app.c',
    'gs-flatpak.c',
    'gs-flatpak-transaction.c',
    'gs-flatpak-utils.c',
    'gs-plugin-flatpak.c'
  ],
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : deps,
)
metainfo = 'org.lingmo.Appstore.Plugin.Flatpak.metainfo.xml'

i18n.merge_file(
  input: metainfo + '.in',
  output: metainfo,
  type: 'xml',
  po_dir: join_paths(meson.project_source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('datadir'), 'metainfo')
)

if get_option('tests')
  subdir('tests')

  cargs += ['-DLOCALPLUGINDIR="' + meson.current_build_dir() + '"']
  cargs += ['-DLOCALPLUGINDIR_CORE="' + meson.current_build_dir() + '/../core"']
  cargs += ['-DTESTDATADIR="' + join_paths(meson.current_build_dir(), 'tests') + '"']
  e = executable(
    'gs-self-test-flatpak',
    compiled_schemas,
    sources : [
      'gs-flatpak-app.c',
      'gs-self-test.c'
    ],
    include_directories : [
      include_directories('../..'),
      include_directories('../../lib'),
    ],
    dependencies : deps,
    c_args : cargs,
  )
  test('gs-self-test-flatpak', e, suite: ['plugins', 'flatpak'], env: test_env, timeout : 120)
endif
