// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef THUMBNAILWIDGET_H
#define THUMBNAILWIDGET_H

#include "BaseWidget.h"

#include <QPointer>

class DocSheet;
class PagingWidget;
class SideBarImageListView;
class DocummentProxy;

/**
 * @brief The ThumbnailWidget class
 * 缩略图目录
 */
class ThumbnailWidget : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(ThumbnailWidget)

public:
    explicit ThumbnailWidget(DocSheet *sheet, DWidget *parent = nullptr);
    ~ThumbnailWidget() Q_DECL_OVERRIDE;

public:
    /**
     * @brief handleOpenSuccess
     * 文档打开成功,初始化数据
     */
    void handleOpenSuccess();

    /**
     * @brief handlePage
     * 滚动到指定页数
     * @param index
     */
    void handlePage(int index);

    /**
     * @brief handleRotate
     * 缩略图旋转
     */
    void handleRotate();

    /**
     * @brief setBookMarkState
     * 设置书签状态
     * @param index 页数
     * @param type 1: 可见 0: 不可见
     */
    void setBookMarkState(const int &index, const int &type);

    /**
     * @brief prevPage
     * 上一页
     */
    void prevPage();

    /**
     * @brief pageUp
     * 上翻页
     */
    void pageUp();

    /**
     * @brief nextPage
     * 下一页
     */
    void nextPage();

    /**
     * @brief pageDown
     * 下翻页
     */
    void pageDown();

    /**
     * @brief adaptWindowSize
     * 控件大小变化响应
     * @param scale
     */
    void adaptWindowSize(const double &scale)  Q_DECL_OVERRIDE;

    /**
     * @brief setTabOrderWidget
     * 设置TAB控件切换顺序
     * @param tabWidgetlst
     */
    void setTabOrderWidget(QList<QWidget *> &tabWidgetlst);

protected:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

private:
    /**
     * @brief scrollToCurrentPage
     * 滚动到当前页数
     */
    void scrollToCurrentPage();

private:
    QPointer<DocSheet> m_sheet;
    PagingWidget *m_pPageWidget = nullptr;
    SideBarImageListView *m_pImageListView = nullptr;
};

#endif  // THUMBNAILWIDGET_H
