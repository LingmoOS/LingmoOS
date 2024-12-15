// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "applet.h"
#include <DConfig>

DCORE_USE_NAMESPACE

namespace osd {
class WindowEffectApplet : public DS_NAMESPACE::DApplet
{
    Q_OBJECT
    Q_PROPERTY(WindowEffectType effectType READ effectType WRITE setEffectType NOTIFY effectTypeChanged FINAL)
public:
    enum WindowEffectType {
        Default = 0,
        Best,
        Better,
        Good,
        Normal,
        Compatible
    };
    Q_ENUM(WindowEffectType)

    explicit WindowEffectApplet(QObject *parent = nullptr);

    void setEffectType(WindowEffectType effectType);
    WindowEffectType effectType() const;

Q_SIGNALS:
    void effectTypeChanged(WindowEffectType value);

private:
    void onWmConfigChanged(const QString &key);

private:
    WindowEffectType m_effectType;
    DConfig* m_wmConfig;
};
}
