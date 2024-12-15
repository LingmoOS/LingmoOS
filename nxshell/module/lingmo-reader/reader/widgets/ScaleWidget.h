// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCALEWIDGET_H
#define SCALEWIDGET_H

#include "DocSheet.h"
#include "ScaleMenu.h"

#include <DWidget>
#include <DIconButton>
#include <DLineEdit>
#include <DGuiApplicationHelper>

#include <QPointer>

class DocSheet;
class ScaleMenu;
/**
 * @brief The ScaleWidget class
 * 缩放控件
 */
class ScaleWidget : public Dtk::Widget::DWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ScaleWidget)

public:
    explicit ScaleWidget(Dtk::Widget::DWidget *parent = nullptr);

    ~ScaleWidget();

    /**
     * @brief setSheet
     * 设置文档对象
     * @param sheet
     */
    void setSheet(DocSheet *sheet);

    /**
     * @brief clear
     * 清空缩放比缩放框
     */
    void clear();

private:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

public slots:
    /**
     * @brief onPrevScale
     * 缩减25%缩放比
     */
    void onPrevScale();

    /**
     * @brief onNextScale
     * 缩增25%缩放比
     */
    void onNextScale();

private slots:
    /**
     * @brief onReturnPressed
     * 缩放比输入框确认响应
     */
    void onReturnPressed();

    /**
     * @brief onEditFinished
     * 输入框编辑完毕
     */
    void onEditFinished();

    /**
     * @brief onArrowBtnlicked
     * 缩放下拉菜单按钮点击
     */
    void onArrowBtnlicked();

#ifdef DTKWIDGET_CLASS_DSizeMode
    /**
     * @brief onSizeModeChanged
     * 控制中心>个性化>切换紧凑模式
     */
    void onSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

private:
    QPointer<DocSheet> m_sheet;
    Dtk::Widget::DLineEdit *m_lineEdit = nullptr;
    ScaleMenu *m_ScaleMenu = nullptr;
    DIconButton *m_arrowBtn;


};

#endif // SCALEWIDGET_H
