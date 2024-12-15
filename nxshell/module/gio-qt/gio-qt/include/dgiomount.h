// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DGIOMOUNT_H
#define DGIOMOUNT_H

#include <QObject>
#include <QSharedData>

namespace Gio {
class Mount;
}

class DGioFile;
class DGioVolume;
class DGioMountPrivate;
class DGioMount : public QObject, public QSharedData
{
    Q_OBJECT
public:
    explicit DGioMount(Gio::Mount *gmmMountPtr, QObject *parent = nullptr);
    ~DGioMount();

    static DGioMount * createFromPath(QString path, QObject *parent = nullptr);

    QString name() const;
    QString uuid() const;
    QString mountClass() const;
    bool isShadowed() const;
    bool canUnmount() const;
    bool canEject() const;
    QString sortKey() const;
    QStringList themedIconNames() const;
    QStringList themedSymbolicIconNames() const;

    void unmount(bool forceUnmount = false);
    void eject(bool forceEject = false);

    QExplicitlySharedDataPointer<DGioFile> getRootFile();
    QExplicitlySharedDataPointer<DGioFile> getDefaultLocationFile();
    QExplicitlySharedDataPointer<DGioVolume> getVolume();

private:
    QScopedPointer<DGioMountPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DGioMount)
};

#endif // DGIOMOUNT_H
