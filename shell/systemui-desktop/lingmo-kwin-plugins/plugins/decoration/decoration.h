/*
 * Copyright (C) 2020 PandaOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#pragma once

// C++ STL
#include <memory>

// KDecoration
#include <KDecoration2/Decoration>
#include <KDecoration2/DecorationButtonGroup>

// Qt
#include <QFileSystemWatcher>
#include <QSettings>
#include <QVariant>
#include <QIcon>

#include "x11shadow.h"
#include "helper.h"

namespace Lingmo
{

class CloseButton;
class MaximizeButton;
class MinimizeButton;

class Decoration : public KDecoration2::Decoration
{
    Q_OBJECT

public:
    Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Decoration() override;

    virtual void paint(QPainter *painter, const QRect &repaintRegion) override;

    QPixmap closeBtnPixmap() { return m_closeBtnPixmap; }
    QPixmap maximizeBtnPixmap() { return m_maximizeBtnPixmap; }
    QPixmap minimizeBtnPixmap() { return m_minimizeBtnPixmap; }
    QPixmap restoreBtnPixmap() { return m_restoreBtnPixmap; }

    bool darkMode() const;
    qreal devicePixelRatio() const { return m_devicePixelRatio; }

public slots:

#if KDECORATION_VERSION <= QT_VERSION_CHECK(5, 27, 11) 
    virtual void init() override;
#else
    virtual bool init() override;
#endif

private:
    void reconfigure();
    void createButtons();
    void recalculateBorders();
    void updateResizeBorders();
    void updateTitleBar();
    void updateButtonsGeometryDelayed();
    void updateButtonsGeometry();
    void updateShadow();

    void updateBtnPixmap();
    QPixmap fromSvgToPixmap(const QString &file, const QSize &size);

    int titleBarHeight() const;

    QColor titleBarBackgroundColor() const;
    QColor titleBarForegroundColor() const;

    bool radiusAvailable() const;
    bool isMaximized() const;

    void paintFrameBackground(QPainter *painter, const QRect &repaintRegion) const;
    void paintCaption(QPainter *painter, const QRect &repaintRegion) const;
    void paintButtons(QPainter *painter, const QRect &repaintRegion) const;

    KDecoration2::DecorationButtonGroup *m_leftButtons;
    KDecoration2::DecorationButtonGroup *m_rightButtons;

    friend class CloseButton;
    friend class MaximizeButton;
    friend class MinimizeButton;

private:
    int m_titleBarHeight = 30;
    int m_frameRadius = 11;
    qreal m_devicePixelRatio = 1.0;
    QColor m_titleBarBgColor = QColor(255, 255, 255, 255);
    QColor m_titleBarFgColor = QColor(56, 56, 56, 255);
    QColor m_unfocusedFgColor = QColor(127, 127, 127, 255);

    QColor m_titleBarBgDarkColor = QColor(44, 44, 45);
    QColor m_titleBarFgDarkColor = QColor(202, 203, 206);
    QColor m_unfocusedFgDarkColor = QColor(112, 112, 112);

    QSettings *m_settings;
    QString m_settingsFile;
    QFileSystemWatcher *m_fileWatcher;

    QPixmap m_closeBtnPixmap;
    QPixmap m_maximizeBtnPixmap;
    QPixmap m_minimizeBtnPixmap;
    QPixmap m_restoreBtnPixmap;

    X11Shadow *m_x11Shadow;
};

}
