// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BOOKMARKFORM_H
#define BOOKMARKFORM_H

#include "BaseWidget.h"

#include <QPointer>

class DocSheet;
class SideBarImageListView;

/**
 * @brief The BookMarkWidget class
 * 书签目录控件类
 */
class BookMarkWidget : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(BookMarkWidget)

public:
    explicit BookMarkWidget(DocSheet *sheet, DWidget *parent = nullptr);
    ~BookMarkWidget() override;

    /**
     * @brief prevPage
     * 上一页
     */
    void prevPage();

    /**
     * @brief pageUp
     * 上翻一页
     */
    void pageUp();

    /**
     * @brief nextPage
     * 下一页
     */
    void nextPage();

    /**
     * @brief pageDown
     * 下翻一页
     */
    void pageDown();

    /**
     * @brief deleteItemByKey
     * 删除当前节点
     */
    void deleteItemByKey();

    /**
     * @brief handlePage
     * 跳转指定页节点
     * @param index
     */
    void handlePage(int index);

    /**
     * @brief handleOpenSuccess
     * 数据初始化
     */
    void handleOpenSuccess();

    /**
     * @brief handleBookMark
     * 设置书签节点状态
     * @param index
     * @param state
     */
    void handleBookMark(int index, int state);

    /**
     * @brief setTabOrderWidget
     * 设置TAB切换顺序控件
     * @param tabWidgetlst
     */
    void setTabOrderWidget(QList<QWidget *> &tabWidgetlst);

public:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

    /**
     * @brief adaptWindowSize
     * 调整控件大小
     */
    void adaptWindowSize(const double &) Q_DECL_OVERRIDE;

    /**
     * @brief showMenu
     * 显示菜单
     */
    void showMenu();

private slots:
    /**
     * @brief onUpdateTheme
     * 响应主题变更刷新
     */
    void onUpdateTheme();

    /**
     * @brief onAddBookMarkClicked
     * 响应添加书签按钮
     */
    void onAddBookMarkClicked();

    /**
     * @brief onListMenuClick
     * 响应菜单点击
     * @param iType
     */
    void onListMenuClick(const int &iType);

private:
    /**
     * @brief deleteAllItem
     * 删除所有书签节点
     */
    void deleteAllItem();

private:
    QPointer<DocSheet> m_sheet;
    DPushButton       *m_pAddBookMarkBtn = nullptr;
    SideBarImageListView     *m_pImageListView = nullptr;
};

#endif  // BOOKMARKFORM_H
