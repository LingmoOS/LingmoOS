// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMCOBOXGROUPWIDGET_H
#define COMCOBOXGROUPWIDGET_H
#include <DLineEdit>
#include <DLabel>

#include "attributewidget.h"
/**
 * @brief 通用水平布局窗口，用于属性水平布局
 */
class HBoxLayoutWidget : public AttributeWgt
{
    Q_OBJECT
public:
    explicit HBoxLayoutWidget(QWidget *parent = nullptr);
    // 设置属性控件的标题
    void setTitle(const QString &title);
    void addWidget(QWidget *widget, int stretch = 0);
    QHBoxLayout *getLayout();
    DLabel *getLabel();
private:
    void initUi();
private:
    DLabel             *m_label = nullptr;
    QHBoxLayout        *m_lay = nullptr;
};
#endif // COMCOBOXGROUPWIDGET_H
