subdir('metainfo')
subdir('icons')
subdir('assets')

cdata = configuration_data()
cdata.set('bindir', join_paths(get_option('prefix'),
                               get_option('bindir')))

# note: the main .desktop file is in src/
# this one is for the background service
# replace @bindir@
configure_file(
  input : 'autostart/org.lingmo.Appstore.desktop.in',
  output : 'org.lingmo.Appstore.desktop',
  install_dir: join_paths(get_option('sysconfdir'), 'xdg/autostart'),
  configuration : cdata
)

compiled_schemas = lingmo.compile_schemas(
  depend_files: 'org.lingmo.appstore.gschema.xml')
install_data('org.lingmo.appstore.gschema.xml',
             install_dir : join_paths(get_option('datadir'), 'glib-2.0', 'schemas'))

if get_option('external_appstream')
  # replace @libexecdir@
  conf_data = configuration_data()
  conf_data.set('libexecdir', join_paths(get_option('prefix'),
                                         get_option('libexecdir')))
  out = configure_file(
    input : 'org.lingmo.appstore.external-appstream.policy.in.in',
    output : 'org.lingmo.appstore.external-appstream.policy.in',
    install: false,
    configuration : conf_data,
  )
  i18n.merge_file(
    input : out,
    output : 'org.lingmo.appstore.external-appstream.policy',
    po_dir: join_paths(meson.project_source_root(), 'po'),
    install: true,
    install_dir: join_paths(get_option('datadir'), 'polkit-1', 'actions'),
  )
endif

install_data('bash-completion/lingmo-appstore',
             install_dir : join_paths(get_option('datadir'), 'bash-completion', 'completions'))
