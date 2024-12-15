// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DRIVERICONWIDGET_H
#define DRIVERICONWIDGET_H

#include <DWidget>
#include <DWaterProgress>

#include <QObject>

DWIDGET_USE_NAMESPACE

class DriverIconWidget : public DWidget
{
    Q_OBJECT
public:
    /**
     * @brief DriverIconWidget 返回有进度条的widget
     * @param strTitle 第一文本
     * @param strDesc 第二文本
     * @param parent
     */
    explicit DriverIconWidget(const QString &strTitle,
                              const QString &strDesc,
                              QWidget *parent = nullptr);
    /**
     * @brief DriverIconWidget 返回有icon的widget
     * @param pixmap 图标
     * @param strTitle 第一文本
     * @param strDesc 第二文本
     * @param parent
     */
    explicit DriverIconWidget(const QPixmap &pixmap,
                              const QString &strTitle,
                              const QString &strDesc,
                              QWidget *parent = nullptr);

    virtual ~DriverIconWidget();

private:
    /**
     * @brief initUI 初始化UI
     * @param iconWidget iconWidget
     * @param strTitle 第一文本
     * @param strDesc 第二文本
     */
    void initUI(QWidget *iconWidget, const QString &strTitle, const QString &strDesc);

public:

signals:
    /**
     * @brief sigProgressFinished 进度到100%时，发送此信号
     */
    void sigProgressFinished();

private:
    DWaterProgress *iconWidget = nullptr;
};

#endif // DRIVERICONWIDGET_H
