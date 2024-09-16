// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "appearancehandler.h"
#include <QDebug>

#include <QApplication>
#include <QScrollArea>
#include <QScreen>

WIDGETS_FRAME_BEGIN_NAMESPACE
static const QString DBusServie = "org.deepin.dde.Appearance1";
static const QString DBusPath = "/org/deepin/dde/Appearance1";

Appearancehandler::Appearancehandler(QObject *parent)
    : QObject (parent)
{
    m_appearance = new Appearance(DBusServie, DBusPath, QDBusConnection::sessionBus(), this);
}

Appearancehandler::~Appearancehandler()
{
    m_targets.clear();
    if (m_appearance) {
        m_appearance->deleteLater();
        m_appearance = nullptr;
    }
}

void Appearancehandler::addTargetWidget(DBlurEffectWidget *target)
{
    target->setMaskAlpha(alpha());
    connect(m_appearance, &Appearance::OpacityChanged, target, [this, target](double  value) {
        target->setMaskAlpha(static_cast<quint8>(value * 255));
    });
    m_targets.append(target);
}

int Appearancehandler::alpha() const
{
    const double value = m_appearance->opacity();
    return static_cast<quint8>(value * 255);
}
WIDGETS_FRAME_END_NAMESPACE
