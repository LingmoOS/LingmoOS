// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CATALOGTREEVIEW_H
#define CATALOGTREEVIEW_H

#include <DTreeView>

#include <QMouseEvent>
#include <QStandardItemModel>

DWIDGET_USE_NAMESPACE
namespace deepin_reader {
struct Section;
}
class DocSheet;

/**
 * @brief The CatalogTreeView class
 * 文档目录
 */
class CatalogTreeView : public DTreeView
{
    Q_OBJECT
    Q_DISABLE_COPY(CatalogTreeView)

public:
    explicit CatalogTreeView(DocSheet *sheet, DWidget *parent = nullptr);

    ~CatalogTreeView() override;

    /**
     * @brief setIndex
     * 指定目录页数
     * @param index
     * @param title
     */
    void setIndex(int index, const QString &title = "");

    /**
     * @brief setRightControl
     * 主动触发,被动触发
     * @param hasControl
     * true: 被动触发,不需要跳转指定页数
     * false: 主动触发,需要跳转指定页数
     */
    void setRightControl(bool hasControl);

    /**
     * @brief nextPage
     * 下一页
     */
    void nextPage();

    /**
     * @brief pageDownPage
     * 下一翻页
     */
    void pageDownPage();

    /**
     * @brief prevPage
     * 上一页
     */
    void prevPage();

    /**
     * @brief pageUpPage
     * 上一翻页
     */
    void pageUpPage();

public:
    /**
     * @brief handleOpenSuccess
     * 数据初始化
     */
    void handleOpenSuccess();

protected:
    /**
     * @brief resizeEvent
     * 控件大小变化事件
     * @param event
     */
    void resizeEvent(QResizeEvent *event) override;

    /**
     * @brief mousePressEvent
     * 鼠标点击事件
     * @param event
     */
    void mousePressEvent(QMouseEvent *event) override;

    /**
     * @brief keyPressEvent
     * 按键事件
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief currentChanged
     * 当前节点发生变化
     * @param current
     * 新节点
     * @param previous
     * 旧节点
     */
    void currentChanged(const QModelIndex &current, const QModelIndex &previous) override;

    /**
     * @brief onItemClicked
     * 节点点击
     * @param current
     */
    void onItemClicked(const QModelIndex &current);

    /**
     * @brief 字体变化
     * @param font
     */
    void onFontChanged(const QFont &font);

private:
    /**
     * @brief parseCatalogData
     * 解析文档目录数据
     */
    void parseCatalogData(const deepin_reader::Section &, QStandardItem *);

    /**
     * @brief resizeCoulumnWidth
     * 重置控件列宽度
     */
    void resizeCoulumnWidth();

    /**
     * @brief getItemList
     * 创建节点
     * @param realleft
     * @param realtop
     * @return
     */
    QList<QStandardItem *> getItemList(const QString &, const int &, const qreal  &realleft, const qreal &realtop);

    /**
     * @brief scrollToIndex
     * 滚动到指定页数
     * @param newIndex
     */
    void scrollToIndex(const QModelIndex &newIndex);

private slots:
    /**
     * @brief SlotCollapsed
     * 收缩
     */
    void slotCollapsed(const QModelIndex &);

    /**
     * @brief SlotExpanded
     * 展开
     */
    void slotExpanded(const QModelIndex &);

private:
    int m_index = -1;
    QString m_title;
    bool rightnotifypagechanged = false;
    DocSheet *m_sheet = nullptr;
};

#endif // CATALOGTREEVIEW_H
