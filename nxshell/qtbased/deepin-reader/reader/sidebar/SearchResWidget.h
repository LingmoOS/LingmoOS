// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SEARCHRESWIDGET_H
#define SEARCHRESWIDGET_H

#include "BaseWidget.h"

namespace deepin_reader {
struct SearchResult;
}
class DocSheet;
class SideBarImageListView;
class QStackedLayout;

/**
 * @brief The SearchResWidget class
 * 搜索目录控件
 */
class SearchResWidget : public BaseWidget
{
    Q_OBJECT
    Q_DISABLE_COPY(SearchResWidget)

public:
    explicit SearchResWidget(DocSheet *sheet, DWidget *parent = nullptr);
    ~SearchResWidget() override;

    /**
     * @brief clearFindResult
     * 清空搜索目录结果
     */
    void clearFindResult();

    /**
     * @brief handleFindFinished
     * 搜索完毕
     * @return 搜索结果个数
     */
    int  handleFindFinished();

    /**
     * @brief handleSearchResultComming
     *
     */
    void handleSearchResultComming(const deepin_reader::SearchResult &);

    /**
     * @brief searchKey
     * 搜索关键词
     * @param searchKey
     */
    void searchKey(const QString &searchKey);

public:
    /**
     * @brief adaptWindowSize
     * 控件大小变化响应
     */
    void adaptWindowSize(const double &) Q_DECL_OVERRIDE;

protected:
    /**
     * @brief initWidget
     * 初始化控件
     */
    void initWidget();

private:
    /**
     * @brief addSearchsItem
     * 添加搜索节点
     * @param pageIndex 页数
     * @param text 文本内容
     * @param resultNum 搜索个数
     */
    void addSearchsItem(const int &pageIndex, const QString &text, const int &resultNum);

private:
    DocSheet *m_sheet = nullptr;
    QString m_searchKey;
    QStackedLayout *m_stackLayout = nullptr;
    SideBarImageListView *m_pImageListView = nullptr;
};

#endif  // NOTESFORM_H
