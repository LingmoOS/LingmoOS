// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef SELECTEDITEMWIDGET_H
#define SELECTEDITEMWIDGET_H

#include "radiuswidget.h"
//#include "buttonlabel.h"

#include <DLabel>
#include <DIconButton>

DWIDGET_USE_NAMESPACE

/**
 * @class SelectedItemWidget
 * @brief 已选分区信息或者磁盘信息展示类
 */

class SelectedItemWidget : public RadiusWidget
{
    Q_OBJECT
public:
    explicit SelectedItemWidget(PVInfoData pvInfoData, QWidget *parent = nullptr);

signals:
    void deleteItem(PVInfoData pvInfoData);

private slots:
    /**
     * @brief 删除按钮点击响应的槽函数
     */
    void onDeleteClicked();

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
    DLabel *m_nameInfo;
    DLabel *m_sizeInfo;
//    ButtonLabel *m_buttonLabel;
    PVInfoData m_pvInfoData;
    DIconButton *m_iconButton;
};

#endif // SELECTEDITEMWIDGET_H
