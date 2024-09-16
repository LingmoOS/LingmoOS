// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TIMEWIDGET_H
#define TIMEWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QTime>
#include <QIcon>
#include <DWidget>
#include <com_deepin_dde_daemon_dock.h>
#include <DFontSizeManager>
#include <QBoxLayout>
#include <QLabel>

DWIDGET_USE_NAMESPACE
using DBusDock = com::deepin::dde::daemon::Dock;

class TimeWidget : public DWidget
{
    Q_OBJECT

public:
    explicit TimeWidget(DWidget *parent = nullptr);
    ~TimeWidget();
    bool enabled();

    /**
     * @brief start:开始计时
     */
    void start();

    /**
     * @brief stop:停止计时
     */
    void stop();

    /**
     * @brief 是否是wayland协议
     * @return
     */
    bool isWaylandProtocol();
    
protected:
    void showEvent(QShowEvent *) override;
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void leaveEvent(QEvent *e) override;
    /**
     * @brief 创建缓存文件，只有wayland模式下的mips或部分arm架构适用
     */
    bool createCacheFile();
private slots:
    /**
     * @brief onTimeout:更新数据
     */
    void onTimeout();

    /**
     * @brief onPositionChanged:dde-dock位置变化通知
     * @param value
     */
    void onPositionChanged(int value);

private:
    void updateIcon();

private:
    QTimer *m_timer;
    DBusDock *m_dockInter;
    QIcon *m_lightIcon;
    QIcon *m_shadeIcon;
    QIcon *m_currentIcon;
    QLabel *m_iconLabel;
    QLabel *m_textLabel;
    QPixmap m_pixmap;
    QTime m_baseTime;
    bool m_bRefresh;
    int m_position;
    bool m_hover;
    bool m_pressed;

    /**
     * @brief m_lightIcon1070 1070下录屏计时图标icon
     */
    QIcon m_lightIcon1070;
    /**
     * @brief m_shadeIcon1070 1070下录屏计时图标icon
     */
    QIcon m_shadeIcon1070;
};

#endif // TIMEWIDGET_H
