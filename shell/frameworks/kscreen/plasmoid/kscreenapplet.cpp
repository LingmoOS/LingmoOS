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
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>

#include <algorithm>

KScreenApplet::KScreenApplet(QObject *parent, const KPluginMetaData &data, const QVariantList &args)
    : Plasma::Applet(parent, data, args)
{
    qmlRegisterUncreatableMetaObject(KScreen::OsdAction::staticMetaObject,
                                     "org.kde.private.kscreen",
                                     1,
                                     0,
                                     "OsdAction",
                                     QStringLiteral("Can't create OsdAction"));
}

KScreenApplet::~KScreenApplet() = default;

void KScreenApplet::init()
{
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

void KScreenApplet::applyLayoutPreset(KScreen::OsdAction::Action action)
{
    KScreen::OsdAction::applyAction(m_screenConfiguration, action);
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
    auto actions = KScreen::OsdAction::availableActions();
    QList<KScreen::OsdAction> ret;
    ret.reserve(actions.size() - 1);
    for (const auto &action : actions) {
        if (action.action != KScreen::OsdAction::NoAction) {
            ret.append(action);
        }
    }
    // Need to wrap it in a QVariant, otherwise QML doesn't like the return type
    return QVariant::fromValue(ret);
}

K_PLUGIN_CLASS(KScreenApplet)

#include "kscreenapplet.moc"

#include "moc_kscreenapplet.cpp"
