cargs = ['-DG_LOG_DOMAIN="GsPluginFedoraPkgdbCollections"']

shared_module(
  'gs_plugin_fedora-pkgdb-collections',
  sources : 'gs-plugin-fedora-pkgdb-collections.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  c_args : cargs,
  dependencies : plugin_libs,
)
