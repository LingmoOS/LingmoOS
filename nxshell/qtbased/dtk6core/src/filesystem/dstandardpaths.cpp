// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dstandardpaths.h"

#include <QProcessEnvironment>
#include <unistd.h>
#include <pwd.h>

DCORE_BEGIN_NAMESPACE

class DSnapStandardPathsPrivate
{
public:
    inline  static QString writableLocation(QStandardPaths::StandardLocation /*type*/)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        return env.value("SNAP_USER_COMMON");
    }

    inline static QStringList standardLocations(QStandardPaths::StandardLocation type)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();

        switch (type) {
        case QStandardPaths::GenericDataLocation: {
            QString snapRoot = env.value("SNAP");
            QString genericDataDir = snapRoot + PREFIX"/share/";
            return QStringList() << genericDataDir;
        }
        default:
            break;
        }

        return QStringList() << env.value("SNAP_USER_COMMON");
    }

private:
    DSnapStandardPathsPrivate();
    ~DSnapStandardPathsPrivate();
    Q_DISABLE_COPY(DSnapStandardPathsPrivate)
};


/*!
  \class Dtk::Core::DStandardPaths
  \inmodule dtkcore
  \brief DStandardPaths提供兼容Snap/Dtk标准的路径模式。DStandardPaths实现了Qt的QStandardPaths主要接口.
  \sa QStandardPaths
 */

/*!
  \enum Dtk::Core::DStandardPaths::Mode
  \brief DStandardPaths支持的路径产生模式。
  \value Auto
  \brief 和Qt标准的行为表现一致。
  \value Snap
  \brief 读取SNAP相关的环境变量，支持将配置存储在SNAP对应目录。
  \value Test
  \brief 和Qt标准的行为表现一致，但是会开启测试模式，参考QStandardPaths::setTestModeEnabled。
 */


static DStandardPaths::Mode s_mode = DStandardPaths::Auto;

QString DStandardPaths::writableLocation(QStandardPaths::StandardLocation type)
{
    switch (s_mode) {
    case Auto:
    case Test:
        return  QStandardPaths::writableLocation(type);
    case Snap:
        return DSnapStandardPathsPrivate::writableLocation(type);
    }
    return QStandardPaths::writableLocation(type);
}

QStringList DStandardPaths::standardLocations(QStandardPaths::StandardLocation type)
{
    switch (s_mode) {
    case Auto:
    case Test:
        return  QStandardPaths::standardLocations(type);
    case Snap:
        return DSnapStandardPathsPrivate::standardLocations(type);
    }
    return  QStandardPaths::standardLocations(type);
}

QString DStandardPaths::locate(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options)
{
    return QStandardPaths::locate(type, fileName, options);
}

QStringList DStandardPaths::locateAll(QStandardPaths::StandardLocation type, const QString &fileName, QStandardPaths::LocateOptions options)
{
    return QStandardPaths::locateAll(type, fileName, options);
}

QString DStandardPaths::findExecutable(const QString &executableName, const QStringList &paths)
{
    return QStandardPaths::findExecutable(executableName, paths);
}

void DStandardPaths::setMode(DStandardPaths::Mode mode)
{
    s_mode = mode;
    QStandardPaths::setTestModeEnabled(mode == Test);
}

// https://gitlabwh.uniontech.com/wuhan/se/deepin-specifications/-/issues/21

QString DStandardPaths::homePath()
{
    const QByteArray &home = qgetenv("HOME");

    if (!home.isEmpty())
        return QString::fromLocal8Bit(home);

    return homePath(getuid());
}

QString DStandardPaths::path(DStandardPaths::XDG type)
{
    switch (type) {
    case XDG::DataHome: {
        const QByteArray &path = qgetenv("XDG_DATA_HOME");
        if (!path.isEmpty())
            return QString::fromLocal8Bit(path);
        return homePath() + QStringLiteral("/.local/share");
    }
    case XDG::CacheHome: {
        const QByteArray &path = qgetenv("XDG_CACHE_HOME");
        if (!path.isEmpty())
            return QString::fromLocal8Bit(path);
        return homePath() + QStringLiteral("/.cache");
    }
    case XDG::ConfigHome: {
        const QByteArray &path = qgetenv("XDG_CONFIG_HOME");
        if (!path.isEmpty())
            return QString::fromLocal8Bit(path);
        return homePath() + QStringLiteral("/.config");
    }
    case XDG::RuntimeDir: {
        const QByteArray &path = qgetenv("XDG_RUNTIME_DIR");
        if (!path.isEmpty())
            return QString::fromLocal8Bit(path);
        return QStringLiteral("/run/user/") + QString::number(getuid());
    }
    case XDG::StateHome: {
        const QByteArray &path = qgetenv("XDG_STATE_HOME");
        if (!path.isEmpty())
            return QString::fromLocal8Bit(path);
#ifdef Q_OS_LINUX
        return homePath() + QStringLiteral("/.local/state");
#else
        // TODO: handle it on mac
        return QString();
#endif
    }
    }
    return QString();
}

QString DStandardPaths::path(DStandardPaths::DSG type)
{
    const auto list = paths(type);
    return list.isEmpty() ? nullptr : list.first();
}

QStringList DStandardPaths::paths(DSG type)
{
    QStringList paths;

    if (type == DSG::DataDir) {
        const QByteArray &path = qgetenv("DSG_DATA_DIRS");
        if (path.isEmpty()) {
            return {QLatin1String(PREFIX"/share/dsg")};
        }
        const auto list = path.split(':');
        paths.reserve(list.size());
        for (const auto &i : list)
            paths.push_back(QString::fromLocal8Bit(i));
    } else if (type == DSG::AppData) {
        const QByteArray &path = qgetenv("DSG_APP_DATA");
        //TODO 应用数据目录规范:`/persistent/appdata/{appid}`, now `appid` is not captured.
        paths.push_back(QString::fromLocal8Bit(path));
    }

    return paths;
}

QString DStandardPaths::filePath(DStandardPaths::XDG type, QString fileName)
{
    const QString &dir = path(type);

    if (dir.isEmpty())
        return QString();

    return dir + QLatin1Char('/') + fileName;
}

QString DStandardPaths::filePath(DStandardPaths::DSG type, const QString fileName)
{
    const QString &dir = path(type);

    if (dir.isEmpty())
        return QString();

    return dir + QLatin1Char('/') + fileName;
}

QString DStandardPaths::homePath(const uint uid)
{
    struct passwd *pw = getpwuid(uid);

    if (!pw)
        return QString();

    const char *homedir = pw->pw_dir;
    return QString::fromLocal8Bit(homedir);
}

DCORE_END_NAMESPACE
