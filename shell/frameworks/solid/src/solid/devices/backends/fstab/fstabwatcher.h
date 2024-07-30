/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef SOLID_BACKENDS_FSTAB_WATCHER_H
#define SOLID_BACKENDS_FSTAB_WATCHER_H

#include <QObject>

class QFileSystemWatcher;
class QFile;
class QSocketNotifier;

namespace Solid
{
namespace Backends
{
namespace Fstab
{
class FstabWatcher : public QObject
{
    Q_OBJECT
public:
    FstabWatcher();
    ~FstabWatcher() override;

    static FstabWatcher *instance();

Q_SIGNALS:
    void mtabChanged();
    void fstabChanged();

private Q_SLOTS:
    void onFileChanged(const QString &path);
    void orphanFileSystemWatcher();

private:
    bool m_isRoutineInstalled;
    QFileSystemWatcher *m_fileSystemWatcher;
    QSocketNotifier *m_mtabSocketNotifier;
    QFile *m_mtabFile;
    bool m_isFstabWatched;
};
}
}
}
#endif // SOLID_BACKENDS_FSTAB_WATCHER_H
