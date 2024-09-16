// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef PARTITIONDIALOG_H
#define PARTITIONDIALOG_H

#include "customcontrol/ddbase.h"

#include <QWidget>
#include <DLabel>

DWIDGET_USE_NAMESPACE

/**
 * @class PartitionDialog
 * @brief 是否进行分区确认窗口类
 */

class PartitionDialog : public DDBase
{
    Q_OBJECT
public:
    explicit PartitionDialog(QWidget *parent = nullptr);

    /**
     * @brief 设置标题内容
     * @param title 标题内容
     * @param subTitle 副标题内容
     */
    void setTitleText(const QString &title, const QString &subTitle = "");

private:
    /**
     * @brief 初始化界面
     */
    void initUi();
    //    void initConnection();
signals:
    //    void showPartWidget();
public slots:
    //    void sendSignal(int index);
private:
    DLabel *m_tipLabel;
};

#endif // PARTITIONDIALOG_H
