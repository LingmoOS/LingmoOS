cargs = ['-DG_LOG_DOMAIN="GsPluginFedoraLangpacks"']

shared_module(
  'gs_plugin_fedora-langpacks',
  sources : 'gs-plugin-fedora-langpacks.c',
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
  e = executable(
    'gs-self-test-fedora-langpacks',
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
    ],
    c_args : cargs,
  )
  test('gs-self-test-fedora-langpacks', e, suite: ['plugins', 'fedora-langpacks'], env: test_env)
endif
