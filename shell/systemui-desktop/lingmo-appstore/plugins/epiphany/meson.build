cargs = ['-DG_LOG_DOMAIN="GsPluginEpiphany"']

# The WebAppProvider interface xml comes from:
# https://github.com/LingmoOS/epiphany/-/blob/master/src/webapp-provider/org.lingmo.Epiphany.WebAppProvider.xml
epiphany_generated = lingmo.gdbus_codegen(
  'gs-epiphany-generated',
  sources : ['org.lingmo.Epiphany.WebAppProvider.xml'],
  interface_prefix : 'org.lingmo.Epiphany',
  namespace : 'GsEphy',
  extra_args : [ '--glib-min-required=' + glib.version() ],
)

shared_module(
  'gs_plugin_epiphany',
  epiphany_generated,
  sources : 'gs-plugin-epiphany.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
  link_with : [
    liblingmosoftware,
  ],
)
metainfo = 'org.lingmo.Appstore.Plugin.Epiphany.metainfo.xml'

i18n.merge_file(
  input: metainfo + '.in',
  output: metainfo,
  type: 'xml',
  po_dir: join_paths(meson.project_source_root(), 'po'),
  install: true,
  install_dir: join_paths(get_option('datadir'), 'metainfo'),
)

if get_option('tests')
  # The DynamicLauncher interface xml comes from:
  # https://github.com/flatpak/xdg-desktop-portal/blob/main/data/org.freedesktop.portal.DynamicLauncher.xml
  gdbus_codegen = find_program('gdbus-codegen')
  dynamic_launcher_portal_iface_h = custom_target(
    'gs-dynamic-launcher-portal-iface.h',
    input: ['org.freedesktop.portal.DynamicLauncher.xml'],
    output: ['gs-dynamic-launcher-portal-iface.h'],
    command: [gdbus_codegen,
              '--interface-info-header',
              '--output', '@OUTPUT@',
              '@INPUT@'],
  )
  dynamic_launcher_portal_iface_c = custom_target(
    'gs-dynamic-launcher-portal-iface.c',
    input: ['org.freedesktop.portal.DynamicLauncher.xml'],
    output: ['gs-dynamic-launcher-portal-iface.c'],
    command: [gdbus_codegen,
              '--interface-info-body',
              '--output', '@OUTPUT@',
              '@INPUT@'],
  )
  cargs += ['-DLOCALPLUGINDIR="' + meson.current_build_dir() + '"']
  cargs += ['-DLOCALPLUGINDIR_CORE="' + meson.current_build_dir() + '/../core"']
  cargs += ['-DTESTDATADIR="' + join_paths(meson.current_source_dir(), '..', '..', 'data') + '"']
  e = executable(
    'gs-self-test-epiphany',
    compiled_schemas,
    sources : [
      'gs-self-test.c',
      dynamic_launcher_portal_iface_c,
      dynamic_launcher_portal_iface_h,
      epiphany_generated,
    ],
    include_directories : [
      include_directories('../..'),
      include_directories('../../lib'),
    ],
    dependencies : [
      plugin_libs,
      dependency('glib-testing-0', fallback: ['libglib-testing', 'libglib_testing_dep']),
    ],
    link_with : [
      liblingmosoftware,
    ],
    c_args : cargs,
  )
  test('gs-self-test-epiphany', e, suite: ['plugins', 'epiphany'], env: test_env)
endif
