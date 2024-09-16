// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef NOTESFORM_H
#define NOTESFORM_H

#include "BaseWidget.h"

#include <QPointer>

namespace deepin_reader {
class Annotation;
}
class DocSheet;
class SideBarImageListView;
class DocummentProxy;

/**
 * @brief The NotesWidget class
 * 注释缩略图控件
 */
class NotesWidget : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(NotesWidget)

public:
    explicit NotesWidget(DocSheet *sheet, DWidget *parent = nullptr);

    ~NotesWidget() override;

    /**
     * @brief prevPage
     * 上一页
     */
    void prevPage();

    /**
     * @brief pageUp
     * 上一翻页
     */
    void pageUp();

    /**
     * @brief nextPage
     * 下一页
     */
    void nextPage();

    /**
     * @brief pageDown
     * 下一翻页
     */
    void pageDown();

    /**
     * @brief deleteItemByKey
     * 删除节点
     */
    void deleteItemByKey();

    /**
     * @brief handleOpenSuccess
     * 文档打开成功,初始化数据
     */
    void handleOpenSuccess();

    /**
     * @brief handleAnntationMsg
     * 注释操作,添加,删除,删除全部
     * @param anno
     */
    void handleAnntationMsg(const int &, deepin_reader::Annotation *anno);

    /**
     * @brief setTabOrderWidget
     * 设置TAB顺序
     * @param tabWidgetlst
     */
    void setTabOrderWidget(QList<QWidget *> &tabWidgetlst);

public:
    /**
     * @brief adaptWindowSize
     * 调整大小
     */
    void adaptWindowSize(const double &) Q_DECL_OVERRIDE;

    /**
     * @brief showMenu
     * 显示菜单
     */
    void showMenu();

public slots:
    /**
     * @brief changeResetModelData
     * 重新加载注释目录数据
     */
    void changeResetModelData();

private:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

private slots:
    /**
     * @brief onListMenuClick
     * 响应注释菜单点击
     */
    void onListMenuClick(const int &);

    /**
     * @brief onListItemClicked
     * 响应注释节点点击
     * @param row
     */
    void onListItemClicked(int row);

    /**
     * @brief onAddAnnotation
     * 响应添加注释按钮
     */
    void onAddAnnotation();

private:
    /**
     * @brief copyNoteContent
     * 拷贝注释内容
     */
    void copyNoteContent();

    /**
     * @brief addNoteItem
     * 添加注释
     * @param anno
     */
    void addNoteItem(deepin_reader::Annotation *anno);

    /**
     * @brief deleteNoteItem
     * 删除注释
     * @param anno
     */
    void deleteNoteItem(deepin_reader::Annotation *anno);

    /**
     * @brief deleteAllItem
     * 删除所有注释
     */
    void deleteAllItem();

private:
    QPointer<DocSheet> m_sheet;
    SideBarImageListView     *m_pImageListView = nullptr;
    DPushButton       *m_pAddAnnotationBtn = nullptr;
};

#endif  // NOTESFORM_H
