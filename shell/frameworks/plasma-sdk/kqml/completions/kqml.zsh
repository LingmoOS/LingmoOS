#compdef kqml

# SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>
#
# SPDX-License-Identifier: GPL-2.0-or-later

_arguments \
  '(-d --domain)'{-d,--domain=}"[The main localization domain]:application's main domain:" \
  '(-)'{-h,--help}'[Displays this help]' \
  '(-)'{-v,--version}'[Displays version information]' \
  '*:files:_files -g "*.qml"'
