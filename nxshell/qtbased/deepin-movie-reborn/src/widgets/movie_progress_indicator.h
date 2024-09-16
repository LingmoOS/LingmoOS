// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
/**
  *@file 这个文件是负责实现全屏展示进度控件相关
  */
#ifndef _DMR_MOVIE_PROGRESS_INDICATOR_H
#define _DMR_MOVIE_PROGRESS_INDICATOR_H 

#include <QtWidgets>

namespace dmr {
/**
 * @brief The MovieProgressIndicator class
 * 这个类是全屏时右上角电影进度的展示控件
 */
class MovieProgressIndicator: public QFrame {
    Q_OBJECT
public:
    /**
     * @brief MovieProgressIndicator 构造函数
     * @param parent 父窗口
     */
    explicit MovieProgressIndicator(QWidget* parent);

public slots:
    /**
     * @brief updateMovieProgress 更新电影进度控件
     * @param duration 总时长
     * @param pos 当前时长
     */
    void updateMovieProgress(qint64 nDuration, qint64 nPos);

protected:
    /**
     * @brief paintEvent 重载绘制事件函数
     * @param pPaintEvent
     */
    void paintEvent(QPaintEvent* pPaintEvent) override;

private:
    /**
     * @brief initMember 初始化成员变量
     */
    void initMember();

private:
    qint64 m_nElapsed;   ///当前播放的进度
    qreal m_pert;        ///当前播放百分比
    QSize m_fixedSize;   ///窗口大小
};

}

#endif /* ifndef _DMR_MOVIE_PROGRESS_INDICATOR_H */
