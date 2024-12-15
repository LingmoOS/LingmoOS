// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef HANDLEMENU_H
#define HANDLEMENU_H

#include <DMenu>

class DocSheet;
/**
 * @brief The HandleMenu class
 * 选择工具菜单
 */
DWIDGET_USE_NAMESPACE
class HandleMenu : public DMenu
{
    Q_OBJECT
    Q_DISABLE_COPY(HandleMenu)

public:
    explicit HandleMenu(DWidget *parent = nullptr);

    /**
     * @brief readCurDocParam
     * 读取文档数据
     * @param docSheet
     */
    void readCurDocParam(DocSheet *docSheet);

protected:
    /**
     * @brief initActions
     * 初始化Action
     */
    void initActions();

private slots:
    /**
     * @brief onHandTool
     * 手型工具
     */
    void onHandTool();

    /**
     * @brief onSelectText
     * 文本选择工具
     */
    void onSelectText();

private:
    QAction *m_textAction = nullptr;
    QAction *m_handAction = nullptr;
    DocSheet *m_docSheet = nullptr;
};

#endif // HANDLEMENU_H
