/*
    Work sponsored by the LiMux project of the city of Munich:
    SPDX-FileCopyrightText: 2018 Kai Uwe Broulik <kde@broulik.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kscreenapplet.h"

#include <QMetaEnum>
#include <QQmlEngine> // for qmlRegisterType

#include <QDBusConnection>
#include <QDBusMessage>

#include <KScreen/Config>
#include <KScreen/ConfigMonitor>
#include <KScreen/GetConfigOperation>
#include <KScreen/Output>

#include <algorithm>

KScreenApplet::KScreenApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
}

KScreenApplet::~KScreenApplet() = default;

void KScreenApplet::init()
{
    qmlRegisterUncreatableType<KScreen::OsdAction>("org.kde.private.kscreen", 1, 0, "OsdAction", QStringLiteral("Can't create OsdAction"));
    connect(new KScreen::GetConfigOperation(KScreen::GetConfigOperation::NoEDID),
            &KScreen::ConfigOperation::finished,
            this,
            [this](KScreen::ConfigOperation *op) {
                m_screenConfiguration = qobject_cast<KScreen::GetConfigOperation *>(op)->config();

                KScreen::ConfigMonitor::instance()->addConfig(m_screenConfiguration);
                connect(KScreen::ConfigMonitor::instance(), &KScreen::ConfigMonitor::configurationChanged, this, &KScreenApplet::checkOutputs);

                checkOutputs();
            });
}

int KScreenApplet::connectedOutputCount() const
{
    return m_connectedOutputCount;
}

void KScreenApplet::applyLayoutPreset(Action action)
{
    const QMetaEnum actionEnum = QMetaEnum::fromType<KScreen::OsdAction::Action>();
    Q_ASSERT(actionEnum.isValid());

    const QString presetName = QString::fromLatin1(actionEnum.valueToKey(action));
    if (presetName.isEmpty()) {
        return;
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.kded5"),
                                                      QStringLiteral("/modules/kscreen"),
                                                      QStringLiteral("org.kde.KScreen"),
                                                      QStringLiteral("applyLayoutPreset"));

    msg.setArguments({presetName});

    QDBusConnection::sessionBus().call(msg, QDBus::NoBlock);
}

void KScreenApplet::checkOutputs()
{
    if (!m_screenConfiguration) {
        return;
    }

    const int oldConnectedOutputCount = m_connectedOutputCount;

    const auto outputs = m_screenConfiguration->outputs();
    m_connectedOutputCount = std::count_if(outputs.begin(), outputs.end(), [](const KScreen::OutputPtr &output) {
        return output->isConnected();
    });

    if (m_connectedOutputCount != oldConnectedOutputCount) {
        emit connectedOutputCountChanged();
    }
}

QVariant KScreenApplet::availableActions()
{
    // Need to wrap it in a QVariant, otherwise QML doesn't like the return type
    return QVariant::fromValue(KScreen::OsdAction::availableActions());
}

K_PLUGIN_CLASS_WITH_JSON(KScreenApplet, "metadata.json")

#include "kscreenapplet.moc"
