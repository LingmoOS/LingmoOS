cargs = ['-DG_LOG_DOMAIN="GsPluginRpmOstree"']

rpmostree_generated = lingmo.gdbus_codegen(
  'gs-rpmostree-generated',
  'org.projectatomic.rpmostree1.xml',
  interface_prefix : 'org.projectatomic.rpmostree1',
  namespace : 'GsRPMOSTree',
  extra_args : [ '--glib-min-required=' + glib.version() ],
)

shared_module(
  'gs_plugin_rpm-ostree',
  rpmostree_generated,
  sources : 'gs-plugin-rpm-ostree.c',
  include_directories : [
    include_directories('../..'),
    include_directories('../../lib'),
  ],
  install : true,
  install_dir: plugin_dir,
  install_rpath: join_paths(rpm_ostree.get_variable('libdir'), 'rpm-ostree'),
  c_args : cargs,
  dependencies : [ plugin_libs, ostree, rpm, rpm_ostree ],
)
