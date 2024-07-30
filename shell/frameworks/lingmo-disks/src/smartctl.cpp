// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

#include "smartctl.h"

#include <KAuth/Action>
#include <KAuth/ExecuteJob>
#include <KAuth/HelperSupport>
#include <KLocalizedString>
#include <QDebug>
#include <QFileInfo>

#include "kded_debug.h"
#include "smartfailure.h"

void SMARTCtl::run(const QString &devicePath)
{
    // https://bugs.kde.org/show_bug.cgi?id=379215
    // One cannot make any kind of concurrent requests to kauth, so we need
    // a fairly awkward busy state and request queuing system.
    if (m_busy) {
        m_requestQueue.push(devicePath);
        return;
    }
    m_busy = true;

    KAuth::Action action(QStringLiteral("org.kde.kded.smart.smartctl"));
    // This is technically never used unless the sysadmin forces our action
    // to require authentication. In that case we'll want to give request context
    // as we do requests per-device though.
    action.setDetailsV2({{KAuth::Action::AuthDetail::DetailMessage,
                          i18nc("@label description of authentication request to read SMART data. %1 is a device path e.g. /dev/sda",
                                "Read SMART report for storage device %1",
                                devicePath)}});
    action.setHelperId(QStringLiteral("org.kde.kded.smart"));

    // The helper only consumes names, ensure we fully resolve the name of the
    // device to /dev/$name.
    const QString canonicalDevicePath = QFileInfo(devicePath).canonicalFilePath();
    Q_ASSERT(!canonicalDevicePath.isEmpty());
    const QFileInfo canonicalDeviceInfo(canonicalDevicePath);
    Q_ASSERT(canonicalDeviceInfo.absolutePath() == QLatin1String("/dev"));

    action.addArgument(QStringLiteral("deviceName"), canonicalDeviceInfo.fileName());
    qCDebug(KDED) << action.isValid() << action.hasHelper() << action.helperId() << action.status();
    KAuth::ExecuteJob *job = action.execute();
    connect(job, &KJob::result, this, [this, job, devicePath] {
        const auto data = job->data();
        const auto code = SMART::Failures(data.value(QStringLiteral("exitCode"), QByteArray()).toInt());
        const auto json = data.value(QStringLiteral("data"), QByteArray()).toByteArray();

        QJsonDocument document;
        if (json.isEmpty()) {
            qCDebug(KDED) << "looks like we got no data back for" << devicePath << code << json.isEmpty();
        } else {
            document = QJsonDocument::fromJson(json);
        }

        // Queue the next pending request if there is any.
        m_busy = false;
        if (!m_requestQueue.empty()) {
            auto request = m_requestQueue.front();
            run(request);
            m_requestQueue.pop();
        }

        Q_EMIT finished(devicePath, document, data.value(QStringLiteral("cliData")).toString());
    });
    job->start();
}

#include "moc_smartctl.cpp"
