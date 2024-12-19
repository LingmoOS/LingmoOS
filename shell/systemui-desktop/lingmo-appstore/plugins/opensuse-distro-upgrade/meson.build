cargs = ['-DG_LOG_DOMAIN="GsPluginOpensuseDistroUpgrade"']

shared_module(
  'gs_plugin_opensuse-distro-upgrade',
  sources : 'gs-plugin-opensuse-distro-upgrade.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib')
  ],
  install : true,
  install_dir : plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)
