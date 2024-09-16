// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCALEMENU_H
#define SCALEMENU_H

#include <DMenu>

class DocSheet;
/**
 * @brief The ScaleMenu class
 * 缩放比菜单
 */
DWIDGET_USE_NAMESPACE
class ScaleMenu : public DMenu
{
    Q_OBJECT

public:
    explicit ScaleMenu(QWidget *parent = nullptr);

    /**
     * @brief readCurDocParam
     * 读取文档数据参数
     * @param docSheet
     */
    void readCurDocParam(DocSheet *docSheet);

private slots:
    /**
     * @brief onTwoPage
     * 双页显示
     */
    void onTwoPage();

    /**
     * @brief onFiteH
     * 适应高度
     */
    void onFiteH();

    /**
     * @brief onFiteW
     * 适应宽度
     */
    void onFiteW();

    /**
     * @brief onDefaultPage
     * 默认大小
     */
    void onDefaultPage();

    /**
     * @brief onFitPage
     * 适合页面
     */
    void onFitPage();

    /**
     * @brief onScaleFactor
     * 百分比
     */
    void onScaleFactor();

private:
    /**
     * @brief 创建Aciton
     * @param objName
     * @param member
     * @param checkable
     * @return
     */
    QAction *createAction(const QString &objName, const char *member, bool checkable);

private:
    QAction *m_pTwoPageAction = nullptr;
    QAction *m_pFitDefaultAction = nullptr;
    QAction *m_pFitWorHAction = nullptr;
    QAction *m_pFiteHAction = nullptr;
    QAction *m_pFiteWAction = nullptr;
    DocSheet *m_sheet = nullptr;
    QList<QAction *> m_actionGroup;
};

#endif // SCALEMENU_H
