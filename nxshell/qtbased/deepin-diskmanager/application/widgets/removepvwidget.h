// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef REMOVEPVWIDGET_H
#define REMOVEPVWIDGET_H

#include "customcontrol/ddbase.h"
#include "partedproxy/dmdbushandler.h"
#include "customcontrol/waterloadingwidget.h"

#include <DLabel>
#include <DPushButton>
#include <DWarningButton>
#include <DStackedWidget>

#include <QWidget>

#include <set>
using std::set;

DWIDGET_USE_NAMESPACE

class RemovePVWidget : public DDBase
{
    Q_OBJECT
public:
    explicit RemovePVWidget(QWidget *parent = nullptr);

signals:

private slots:
    /**
     * @brief 取消按钮点击响应的槽函数
     */
    void onCancelButtonClicked();

    /**
     * @brief 删除按钮点击响应的槽函数
     */
    void onButtonClicked();

    /**
     * @brief 删除PV信号响应的槽函数
     */
    void onPVDeleteMessage(const QString &pvMessage);

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
    DPushButton *m_cancelButton;
    DWarningButton *m_deleteButton;
    DStackedWidget *m_stackedWidget;
    WaterLoadingWidget *m_waterLoadingWidget;
};

#endif // REMOVEPVWIDGET_H
