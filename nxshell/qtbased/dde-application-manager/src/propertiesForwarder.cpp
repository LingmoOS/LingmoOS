// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "propertiesForwarder.h"
#include "global.h"
#include <QMetaProperty>
#include <QDBusMessage>
#include <QDBusAbstractAdaptor>

PropertiesForwarder::PropertiesForwarder(QString path, QObject *parent)
    : QObject(parent)
    , m_path(std::move(path))
{
    const auto *mo = parent->metaObject();

    if (mo == nullptr) {
        qCritical() << "relay propertiesChanged failed.";
        return;
    }

    for (auto i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        auto prop = mo->property(i);
        if (!prop.hasNotifySignal()) {
            continue;
        }

        auto signal = prop.notifySignal();
        auto slot = metaObject()->method(metaObject()->indexOfSlot("PropertyChanged()"));
        QObject::connect(parent, signal, this, slot);
    }
}

void PropertiesForwarder::PropertyChanged()
{
    auto *sender = QObject::sender();
    auto sigIndex = QObject::senderSignalIndex();

    const auto *mo = sender->metaObject();
    if (mo == nullptr) {
        qCritical() << "PropertiesForwarder::PropertyChanged [relay propertiesChanged failed.]";
        return;
    }

    auto sig = mo->method(sigIndex);
    auto signature = sig.methodSignature();

    QByteArray propName;
    for (auto i = mo->propertyOffset(); i < mo->propertyCount(); ++i) {
        auto prop = mo->property(i);
        if (!prop.hasNotifySignal()) {
            continue;
        }

        if (prop.notifySignal().methodSignature() == signature) {
            propName = prop.name();
        }
    }

    if (propName.isEmpty()) {
        qDebug() << "can't find corresponding property:" << signature;
        return;
    }

    auto propIndex = mo->indexOfProperty(propName.constData());
    auto prop = mo->property(propIndex);
    auto value = prop.read(sender);

    auto msg = QDBusMessage::createSignal(m_path, "org.freedesktop.DBus.Properties", "PropertiesChanged");

    msg << QString{ApplicationInterface};
    msg << QVariantMap{{QString{propName}, value}};
    msg << QStringList{};

    ApplicationManager1DBus::instance().globalServerBus().send(msg);
}
