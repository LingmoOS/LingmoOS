// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "display.h"
#include "displaymanager.h"

#include <QDBusObjectPath>
#include <QJsonDocument>

using namespace dde::display;

Display1::Display1(QObject *parent)
    :QObject(parent)
    ,m_manager(new DisplayManager(this))
{
    registerScreenRectMetaType();
    registerBrightnessMapMetaType();
    registerTouchscreenInfoListMetaType();
    registerTouchscreenInfoList_V2MetaType();
    registerTouchscreenMapMetaType();
}

void Display1::init()
{

}

void Display1::ApplyChanges()
{

}

bool Display1::CanRotate()
{
    return false;
}

bool Display1::GetRealDisplayMode()
{
    return false;
}

QStringList Display1::ListOutputNames()
{
    QStringList list;
    if (!m_manager) {
        return list;
    }

    for (auto monitor : m_manager->monitors()) {
        list << monitor->name();
    }

    return list;
}

ResolutionList Display1::ListOutputsCommonModes()
{
    ResolutionList list;
    if (!m_manager) {
        return list;
    }

    for (auto monitor : m_manager->monitors()) {
        list.append(Resolution{monitor->id(), monitor->size().width(),
                               monitor->size().height(),
                               monitor->currentMode()->refreshRate()});
    }

    return list;
}

void Display1::Reset()
{

}

void Display1::ResetChanges()
{

}

void Display1::Save()
{

}

void Display1::SetPrimary(const QString &name)
{

}

void Display1::SwitchMode(const uchar &mode, const QString &name)
{

}

void Display1::AssociateTouch(const QString &in0, const QString &in1)
{

}

void Display1::AssociateTouchByUUID(const QString &in0, const QString &in1)
{

}

void Display1::ChangeBrightness(bool in0)
{

}

void Display1::DeleteCustomMode(const QString &in0)
{

}

void Display1::ModifyConfigName(const QString &in0, const QString &in1)
{

}

void Display1::RefreshBrightness()
{

}

void Display1::SetAndSaveBrightness(const QString &in0, double in1)
{

}

void Display1::SetBrightness(const QString &in0, double in1)
{

}

void Display1::SetColorTemperature(int in0)
{

}

void Display1::SetMethodAdjustCCT(int in0)
{

}

QString Display1::primary() const
{
    QString primary;

    if (!m_manager) {
       return primary;
    }

    for (auto monitor : m_manager->monitors()) {
        if (monitor->isEnabled() && monitor->isPrimary()) {
            primary = monitor->name();
        }
    }

    return primary;
}

quint16 Display1::screenHeight() const
{
    quint16 height;
    if (!m_manager) {
        return height;
    }

    for (auto monitor : m_manager->monitors()) {
        if (monitor->isEnabled() && monitor->isPrimary()) {
            height = monitor->size().height();
        }
    }

    return height;
}

quint16 Display1::screenWidth() const
{
    quint16 width;
    if (!m_manager) {
        return width;
    }

    for (auto monitor : m_manager->monitors()) {
        if (monitor->isEnabled() && monitor->isPrimary()) {
            width = monitor->size().width();
        }
    }

    return width;
}

ScreenRect Display1::primaryRect() const
{
    ScreenRect rect;

    if (!m_manager) {
        return rect;
    }

    for (auto monitor : m_manager->monitors()) {
       rect = ScreenRect{quint16(monitor->pos().x()), quint16(monitor->pos().y()), quint16(monitor->size().width()), quint16(monitor->size().height())};
    }

    return rect;
}

BrightnessMap Display1::brightness() const
{
    BrightnessMap brightness;

    if (!m_manager) {
        return brightness;
    }

    for (auto monitor : m_manager->monitors()) {
       brightness[monitor->name()] = 1.0;
    }

    return brightness;
}

QList<QDBusObjectPath> Display1::monitors() const
{
    QList<QDBusObjectPath> monitors;

    if (!m_manager) {
        return monitors;
    }

    for (auto monitor : m_manager->monitors().keys()) {
        monitors.append(QDBusObjectPath(monitor));
    }

    return monitors;
}

