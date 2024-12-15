// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "windoweffectapplet.h"

#include "pluginfactory.h"

const QString ORG_KDE_KWIN = QStringLiteral("org.kde.kwin");
const QString ORG_KDE_KWIN_COMPOSITING = QStringLiteral("org.kde.kwin.compositing");
const QString WINDOW_EFFECT_TYPE_KEY = QStringLiteral("user_type");

namespace osd {

WindowEffectApplet::WindowEffectApplet(QObject *parent)
    : DApplet(parent)
    , m_wmConfig(DConfig::create(ORG_KDE_KWIN, ORG_KDE_KWIN_COMPOSITING, "", this))
{
    connect(m_wmConfig, &DConfig::valueChanged, this, &WindowEffectApplet::onWmConfigChanged);
    onWmConfigChanged(WINDOW_EFFECT_TYPE_KEY);
}

void WindowEffectApplet::setEffectType(WindowEffectType effectType)
{
    m_wmConfig->setValue(WINDOW_EFFECT_TYPE_KEY, effectType);
}

WindowEffectApplet::WindowEffectType WindowEffectApplet::effectType() const
{
    return m_effectType;
}

void WindowEffectApplet::onWmConfigChanged(const QString &key)
{
    if (key == WINDOW_EFFECT_TYPE_KEY) {
        auto value = m_wmConfig->value(WINDOW_EFFECT_TYPE_KEY).toInt();
        m_effectType = static_cast<WindowEffectType>(value);
        Q_EMIT effectTypeChanged(m_effectType);
    }
}

D_APPLET_CLASS(WindowEffectApplet)

}

#include "windoweffectapplet.moc"
