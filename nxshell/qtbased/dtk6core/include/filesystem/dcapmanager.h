// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DCAPMANAGER_H
#define DCAPMANAGER_H

#include <DObject>

#include <QObject>

DCORE_BEGIN_NAMESPACE
class DCapManagerPrivate;
class DCapManager : public QObject, public DObject
{
    Q_OBJECT
    Q_DISABLE_COPY(DCapManager)
    D_DECLARE_PRIVATE(DCapManager)
public:
    static DCapManager *instance();
#if DTK_VERSION < DTK_VERSION_CHECK(6, 0, 0, 0)
    QT_DEPRECATED_X("This api will no longer take effect, please use DCapDir or DCapFile")
    static void registerFileEngine();
    QT_DEPRECATED_X("This api will no longer take effect, please use DCapDir or DCapFile")
    static void unregisterFileEngine();
#endif
    void appendPath(const QString &path);
    void appendPaths(const QStringList &pathList);

    void removePath(const QString &path);
    void removePaths(const QStringList &paths);

    QStringList paths() const;

protected:
    explicit DCapManager();
};

DCORE_END_NAMESPACE
#endif // DCAPMANAGER_H
