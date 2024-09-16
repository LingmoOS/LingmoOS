/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WINDOWSTYLEMANAGER_H
#define WINDOWSTYLEMANAGER_H

#include <QObject>
#include <memory>

namespace KWin
{
class Window;
class Unmanaged;
class ConfigReader;

class WindowStyleManager : public QObject
{
    Q_OBJECT
public:
    explicit WindowStyleManager();
    ~WindowStyleManager();

    enum effectType {
        effectNone   = 0x0,        // 取消动效
        effectNormal = 0x01,       // 标准缩放动效
        effectCursor = 0x02,       // 鼠标位置展开动效
        effectTop    = 0x04,       // 从上往下展开
        effectBottom = 0x08,       // 从下往上展开
        effectOut    = 0x10,       // 由外向内
    };

public:
    float getOsRadius();
    float getOsScale();
    void handleSpecialWindowStyle(Window *);
    void parseWinCustomEffect(Window *);
    bool compositingEnabled() { return m_compositingEnabled; }

Q_SIGNALS:
    void sigRadiusChanged(float &);
    void sigThemeChanged(bool &);

public Q_SLOTS:
    void onRadiusChange(QVariant);
    void onThemeChange(QVariant);
    void onWindowAdded(Window*);
    void onWindowMaxiChanged(Window *, bool, bool);
    void onWindowActiveChanged();
    void onGeometryShapeChanged(Window *, QRectF);
    void onCompositingChanged(bool);
    void onWaylandWindowCustomEffect(uint32_t);
    void onWaylandWindowStartUpEffect(uint32_t);
    void onCompositingToggle(bool);

private:
    std::unique_ptr<ConfigReader> m_radiusConfig;
    std::unique_ptr<ConfigReader> m_themeConfig;
    float                         m_osRadius = -1.0;
    float                         m_scale = 1.0;
    bool                          m_compositingEnabled = true;
};
}

#endif
