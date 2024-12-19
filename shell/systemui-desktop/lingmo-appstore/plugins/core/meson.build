cargs = ['-DG_LOG_DOMAIN="GsPlugin"']

shared_module(
  'gs_plugin_generic-updates',
  sources : 'gs-plugin-generic-updates.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)

shared_module(
  'gs_plugin_provenance',
  sources : 'gs-plugin-provenance.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)

shared_module(
  'gs_plugin_provenance-license',
  sources : 'gs-plugin-provenance-license.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)


shared_module(
  'gs_plugin_icons',
  sources : 'gs-plugin-icons.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)

shared_module(
  'gs_plugin_appstream',
  sources : [
    'gs-plugin-appstream.c'
  ],
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : [
    plugin_libs,
    libxmlb,
  ],
)

shared_module(
  'gs_plugin_hardcoded-blocklist',
  sources : 'gs-plugin-hardcoded-blocklist.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)

shared_module(
  'gs_plugin_os-release',
  sources : 'gs-plugin-os-release.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)

if get_option('tests')
  cargs += ['-DLOCALPLUGINDIR="' + meson.current_build_dir() + '"']
  cargs += ['-DTESTDATADIR="' + join_paths(meson.current_source_dir(), 'tests') + '"']
  e = executable(
    'gs-self-test-core',
    compiled_schemas,
    sources : [
      'gs-self-test.c',
    ],
    include_directories : [
      include_directories('../..'),
      include_directories('../../lib'),
    ],
    dependencies : [
      plugin_libs,
      libxmlb,
    ],
    c_args : cargs,
  )
  test('gs-self-test-core', e, suite: ['plugins', 'core'], env: test_env)
endif
