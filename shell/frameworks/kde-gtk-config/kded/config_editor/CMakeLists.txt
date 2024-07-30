# SPDX-FileCopyrightText: 2022 Mikhail Zolotukhin <zomial@protonmail.com>
# SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

target_sources(gtkconfig 
  PRIVATE 
    custom_css.cpp 
    gsettings.cpp 
    gtk2.cpp
    settings_ini.cpp 
    utils.cpp
    xsettings.cpp
)
