// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * (c) 2017, Deepin Technology Co., Ltd. <support@deepin.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * is provided AS IS, WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, and
 * NON-INFRINGEMENT.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/**
 *@file 这个文件是播放音乐时显示的窗口动画
 */
#ifndef MIRCASTSHOWWIDGET_H
#define MIRCASTSHOWWIDGET_H

#include <QGraphicsView>
#include <QGraphicsSvgItem>

#include <DLabel>

DWIDGET_USE_NAMESPACE

class QSvgWidget;
class ExitButton: public QWidget
{
    Q_OBJECT
public:
    enum ButtonState {
        Normal,
        Hover,
        Press,
    };
public:
    ExitButton(QWidget *parent = nullptr);

signals:
    void exitMircast();

protected:
    void enterEvent(QEvent *pEvent) override;
    void leaveEvent(QEvent *pEvent) override;
    void mousePressEvent(QMouseEvent *pEvent) override;
    void mouseReleaseEvent(QMouseEvent *pEvent) override;
    void paintEvent(QPaintEvent *pEvent) override;
private:
    QSvgWidget *m_svgWidget;//投屏退出图标窗口
    ButtonState m_state; //按钮状态
};

class MircastShowWidget: public QGraphicsView
{
    Q_OBJECT
public:
    explicit MircastShowWidget(QWidget *parent = nullptr);
    ~MircastShowWidget();
    /**
     * @brief setDeviceName 设置投屏设备名称
     */
    void setDeviceName(QString);

protected:
    void mouseMoveEvent(QMouseEvent* pEvent) override;

signals:
    void exitMircast();

private:
    /**
     * @brief customizeText 设置投屏设备显示名称
     * @param name 设备名
     */
    QString customizeText(QString name);

private:
    QGraphicsSvgItem *m_pBgSvgItem;
    QGraphicsScene *m_pScene;
    QGraphicsTextItem *m_deviceName;
    QSvgRenderer *m_pBgRender;    ///背景render
};

#endif  //MIRCASTSHOWWIDGET_H
