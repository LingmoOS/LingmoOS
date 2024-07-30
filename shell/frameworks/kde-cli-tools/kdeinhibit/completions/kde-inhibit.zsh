#compdef kde-inhibit

# SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: GPL-2.0-or-later

_arguments -s \
  '(- *)'{-h,--help}'[Displays help on commandline options]' \
  '--power[Inhibit power management]' \
  '--screenSaver[Inhibit screensaver]' \
  '--nightLight[Inhibit night light (blue light filter)]' \
  '--notifications[Inhibit notifications (Do not disturb)]' \
  ':program: _command_names -e' \
  '*::program arguments: _normal'
