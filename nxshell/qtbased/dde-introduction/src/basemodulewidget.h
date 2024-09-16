// Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BASEMODULEWIDGET_H
#define BASEMODULEWIDGET_H

#include <DFontSizeManager>
#include <DLabel>
#include <DPalette>
#include <DWidget>
#include <QVBoxLayout>

DGUI_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class BaseModuleWidget : public DWidget
{
    Q_OBJECT
public:
    explicit BaseModuleWidget(QWidget* content, QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setDescribe(const QString& describe, bool bIsFirst = false);
    DWidget* getModel() { return m_content; }
    /*
     * setContentWidgetHeight设置内容窗口高度
     */
    void setContentWidgetHeight(int iHeight);

private:
    QVBoxLayout* m_layout;
    DLabel* m_titleLbl;
    DLabel* m_describeLbl;
    DWidget* m_content;
};

#endif  // BASEMODULEWIDGET_H
