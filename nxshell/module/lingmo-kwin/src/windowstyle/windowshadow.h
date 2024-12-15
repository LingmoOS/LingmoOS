/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2024 zhang yu <zhangyud@uniontech.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#ifndef WINDOWSHADOW_H
#define WINDOWSHADOW_H 

#include <QObject>
#include <QPointF>
#include <QColor>
#include <QImage>
#include <QMargins>
#include <QMap>
#include <QVector>

namespace KWin
{
class Window;

struct shadowConfig
{
    QPoint windowRadius;
    QPoint shadowOffset;
    QColor  shadowColor;
    int   shadowRadius;
    int   borderWidth;
    QColor  borderColor;
};

class WindowShadow : public QObject
{
    Q_OBJECT
public:
    explicit WindowShadow(Window *window);
    ~WindowShadow();

    bool            updateWindowShadow();
    QPointF         getWindowRadius();
    QString         getDefaultShadowColor();
    qreal           getDefaultShadowRadius();
    QString         getDefaultBorderColor();
    float           getDefaultShadowOffset();

    bool            isSpecialWindow();

    static QString  buildShadowCacheKey(shadowConfig &config);
    bool            getShadow();
    void            resetShadowKey();
    QString         getShadowKey();
    QMargins        getPadding();

public Q_SLOTS:
    void onUpdateBorderWidthChanged();
    void onUpdateBorderColorChanged();
    void onUpdateShadowColorChanged();

public:
    static QMap<QString, QVector<QImage>> m_cacheShadow;
private:
    QImage  kwin_popup_shadow_top,
            kwin_popup_shadow_top_right,
            kwin_popup_shadow_right,
            kwin_popup_shadow_bottom_right,
            kwin_popup_shadow_bottom,
            kwin_popup_shadow_bottom_left,
            kwin_popup_shadow_left,
            kwin_popup_shadow_top_left;

    QMargins m_padding;
    QString  m_key;
    QString  m_lastKey;
    Window   *m_window;
};
}
#endif