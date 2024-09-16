// Copyright (C) 2021 ~ 2021 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#define VERSION_STRING_FULL "0.5.4"
#define DATADIR "/usr/share"
#define LOCALEDIR "/usr/share/locale"
#define EXEC_PREFIX "/usr/bin"
#define PACKAGE "fcitx"
#define XLIBDIR "/usr/lib/X11"
#define FCITX4_EXEC_PREFIX "usr"
#define FCITX4_MAJOR_VERSION 4
#define FCITX4_MINOR_VERSION 2
#define FCITX4_PATCH_VERSION 9

#define FCITX_CHECK_VERSION(major, minor, micro) \
    (FCITX4_MAJOR_VERSION > (major) || (FCITX4_MAJOR_VERSION == (major) && FCITX4_MINOR_VERSION > (minor)) || (FCITX4_MAJOR_VERSION == (major) && FCITX4_MINOR_VERSION == (minor) && FCITX4_PATCH_VERSION >= (micro)))
