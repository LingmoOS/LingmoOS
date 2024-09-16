// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef UTILS_H
#define UTILS_H

#include <gio/gio.h>

#include <QString>
#include <QPair>

namespace Utils {
// daemon mounts mounted at "/media/$USER/smbmounts/hello on 1.2.3.4"
inline constexpr char kRegxDaemonMountPath[] { "^/media/.*/smbmounts/" };
inline constexpr char kDaemonService[] { "com.deepin.filemanager.daemon" };
inline constexpr char kDaemonPath[] { "/com/deepin/filemanager/daemon" };
inline constexpr char kDaemonIntro[] { "org.freedesktop.DBus.Introspectable" };
inline constexpr char kDaemonIntroMethod[] { "Introspect" };
inline constexpr char kMountControlPath[] { "/com/deepin/filemanager/daemon/MountControl" };
inline constexpr char kMountControlIFace[] { "com.deepin.filemanager.daemon.MountControl" };
inline constexpr char kMountControlMount[] { "Mount" };
inline constexpr char kMountControlUnmount[] { "Unmount" };

QString workingUser();

QString mountPointOf(GMount *mount);
QPair<QString, QString> mountInfoOf(GMount *mount);

}   // namespace Utils

#endif   // UTILS_H
