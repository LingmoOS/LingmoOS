// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BrowserMenu_H
#define BrowserMenu_H

#include <DMenu>

DWIDGET_USE_NAMESPACE
typedef enum SheetMenuType_e {
    DOC_MENU_DEFAULT        = 0,
    DOC_MENU_ANNO_ICON      = 1,
    DOC_MENU_ANNO_HIGHLIGHT = 2,
    DOC_MENU_SELECT_TEXT    = 3,
    DOC_MENU_KEY            = 4
} SheetMenuType_e;

class DocSheet;
class ColorWidgetAction;

/**
 * @brief The BrowserMenu class
 * 文档菜单
 */
class BrowserMenu : public DMenu
{
    Q_OBJECT
signals:
    /**
     * @brief signalMenuItemClicked
     * 点击菜单中的哪一项(信号)
     * @param objectname
     */
    void signalMenuItemClicked(const QString &objectname);

    void sigMenuHide();

public:
    explicit BrowserMenu(QWidget *parent = nullptr);
    /**
     * @brief initActions
     * 初始化菜单
     * @param sheet
     * @param index
     * @param type
     * @param copytext
     */
    void initActions(DocSheet *sheet, int index, SheetMenuType_e type, const QString &copytext = "");

    // QWidget interface
protected:
    void hideEvent(QHideEvent *event) override;

private slots:
    /**
     * @brief onItemClicked
     *单击菜单项槽函数
     */
    void onItemClicked();
    /**
     * @brief onSetHighLight
     * 相应高亮擦操作
     */
    void onSetHighLight();

private:
    /**
     * @brief createAction
     * 创建菜单i子项
     * @param displayname
     * @param objectname
     * @return
     */
    QAction *createAction(const QString &displayname, const QString &objectname);

private:
    int m_type;

    ColorWidgetAction *m_pColorWidgetAction;

};

#endif // BrowserMenu_H
