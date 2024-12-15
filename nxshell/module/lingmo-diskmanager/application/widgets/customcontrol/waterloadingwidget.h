// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef WATERLOADINGWIDGET_H
#define WATERLOADINGWIDGET_H

#include <DWaterProgress>

#include <QWidget>
#include <QTimer>

DWIDGET_USE_NAMESPACE

/**
 * @class WaterLoadingWidget
 * @brief 水滴加载动画类
 */

class WaterLoadingWidget : public QWidget
{
    Q_OBJECT
public:
    explicit WaterLoadingWidget(QWidget *parent = nullptr);

    /**
     * @brief 设置定时器时间
     * @param msec 毫秒
     */
    void setStartTime(int msec);

    /**
     * @brief 停止定时器
     */
    void stopTimer();

signals:

public slots:

private slots:
    /**
     * @brief 水滴动画进度改变响应的槽函数
     */
    void onStartWaterProgress();

private:
    /**
     * @brief 初始化界面
     */
    void initUi();

    /**
     * @brief 初始化连接
     */
    void initConnection();

private:
    DWaterProgress *m_waterProgress;
    QTimer *m_time;
};

#endif // WATERLOADINGWIDGET_H
