// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "kblayoutapplet.h"

#include "pluginfactory.h"

#include <QDBusConnection>
#include <QDBusPendingCall>
#include <QDBusReply>
#include <QLoggingCategory>

#include <DDBusSender>

namespace osd {

Q_LOGGING_CATEGORY(osdKBLog, "dde.shell.osd.kblayout")

static DDBusSender keyboardInter()
{
    return DDBusSender().service("org.deepin.dde.InputDevices1")
        .path("/org/deepin/dde/InputDevice1/Keyboard")
        .interface("org.deepin.dde.InputDevice1.Keyboard");
}

KBLayout::KBLayout(const QString &key, const QString &text, QObject *parent)
    : QObject(parent)
    , m_key(key)
    , m_text(text)
{

}

QString KBLayout::text() const
{
    return m_text;
}

QString KBLayout::key() const
{
    return m_key;
}

QString KBLayoutApplet::currentLayout() const
{
    return m_currentLayout;
}

void KBLayoutApplet::setCurrentLayout(const QString &newCurrentLayout)
{
    if (m_currentLayout == newCurrentLayout)
        return;
    m_currentLayout = newCurrentLayout;
    emit currentLayoutChanged();
}

void KBLayoutApplet::fetchLayouts()
{
    auto inter = keyboardInter();
    QDBusReply<QVariant> userLayoutList = inter.property("UserLayoutList").get();
    if (!userLayoutList.isValid()) {
        qCWarning(osdKBLog) << "Failed to fetch UserLayoutList" << userLayoutList.error();
        return;
    }
    auto userLayouts = qdbus_cast<QStringList>(userLayoutList.value());
    userLayouts.removeDuplicates();

    QDBusReply<QDBusArgument> layoutList(inter.method("LayoutList").call());

    if (!layoutList.isValid()) {
        qCWarning(osdKBLog) << "Failed to fetch LayoutList" << layoutList.error();
        return;
    }
    const auto layouts = qdbus_cast<QMap<QString, QString>>(layoutList.value());

    qDeleteAll(m_layouts);
    m_layouts.clear();

    for (auto item : userLayouts) {
        const auto text = layouts.value(item, item);
        auto layoutItem = new KBLayout(item, text, this);
        m_layouts.append(layoutItem);
    }
    Q_EMIT layoutsChanged();
}

void KBLayoutApplet::fetchCurrentLayout()
{
    QDBusReply<QVariant> currentLayoutList = keyboardInter().property("CurrentLayout").get();
    if (!currentLayoutList.isValid()) {
        qCWarning(osdKBLog) << "Failed to fetch CurrentLayout" << currentLayoutList.error();
        return;
    }
    setCurrentLayout(qdbus_cast<QString>(currentLayoutList));
}

KBLayoutApplet::KBLayoutApplet(QObject *parent)
    : DApplet(parent)
{
}

QQmlListProperty<KBLayout> KBLayoutApplet::layouts()
{
    return QQmlListProperty<KBLayout>(this, &m_layouts);
}

void KBLayoutApplet::sync()
{
    fetchLayouts();
    fetchCurrentLayout();
}

void KBLayoutApplet::next()
{
    auto it = std::find_if(m_layouts.begin(), m_layouts.end(), [this](const KBLayout *item) {
        return m_currentLayout == item->key();
    });
    if (it != m_layouts.end()) {
        auto offset = it - m_layouts.begin();
        auto nextIndex = (offset + 1) % m_layouts.count();
        const auto item = m_layouts.at(nextIndex);
        Q_ASSERT(item);
        setCurrentLayout(item->key());

        QDBusReply<void> reply = keyboardInter().property("CurrentLayout").set(m_currentLayout);
        if (!reply.isValid()) {
            qCWarning(osdKBLog) << "Failed to call CurrentLayout" << reply.error();
            return;
        }
        qCInfo(osdKBLog) << "next keyboard layout" << m_currentLayout;
    }
}

D_APPLET_CLASS(KBLayoutApplet)

}

#include "kblayoutapplet.moc"
