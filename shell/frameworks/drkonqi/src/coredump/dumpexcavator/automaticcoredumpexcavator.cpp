// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "automaticcoredumpexcavator.h"

#include <fcntl.h>
#include <unistd.h>

#include <chrono>
#include <filesystem>

#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusMessage>
#include <QDBusPendingCallWatcher>
#include <QDBusUnixFileDescriptor>
#include <QDebug>

#include "coredumpexcavator.h"

using namespace Qt::StringLiterals;
using namespace std::chrono_literals;

void AutomaticCoredumpExcavator::excavateFrom(const QString &coredumpFilename)
{
    if (!m_coreDir || !m_coreDir->isValid()) {
        m_coreDir = std::make_unique<QTemporaryDir>(QDir::tempPath() + u"/drkonqi-core"_s);
        Q_ASSERT(m_coreDir->isValid());
        if (!m_coreDir->isValid()) {
            Q_EMIT failed();
            return;
        }
        // Keep the core to ourself.
        std::filesystem::permissions(m_coreDir->path().toStdString(), std::filesystem::perms::owner_all, std::filesystem::perm_options::replace);
    }

    const auto coreFileTarget = m_coreDir->filePath(u"core"_s);
    const auto coredumpFileInfo = QFileInfo(coredumpFilename);

    auto coreFile = std::make_shared<QFile>(coreFileTarget);
    if (coreFile->exists()) {
        qDebug() << "Core already exists, returning early";
        Q_EMIT excavated(coreFileTarget);
        return;
    }
    if (!coreFile->open(QFile::WriteOnly, QFile::ReadUser | QFile::WriteUser)) {
        qWarning() << "Failed to open coreFileTarget" << coreFileTarget << coreFile->errorString();
        Q_EMIT failed();
        return;
    }

    if (coredumpFileInfo.isReadable()) {
        auto excavator = new CoredumpExcavator(this);
        connect(excavator, &CoredumpExcavator::excavated, this, [this, coreFileTarget](int exitCode) {
            if (exitCode != 0) {
                qWarning() << "Failed to excavate core from file:" << exitCode;
                Q_EMIT failed();
                return;
            }
            Q_EMIT excavated(coreFileTarget);
        });
        excavator->excavateFromTo(coredumpFilename, coreFile);
    } else {
        auto msg = QDBusMessage::createMethodCall("org.kde.drkonqi"_L1, "/"_L1, "org.kde.drkonqi"_L1, "saveCoreToFile"_L1);

        msg << coredumpFileInfo.fileName() << QVariant::fromValue(QDBusUnixFileDescriptor(coreFile->handle()));
        constexpr auto timeout = std::chrono::milliseconds(5min).count();
        auto watcher = new QDBusPendingCallWatcher(QDBusConnection::systemBus().asyncCall(msg, timeout));
        QObject::connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, watcher, coreFileTarget] {
            watcher->deleteLater();
            QDBusReply<void> reply = *watcher;
            qWarning() << reply.isValid() << reply.error();
            if (!reply.isValid()) {
                m_coreDir = nullptr;
                qWarning() << "Failed to excavate core as admin:" << reply.error();
                Q_EMIT failed();
                return;
            }
            Q_EMIT excavated(coreFileTarget);
        });
    }
}
