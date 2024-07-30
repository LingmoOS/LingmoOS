/*
    SPDX-FileCopyrightText: 2010 Mario Bensi <mbensi@ipsquad.net>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "fstabwatcher.h"
#include "fstab_debug.h"
#include "soliddefs_p.h"

#include <QCoreApplication>
#include <QFile>
#include <QFileSystemWatcher>
#include <QSocketNotifier>

namespace Solid
{
namespace Backends
{
namespace Fstab
{
Q_GLOBAL_STATIC(FstabWatcher, globalFstabWatcher)

static const QString s_mtabFile = QStringLiteral("/etc/mtab");
#ifdef Q_OS_SOLARIS
static const QString s_fstabFile = QStringLiteral("/etc/vfstab");
#else
static const QString s_fstabFile = QStringLiteral("/etc/fstab");
#endif
static const QString s_fstabPath = QStringLiteral("/etc");

FstabWatcher::FstabWatcher()
    : m_isRoutineInstalled(false)
    , m_fileSystemWatcher(new QFileSystemWatcher(this))
{
    if (qApp) {
        connect(qApp, &QCoreApplication::aboutToQuit, this, &FstabWatcher::orphanFileSystemWatcher);
    }

    m_mtabFile = new QFile(s_mtabFile, this);
    if (m_mtabFile && m_mtabFile->symLinkTarget().startsWith("/proc/") && m_mtabFile->open(QIODevice::ReadOnly)) {
        m_mtabSocketNotifier = new QSocketNotifier(m_mtabFile->handle(), QSocketNotifier::Exception, this);
        connect(m_mtabSocketNotifier, &QSocketNotifier::activated, this, &FstabWatcher::mtabChanged);
    } else {
        m_fileSystemWatcher->addPath(s_mtabFile);
    }

    m_fileSystemWatcher->addPath(s_fstabPath);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, [this](const QString &) {
        if (!m_isFstabWatched) {
            m_isFstabWatched = m_fileSystemWatcher->addPath(s_fstabFile);
            if (m_isFstabWatched) {
                qCDebug(FSTAB_LOG) << "Re-added" << s_fstabFile;
                Q_EMIT onFileChanged(s_fstabFile);
            }
        }
    });

    m_isFstabWatched = m_fileSystemWatcher->addPath(s_fstabFile);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &FstabWatcher::onFileChanged);
}

FstabWatcher::~FstabWatcher()
{
    // The QFileSystemWatcher doesn't work correctly in a singleton
    // The solution so far was to destroy the QFileSystemWatcher when the application quits
    // But we have some crash with this solution.
    // For the moment to workaround the problem, we detach the QFileSystemWatcher from the parent
    // effectively leaking it on purpose.

#if 0
    //qRemovePostRoutine(globalFstabWatcher.destroy);
#else
    m_fileSystemWatcher->setParent(nullptr);
#endif
}

void FstabWatcher::orphanFileSystemWatcher()
{
    m_fileSystemWatcher->setParent(nullptr);
}

FstabWatcher *FstabWatcher::instance()
{
#if 0
    FstabWatcher *fstabWatcher = globalFstabWatcher;

    if (fstabWatcher && !fstabWatcher->m_isRoutineInstalled) {
        qAddPostRoutine(globalFstabWatcher.destroy);
        fstabWatcher->m_isRoutineInstalled = true;
    }
    return fstabWatcher;
#else
    return globalFstabWatcher;
#endif
}

void FstabWatcher::onFileChanged(const QString &path)
{
    if (path == s_mtabFile) {
        Q_EMIT mtabChanged();
        if (!m_fileSystemWatcher->files().contains(s_mtabFile)) {
            m_fileSystemWatcher->addPath(s_mtabFile);
        }
    }
    if (path == s_fstabFile) {
        Q_EMIT fstabChanged();
        if (!m_fileSystemWatcher->files().contains(s_fstabFile)) {
            m_isFstabWatched = m_fileSystemWatcher->addPath(s_fstabFile);
            qCDebug(FSTAB_LOG) << "Fstab removed, re-added:" << m_isFstabWatched;
        }
    }
}

}
}
} // namespace Solid:Backends::Fstab

#include "moc_fstabwatcher.cpp"
