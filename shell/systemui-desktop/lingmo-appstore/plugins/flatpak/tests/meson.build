tar = find_program('tar')
custom_target(
  'flatpak-self-test-data',
  input : 'flatpakrepos.tar.gz',
  output : 'done',
  command : [
    tar,
    '--no-same-owner',
    '--directory=' + meson.current_build_dir(),
    '-xf', '@INPUT@',
  ],
  build_by_default : true,
)

custom_target(
  'flatpak-self-test-bundle',
  output : 'flatpakrepos.tar.gz',
  command : [
    tar,
    '-czf', '@OUTPUT@',
    'app-missing-runtime/repo/',
    'app-update/repo/',
    'app-with-runtime/repo/',
    'only-runtime/repo/',
    'app-extension/repo',
    'app-extension-update/repo',
  ],
)

configure_file(
  input : 'chiron.flatpak',
  output : 'chiron.flatpak',
  copy : true,
)
