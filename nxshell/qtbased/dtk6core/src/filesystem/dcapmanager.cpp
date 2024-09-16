// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dcapmanager.h"
#include "dobject_p.h"
#include "dstandardpaths.h"
#include "private/dcapfsfileengine_p.h"

#include <QStandardPaths>
#include <QDebug>

DCORE_BEGIN_NAMESPACE

QString _d_cleanPath(const QString &path) {
    return path.size() < 2 || !path.endsWith(QDir::separator()) ? path : path.chopped(1);
}

bool _d_isSubFileOf(const QString &filePath, const QString &directoryPath)
{
    QString path = _d_cleanPath(filePath);
    bool ret = path.startsWith(directoryPath);
    return ret;
}

static QStringList defaultWriteablePaths() {
    QStringList paths;
    int list[] = {QStandardPaths::AppConfigLocation,
                  QStandardPaths::AppDataLocation,
                  QStandardPaths::CacheLocation,
                  QStandardPaths::TempLocation,
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
                  QStandardPaths::DataLocation,
#endif
                  QStandardPaths::GenericConfigLocation,
                  QStandardPaths::HomeLocation,
                  QStandardPaths::MusicLocation,
                  QStandardPaths::DocumentsLocation,
                  QStandardPaths::MoviesLocation,
                  QStandardPaths::PicturesLocation,
                  QStandardPaths::DownloadLocation};

    for (uint i = 0; i < sizeof (list) / sizeof (int); ++i) {
        const QString &path  = QStandardPaths::writableLocation(QStandardPaths::StandardLocation(list[i]));
        if (path.isEmpty())
            continue;

        paths.append(path);
    }

    for (int i = 0; i <= static_cast<int>(DStandardPaths::XDG::RuntimeDir); ++i) {
        const QString &path = DStandardPaths::path(DStandardPaths::XDG(i));
        if (path.isEmpty())
            continue;

        paths.append(path);
    }

    for (int i = 0; i <= static_cast<int>(DStandardPaths::DSG::DataDir); ++i) {
        const QStringList &pathList = DStandardPaths::paths(DStandardPaths::DSG(i));
        if (pathList.isEmpty())
            continue;

        for (auto path : pathList) {
            if (path.isEmpty() || paths.contains(path))
                continue;

            paths.append(path);
        }
    }
    return paths;
}

class DCapManagerPrivate : public DObjectPrivate
{
    D_DECLARE_PUBLIC(DCapManager)
public:
    DCapManagerPrivate(DCapManager *qq);

    QStringList pathList;
};

class DCapManager_ : public DCapManager {};
Q_GLOBAL_STATIC(DCapManager_, capManager)

DCapManagerPrivate::DCapManagerPrivate(DCapManager *qq)
    : DObjectPrivate(qq)
{
    pathList = defaultWriteablePaths();
}

DCapManager::DCapManager()
    : DObject(*new DCapManagerPrivate(this))
{

}

DCapManager *DCapManager::instance()
{
    return capManager;
}

#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
void DCapManager::registerFileEngine()
{
}

void DCapManager::unregisterFileEngine()
{
}
#endif

void DCapManager::appendPath(const QString &path)
{
    D_D(DCapManager);
    const QString &targetPath = _d_cleanPath(path);
    bool exist = std::any_of(d->pathList.cbegin(), d->pathList.cend(),
                             std::bind(_d_isSubFileOf, targetPath, std::placeholders::_1));
    if (exist)
        return;
    d->pathList.append(targetPath);
}

void DCapManager::appendPaths(const QStringList &pathList)
{
    for (auto path : pathList)
        appendPath(path);
}

void DCapManager::removePath(const QString &path)
{
    D_D(DCapManager);
    const QString &targetPath = _d_cleanPath(path);
    if (!d->pathList.contains(targetPath))
        return;
    d->pathList.removeOne(targetPath);
}

void DCapManager::removePaths(const QStringList &paths)
{
    for (auto path : paths)
        removePath(path);
}

QStringList DCapManager::paths() const
{
    D_DC(DCapManager);
    return d->pathList;
}

DCORE_END_NAMESPACE
