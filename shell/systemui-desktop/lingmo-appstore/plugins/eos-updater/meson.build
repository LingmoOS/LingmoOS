cargs = ['-DG_LOG_DOMAIN="GsPluginEosUpdater"']

eos_updater_generated = lingmo.gdbus_codegen(
  'gs-eos-updater-generated',
  sources : ['com.endlessm.Updater.xml'],
  interface_prefix : 'com.endlessm.',
  namespace : 'GsEos',
  extra_args : [ '--glib-min-required=' + glib.version() ],
)

shared_module(
  'gs_plugin_eos-updater',
  eos_updater_generated,
  sources : 'gs-plugin-eos-updater.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : [
    plugin_libs,
    ostree,
  ],
)
