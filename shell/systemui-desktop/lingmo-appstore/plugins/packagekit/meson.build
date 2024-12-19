cargs = ['-DG_LOG_DOMAIN="GsPluginPackageKit"']
cargs += ['-DLOCALPLUGINDIR="' + meson.current_build_dir() + '"']
deps = [
  plugin_libs,
  packagekit,
]

if get_option('mogwai')
  deps += [mogwai_schedule_client]
endif

shared_module(
  'gs_plugin_packagekit',
  sources : [
    'gs-plugin-packagekit.c',
    'gs-packagekit-helper.c',
    'gs-packagekit-task.c',
    'packagekit-common.c',
    'gs-markdown.c',
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

if get_option('tests')
  cargs += ['-DTESTDATADIR="' + join_paths(meson.current_source_dir(), 'tests') + '"']
  e = executable(
    'gs-self-test-packagekit',
    compiled_schemas,
    sources : [
      'gs-markdown.c',
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
  test('gs-self-test-packagekit', e, suite: ['plugins', 'packagekit'], env: test_env)
endif
