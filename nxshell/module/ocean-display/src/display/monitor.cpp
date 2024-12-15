// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "monitor.h"

Monitor::Monitor(const KScreen::OutputPtr &output, QObject *parent)
    :QObject(parent)
    ,m_monitor(output)
{
    registerResolutionMetaType();
    registerResolutionListMetaType();
    registerRotationListMetaType();
    registerReflectListMetaType();
}

QString Monitor::name() const
{
    return m_monitor->name();
}

quint32 Monitor::id() const
{
    return m_monitor->id();
}

short Monitor::x() const
{
    return m_monitor->pos().x();
}

short Monitor::y() const
{
    return m_monitor->pos().y();
}

void Monitor::Enable(bool in0)
{
    m_monitor->setEnabled(in0);
}

ushort Monitor::width() const
{
    return m_monitor->size().width();
}

ushort Monitor::height() const
{
    return m_monitor->size().height();
}

quint32 Monitor::mmHeight() const
{
    return m_monitor->sizeMm().height();
}

quint32 Monitor::mmWidth() const
{
    return m_monitor->sizeMm().width();
}

double Monitor::refreshRate() const
{
    return m_monitor->currentMode()->refreshRate();
}

Resolution Monitor::currentMode() const
{
    return Resolution{m_monitor->id(), m_monitor->size().width(), m_monitor->size().height(), m_monitor->currentMode()->refreshRate()};
}

void Monitor::SetMode(uint in0)
{

}

void Monitor::SetModeBySize(ushort in0, ushort in1)
{

}

void Monitor::SetPosition(short in0, short in1)
{
    QPoint pos(in0, in1);
    m_monitor->setPos(pos);
}

void Monitor::SetReflect(ushort in0)
{

}

void Monitor::SetRotation(ushort in0)
{

}
