plugin_dir = join_paths(gs_private_libdir, 'plugins-' + gs_plugin_api_version)
plugin_libs = [
  appstream,
  gio_unix,
  json_glib,
  libsoup,
  liblingmosoftware_dep,
]

subdir('core')
subdir('dpkg')
subdir('dummy')
subdir('fedora-langpacks')
subdir('fedora-pkgdb-collections')

if get_option('eos_updater')
  subdir('eos-updater')
endif
if get_option('flatpak')
  subdir('flatpak')
endif
if get_option('fwupd')
  subdir('fwupd')
endif
if get_option('gudev')
  subdir('modalias')
endif
if get_option('malcontent')
  subdir('malcontent')
endif
if get_option('opensuse-distro-upgrade')
  subdir('opensuse-distro-upgrade')
endif
if get_option('packagekit')
  subdir('packagekit')
endif
subdir('repos')
if get_option('rpm_ostree')
  subdir('rpm-ostree')
endif
if get_option('snap')
  subdir('snap')
endif
if get_option('webapps')
  subdir('epiphany')
endif
