// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2020-2021 Harald Sitter <sitter@kde.org>

#include "smartnotifier.h"

#include <KIO/ApplicationLauncherJob>
#include <KLocalizedString>
#include <KNotification>
#include <KService>

#include <QDebug>

#include "device.h"
#include "smartmonitor.h"

class FailureNotification : public QObject
{
    Q_OBJECT
public:
    FailureNotification(const Device *device, QObject *parent = nullptr)
        : QObject(parent)
    {
        m_notification->setComponentName(QStringLiteral("org.kde.kded.smart"));
        if (device->failed()) {
            m_notification->setIconName(QStringLiteral("data-warning"));
        } else {
            m_notification->setIconName(QStringLiteral("data-information"));
        }
        m_notification->setTitle(i18nc("@title notification", "Storage Device Problems"));
        if (device->failed()) {
            m_notification->setText(xi18nc("@info notification; text %1 is a pretty product name; %2 the device path e.g. /dev/sda",
                                           "The storage device <emphasis>%1</emphasis> (<filename>%2</filename>) is likely to fail soon!",
                                           device->product(),
                                           device->path()));
        } else {
            m_notification->setText(xi18nc("@info notification; text %1 is a pretty product name; %2 the device path e.g. /dev/sda",
                                           "The storage device <emphasis>%1</emphasis> (<filename>%2</filename>) is showing indications of instability.",
                                           device->product(),
                                           device->path()));
        }

        KService::Ptr kcm = KService::serviceByDesktopName(QStringLiteral("kcm_disks"));
        Q_ASSERT(kcm); // there's a bug or installation is broken; mustn't happen in production
        auto manageAction = m_notification->addAction(i18nc("@action:button notification action to manage device problems", "Manage"));
        connect(manageAction, &KNotificationAction::activated, this, [kcm] {
            KIO::ApplicationLauncherJob(kcm).start();
        });

        connect(m_notification, &KNotification::closed, this, [this] {
            deleteLater();
            m_notification = nullptr;
        });

        m_notification->sendEvent();
    }

    ~FailureNotification() override
    {
        if (m_notification) {
            m_notification->close();
        }
    }

private:
    KNotification *m_notification = new KNotification{QStringLiteral("imminentDeviceFailure"), KNotification::Persistent, nullptr};
};

SMARTNotifier::SMARTNotifier(SMARTMonitor *monitor, QObject *parent)
    : QObject(parent)
{
    connect(monitor, &SMARTMonitor::deviceAdded, this, [this](const Device *device) {
        connect(device, &Device::failedChanged, this, &SMARTNotifier::onMaybeFailed);
        // The device may already be in failure state. Make sure we display it if applicable.
        maybeFailed(device);
    });
    // upon removal the devices are deleted which takes care of disconnecting
}

void SMARTNotifier::onMaybeFailed()
{
    maybeFailed(qobject_cast<Device *>(sender()));
}

void SMARTNotifier::maybeFailed(const Device *device)
{
    Q_ASSERT(device);
    // NB: do not notify on instabilities in 5.22 it's shown to raise false positives or at least be annoying when
    //   failure seems unlikely. fixing it properly requires larger changes to the UI and strings
    // https://bugs.kde.org/show_bug.cgi?id=438539
    if ((!device->failed() /*&& device->instabilities().isEmpty()*/) || device->ignore()) {
        return;
    }

    new FailureNotification(device, this); // auto-deletes
    // once displayed we'll not want to trigger any more notifications
    device->disconnect(this);
}

#include "smartnotifier.moc"

#include "moc_smartnotifier.cpp"
