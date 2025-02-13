/*
    SPDX-FileCopyrightText: 2007-2008 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kdeplatformplugin.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QtPlugin>

#include <KAboutData>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KMessageBox>
#include <KNotification>
#include <KSharedConfig>

#include "debug.h"
#include "kiomediastream.h"

namespace Phonon
{
KdePlatformPlugin::KdePlatformPlugin()
{
}

KdePlatformPlugin::~KdePlatformPlugin()
{
}

AbstractMediaStream *KdePlatformPlugin::createMediaStream(const QUrl &url, QObject *parent)
{
    return new KioMediaStream(url, parent);
}

QIcon KdePlatformPlugin::icon(const QString &name) const
{
    return QIcon::fromTheme(name);
}

void KdePlatformPlugin::notification(const char *notificationName,
                                     const QString &text,
                                     const QStringList &actions,
                                     QObject *receiver,
                                     const char *actionSlot) const
{
    KNotification *notification = new KNotification(notificationName);
    notification->setComponentName(QLatin1String("phonon"));
    notification->setText(text);
    notification->addContext(QLatin1String("Application"), KAboutData::applicationData().componentName());
    if (!actions.isEmpty() && receiver && actionSlot) {
        notification->setActions(actions);
        QObject::connect(notification, SIGNAL(activated(unsigned int)), receiver, actionSlot);
    }
    notification->sendEvent();
}

QString KdePlatformPlugin::applicationName() const
{
    KAboutData aboutData = KAboutData::applicationData();
    if (!aboutData.displayName().isEmpty()) {
        return aboutData.displayName();
    }
    if (!aboutData.componentName().isEmpty()) {
        return aboutData.componentName();
    }
    // FIXME: why was this not localized?
    return QLatin1String("Qt Application");
}

// Phonon4Qt5 internally implements backend lookup an creation. Driving it
// through KService is not practical because Phonon4Qt5 lacks appropriate
// wiring to frameworks.

QObject *KdePlatformPlugin::createBackend()
{
    return nullptr;
}

QObject *KdePlatformPlugin::createBackend(const QString & /*library*/, const QString & /*version*/)
{
    return nullptr;
}

bool KdePlatformPlugin::isMimeTypeAvailable(const QString & /*mimeType*/) const
{
    // Static mimetype based support reporting is utter nonsense, so always say
    // everything is supported.
    // In particular there's two problems
    // 1. mimetypes do not map well to actual formats because the majority of
    //    files these days are containers that can contain arbitrary content
    //    streams, so mimetypes are too generic to properly define supportedness.
    // 2. just about every multimedia library in the world draws format support
    //    from a plugin based architecture which means that technically everything
    //    can support anything as long as there is a plugin and/or the means to
    //    install a plugin.
    // So, always say every mimetype is supported.
    // Phonon5 will do away with all mentionings of mimetypes as well.
    return true;
}

// Volume restoration is a capability that will also be removed in Phonon5.
// For proper restoration capabilities the actual platform will be used (e.g.
// PulseAudio on Linux will remember streams and correctly restore the volume).

void KdePlatformPlugin::saveVolume(const QString &outputName, qreal volume)
{
    KConfigGroup config(KSharedConfig::openConfig(), "Phonon::AudioOutput");
    config.writeEntry(outputName + "_Volume", volume);
}

qreal KdePlatformPlugin::loadVolume(const QString &outputName) const
{
    KConfigGroup config(KSharedConfig::openConfig(), "Phonon::AudioOutput");
    return config.readEntry<qreal>(outputName + "_Volume", 1.0);
}

QList<int> KdePlatformPlugin::objectDescriptionIndexes(ObjectDescriptionType type) const
{
    switch (type) {
    case AudioOutputDeviceType:
    case AudioCaptureDeviceType:
    case VideoCaptureDeviceType:
    default:
        return QList<int>();
    }
}

QHash<QByteArray, QVariant> KdePlatformPlugin::objectDescriptionProperties(ObjectDescriptionType type, int index) const
{
    Q_UNUSED(index);
    switch (type) {
    case AudioOutputDeviceType:
    case AudioCaptureDeviceType:
    case VideoCaptureDeviceType:
    default:
        return QHash<QByteArray, QVariant>();
    }
}

} // namespace Phonon
