/*
 * Peony-Qt's Library
 *
 * Copyright (C) 2020, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: MeihongHe <hemeihong@kylinos.cn>
 *
 */

#ifndef TABSTATUSBAR_H
#define TABSTATUSBAR_H

#include <QStatusBar>
#include <main-window.h>

class QLabel;
class QToolBar;
class TabWidget;
class ElidedLabel;
class QSlider;
class QVariantAnimation;

class TabStatusBar : public QStatusBar
{
    friend class MainWindow;
    friend class TabWidget;
    Q_OBJECT
public:
    explicit TabStatusBar(TabWidget *tab, QWidget *parent = nullptr);
    ~TabStatusBar() override;

    int currentZoomLevel();

Q_SIGNALS:
    void zoomLevelChangedRequest(int zoomLevel);

public Q_SLOTS:
    void update();
    void update(const QString &message);
    void updateZoomLevelState(int zoomLevel);

    void onZoomRequest(bool zoomIn);
    //显示隐藏文件，更新项目个数
    void updateItemsNum();
    void updateSearchProgress(bool searching);

protected:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;

    void resizeEvent(QResizeEvent *e) override;

private:
    TabWidget *m_tab = nullptr;
    ElidedLabel *m_label = nullptr;
    QToolBar *m_styled_toolbar = nullptr;
    QSlider *m_slider;
    bool m_searching = false;
    QVariantAnimation* m_animation;
};

class ElidedLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ElidedLabel(QWidget *parent);

    void setText(const QString &text);
    void setSearch(bool searching);
    void setValue(double value);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QString m_text;
    QPixmap m_disc;
    bool m_searching = false;
    double m_val = 0.0;
};

#endif // TABSTATUSBAR_H
