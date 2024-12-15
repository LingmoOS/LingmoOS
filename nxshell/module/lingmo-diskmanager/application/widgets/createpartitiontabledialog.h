// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only

#ifndef CREATEPARTITIONTABLEDIALOG_H
#define CREATEPARTITIONTABLEDIALOG_H

#include "customcontrol/ddbase.h"

#include <DComboBox>

DWIDGET_USE_NAMESPACE

#include <QWidget>

class CreatePartitionTableDialog : public DDBase
{
    Q_OBJECT
public:
    explicit CreatePartitionTableDialog(QWidget *parent = nullptr);

signals:

private slots:

    /**
     * @brief 按钮点击响应的槽函数
     * @param index 当前点击按钮
     * @param text 当前点击按钮文本
     */
    void onButtonClicked(int index, const QString &text);

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
    DComboBox *m_ComboBox;
    QString m_curType;
};

#endif // CREATEPARTITIONTABLEDIALOG_H
