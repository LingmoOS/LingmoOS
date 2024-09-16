/*
 * Copyright (C) 2017 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     zccrs <zccrs@live.com>
 *
 * Maintainer: zccrs <zhangjide@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CHAMELEON_H
#define CHAMELEON_H

#include "chameleontheme.h"

#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButtonGroup>

#include <kwineffects.h>

#include <QVariant>
#include <QDir>
#include <QSettings>
#include <QScreen>
#include <QPainterPath>
#include <QFont>
#include "wayland/ddeshell_interface.h"

class Settings;
class ChameleonWindowTheme;
class Chameleon : public KDecoration2::Decoration
{
    Q_OBJECT

public:
    explicit Chameleon(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Chameleon();

    void paint(QPainter *painter, const QRect &repaintArea) override;

    const ChameleonTheme::ThemeConfig *themeConfig() const;
    KWin::EffectWindow *effect() const;
    bool noTitleBar() const;

    qreal borderWidth() const;
    qreal titleBarHeight() const;
    qreal shadowRadius() const;
    QPointF shadowOffset() const;
    QPointF windowRadius() const;
    QMarginsF mouseInputAreaMargins() const;

    QColor shadowColor() const;
    QColor borderColor() const;
    QColor getBackgroundColor() const;

    QIcon menuIcon() const;
    QIcon minimizeIcon() const;
    QIcon maximizeIcon() const;
    QIcon unmaximizeIcon() const;
    QIcon closeIcon() const;

    QPointF menuIconPos() const;
    qint32 menuIconWidth() const;
    qint32 menuIconHeight() const;

Q_SIGNALS:
    void noTitleBarChanged(bool noTitleBar);
    void effectInitialized(KWin::EffectWindow *effect);

public Q_SLOTS:
    void init() override;

private Q_SLOTS:
    void updateFont(QString updateType, QString val);
    void handleTitlebarHeightChanged();

private:
    void initButtons();
    void updateButtonsGeometry();

    void updateTitleGeometry();

    void updateTheme();
    void updateConfig();
    void updateTitleBarArea();
    void updateBorderPath();
    void updateShadow();
    void updateMouseInputAreaMargins();

    void onClientWidthChanged();
    void onClientHeightChanged();
    void onNoTitlebarPropertyChanged(quint32 windowId);

    void onThemeWindowRadiusChanged();
    void onThemeBorderWidthChanged();
    void onThemeBorderColorChanged();
    void onThemeShadowRadiusChanged();
    void onThemeShadowOffsetChanged();

    bool windowNeedRadius() const;
    bool windowNeedBorder() const;

    QColor getTextColor() const;

    bool m_initialized = false;
    qint8 m_noTitleBar = -1;
    QObject *m_client = nullptr;

    QMarginsF m_titleBarAreaMargins;
    QPainterPath m_borderPath;
    ChameleonTheme::ConfigGroup* m_configGroup = nullptr;
    ChameleonTheme::ThemeConfig *m_config = nullptr;
    ChameleonWindowTheme *m_theme = nullptr;

    QString m_title;
    QRect m_titleArea;

    KDecoration2::DecorationButtonGroup *m_leftButtons = nullptr;
    KDecoration2::DecorationButtonGroup *m_rightButtons = nullptr;

    QPointer<KWin::EffectWindow> m_effect;
    QFont m_font;
    KWaylandServer::DDEShellSurfaceInterface * m_ddeShellSurface = nullptr;
};

#endif // CHAMELEON_H
