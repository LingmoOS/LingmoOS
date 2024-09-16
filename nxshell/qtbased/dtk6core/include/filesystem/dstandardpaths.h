// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H
#define DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H

#include <QStandardPaths>

#include "dtkcore_global.h"

DCORE_BEGIN_NAMESPACE

class DStandardPathsPrivate;
class LIBDTKCORESHARED_EXPORT DStandardPaths
{
public:
    enum Mode {
        Auto,
        Snap,
        Test,
    };

    static QString writableLocation(QStandardPaths::StandardLocation type);
    static QStringList standardLocations(QStandardPaths::StandardLocation type);

    static QString locate(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options = QStandardPaths::LocateFile);
    static QStringList locateAll(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options = QStandardPaths::LocateFile);
    static QString findExecutable(const QString &executableName, const QStringList &paths = QStringList());
    static void setMode(Mode mode);

    /**
     * @brief About XDG dir, view it in https://gitlab.freedesktop.org/xdg/xdg-specs/
     */
    enum class XDG {
        /*
         * @brief DataHome, usually is ~/.local/share, also can be defined by ${XDG_DATA_HOME}, where stores the data of applications
         */
        DataHome,
        /*
         * @brief ConfigHome, usually is ~/.config, can be defined by ${XDG_CONFIG_HOME}, where stores the config of applications
         */
        ConfigHome,
        /*
         * @brief CacheHome, usually is ~/.cache, can be defined by ${XDG_CACHE_HOME}, where stores caches, can be always cleared
         */
        CacheHome,
        /*
         * @brief Where temp files or sock files always be put in, like sddm.sock. It is unique per session. It is /run/user/${uid} or ${XDG_RUNTIME_DIR},
         */
        RuntimeDir,
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
        RuntimeTime [[deprecated("Use RuntimeDir Instead")]] = RuntimeDir,
#endif
        /*
         * @brief where history file and state file should be. It is induced because users do not want to mix their config files and state files. It is always ~/.local/state, or defined by ${XDG_STATE_HOME}
         */
        StateHome
    };

    enum class DSG {
        AppData,
        DataDir
    };

    static QString homePath();
    static QString homePath(const uint uid);
    /*
     * @brief Get the XDG dir path by XDG type
     */
    static QString path(XDG type);
    static QString path(DSG type);
    static QStringList paths(DSG type);
    static QString filePath(XDG type, QString fileName);
    static QString filePath(DSG type, QString fileName);

private:
    DStandardPaths();
    ~DStandardPaths();
    Q_DISABLE_COPY(DStandardPaths)
};

DCORE_END_NAMESPACE

#endif // DTK_CORE_FILESYSTEM_DSTANDARDPATHS_H
