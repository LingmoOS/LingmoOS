// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIDEBARIMAGELISTVIEW_H
#define SIDEBARIMAGELISTVIEW_H

#include <DListView>
#include <DMenu>

DWIDGET_USE_NAMESPACE
struct ImagePageInfo_t;

class DocSheet;
class SideBarImageViewModel;
class BookMarkMenu;
class NoteMenu;
class QMouseEvent;

const int LEFTMINWIDTH = 266;
const int LEFTMAXWIDTH = 380;

/**
 * @brief The SideBarImageListView class
 * 左侧栏缩略图目录VIEW
 */
class SideBarImageListView : public DListView
{
    Q_OBJECT
public:
    SideBarImageListView(DocSheet *sheet, QWidget *parent = nullptr);

signals:
    /**
     * @brief sigListMenuClick
     * 节点右键菜单点击
     */
    void sigListMenuClick(const int &);

    /**
     * @brief sigListItemClicked
     * 节点左键点击
     * @param row
     */
    void sigListItemClicked(int row);

public:
    /**
     * @brief handleOpenSuccess
     * 文档打开成功,初始化相关数据
     */
    void handleOpenSuccess();

    /**
     * @brief setListType
     * 设置ListView类型
     * @param type 缩略图,目录,书签,注释,搜索
     */
    void setListType(int type);

    /**
     * @brief scrollToIndex
     * 滚动到指定页数
     * @param pageIndex
     * @param scrollTo
     * @return
     */
    bool scrollToIndex(int pageIndex, bool scrollTo = true);

    /**
     * @brief scrollToModelInexPage
     * 滚动到指定节点
     * @param index
     * @param scrollto
     */
    void scrollToModelInexPage(const QModelIndex &index, bool scrollto = true);

    /**
     * @brief getModelIndexForPageIndex
     * 根据页数获取节点索引
     * @param pageIndex
     * @return
     */
    int  getModelIndexForPageIndex(int pageIndex);

    /**
     * @brief getPageIndexForModelIndex
     * 根据节点索引获取文档页码
     * @param row
     * @return
     */
    int  getPageIndexForModelIndex(int row);

    /**
     * @brief getImageModel
     * 获取model
     * @return
     */
    SideBarImageViewModel *getImageModel();

    /**
     * @brief showMenu
     * 显示菜单
     */
    void showMenu();

    /**
     * @brief pageUpIndex
     * 上翻页
     * @return
     */
    QModelIndex pageUpIndex();

    /**
     * @brief pageDownIndex
     * 下翻页
     * @return
     */
    QModelIndex pageDownIndex();

private:
    /**
     * @brief initControl
     * 初始化控件
     */
    void initControl();

    /**
     * @brief showNoteMenu
     * 显示注释菜单
     */
    void showNoteMenu(const QPoint &point);

    /**
     * @brief showBookMarkMenu
     * 显示书签菜单
     */
    void showBookMarkMenu(const QPoint &point);

private slots:
    /**
     * @brief onItemClicked
     * 节点点击
     * @param index
     */
    void onItemClicked(const QModelIndex &index);

    /**
     * @brief onSetDocSlideGesture
     * 设置文档触摸屏滑动事件
     */
    void onSetThumbnailListSlideGesture();

    /**
     * @brief onRemoveDocSlideGesture
     * 移除文档触摸屏滑动事件
     */
    void onRemoveThumbnailListSlideGesture();

protected:
    /**
     * @brief mousePressEvent
     * 鼠标点击事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event);

private:
    int m_listType;
    DocSheet *m_docSheet;
    SideBarImageViewModel *m_imageModel;

    DMenu *m_pBookMarkMenu;
    DMenu *m_pNoteMenu;
};

#endif // SIDEBARIMAGELISTVIEW_H
