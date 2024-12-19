cargs = ['-DG_LOG_DOMAIN="GsPluginRepos"']
cargs += ['-DLOCALPLUGINDIR="' + meson.current_build_dir() + '"']

shared_module(
  'gs_plugin_repos',
  sources : 'gs-plugin-repos.c',
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
  cargs += ['-DTESTDATADIR="' + join_paths(meson.current_source_dir(), 'tests') + '"']
  e = executable(
    'gs-self-test-repos',
    compiled_schemas,
    sources : [
      'gs-self-test.c'
    ],
    include_directories : [
      include_directories('../..'),
      include_directories('../../lib'),
    ],
    dependencies : [
      plugin_libs,
    ],
    c_args : cargs,
  )
  test('gs-self-test-repos', e, suite: ['plugins', 'repos'], env: test_env)
endif
