#!/bin/bash

# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Anupam Basak <anupam.basak27@gmail.com>

printf "\n\n### Locales added by Pico Wizard ###\n" >> /etc/locale.gen

for i in "$@"; do
  printf "%s\n" "$i"
done >> /etc/locale.gen