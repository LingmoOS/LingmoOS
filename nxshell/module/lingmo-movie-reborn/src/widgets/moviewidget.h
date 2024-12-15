// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
 *@file 这个文件是播放音乐时显示的窗口动画
 */
#ifndef MOVIEWIDGET_H
#define MOVIEWIDGET_H

#include <DWidget>
#include <QResizeEvent>
#include <QSvgRenderer>
#include <QGraphicsView>
#include <QGraphicsSvgItem>

DWIDGET_USE_NAMESPACE

class QTimer;
class QHBoxLayout;
class QLabel;

namespace dmr {
/**
 * @brief The MovieWidget class
 * 播放音乐时动画效果显示窗口类
 */
class MovieWidget: public QGraphicsView
{
    Q_OBJECT

    enum PlayState {
        STATE_PLAYING = 0,
        STATE_PAUSE,
        STATE_STOP
    };

public:
    /**
     * @brief MovieWidget 播放动画显示窗口
     * @param parent 父窗口
     */
    explicit MovieWidget(QWidget *parent = nullptr);
    ~MovieWidget();

public slots:
    /**
     * @brief startPlaying
     * 开始播放时的槽函数
     */
    void startPlaying();
    /**
     * @brief stopPlaying
     * 停止播放时的槽函数
     */
    void stopPlaying();
    /**
     * @brief pausePlaying
     * 暂停播放时的槽函数
     */
    void pausePlaying();
    /**
     * @brief updateView
     * 更新窗口函数
     */
    void updateView();
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

protected:
    void mousePressEvent(QMouseEvent* pEvent) override;

    void mouseReleaseEvent(QMouseEvent* pEvent) override;

    void mouseMoveEvent(QMouseEvent* pEvent) override;

private:
    QGraphicsSvgItem *m_pBgSvgItem;
    QGraphicsSvgItem *m_pNoteSvgItem;
    QGraphicsScene *m_pScene;
    QTimer *m_pTimer;             ///控制动画播放速度的时间
    int m_nRotate;                ///旋转角度
    PlayState m_state;            ///播放状态
    QSvgRenderer *m_pBgRender;    ///背景render
    QSvgRenderer *m_pNoteRender;  ///音符render
};

}

#endif // MOVIEWIDGET_H
