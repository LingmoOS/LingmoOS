/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef EFFECTWATCHER_P_H
#define EFFECTWATCHER_P_H

#include <QGuiApplication>
#include <QObject>

#include <QAbstractNativeEventFilter>

#include <xcb/xcb.h>

namespace Lingmo
{
class EffectWatcher : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit EffectWatcher(const QString &property, QObject *parent = nullptr);

protected:
    bool isEffectActive() const;
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *) override;

Q_SIGNALS:
    void effectChanged(bool on);

private:
    void init(const QString &property);
    xcb_atom_t m_property;
    bool m_effectActive;
    QNativeInterface::QX11Application *m_x11Interface = nullptr;
};

} // namespace Lingmo

#endif
