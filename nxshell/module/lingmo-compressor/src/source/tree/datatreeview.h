// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DATATREEVIEW_H
#define DATATREEVIEW_H

#include "uistruct.h"

#include <DTreeView>

#include <QStyledItemDelegate>
#include <QDir>
#include <QTime>

DWIDGET_USE_NAMESPACE

class DataModel;
class TreeHeaderView;

// treeview代理，用来设置高度和去掉部分样式
class StyleTreeViewDelegate : public QStyledItemDelegate
{
public:
    explicit StyleTreeViewDelegate(QObject *parent = nullptr);
    ~StyleTreeViewDelegate() override;

    // QAbstractItemDelegate interface
public:
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    // QAbstractItemDelegate interface
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
};

// 自定义树形列表
class DataTreeView : public DTreeView
{
    Q_OBJECT
public:
    explicit DataTreeView(QWidget *parent = nullptr);
    ~DataTreeView() override;

    /**
     * @brief resetLevel    重置根节点
     */
    void resetLevel();

    TreeHeaderView *getHeaderView() const;

protected:
    void drawRow(QPainter *painter, const QStyleOptionViewItem &options, const QModelIndex &index) const override;
    void focusInEvent(QFocusEvent *event) override;

    /**
     * @brief dragEnterEvent    拖拽进入
     */
    void dragEnterEvent(QDragEnterEvent *) override;

    /**
     * @brief dragMoveEvent     拖拽移动
     */
    void dragMoveEvent(QDragMoveEvent *) override;

    /**
     * @brief dropEvent 拖拽放下
     */
    void dropEvent(QDropEvent *) override;

    void resizeEvent(QResizeEvent *event) override;

    bool event(QEvent *event) override;

    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    /**
     * @brief keyPressEvent 键盘事件 delete、enter、Alt+M
     * @param event
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief setPreLblVisible  设置返回上一级是否可见
     * @param bVisible          是否可见标志
     * @param strPath            上一级路径
     */
    void setPreLblVisible(bool bVisible, const QString &strPath = "");

private:
    /**
     * @brief initUI    初始化界面
     */
    void initUI();

    /**
     * @brief initConnections   初始化信号槽
     */
    void initConnections();

protected:
    /**
     * @brief resizeColumnWidth 重置列宽度
     */
    void resizeColumnWidth();

Q_SIGNALS:
    /**
     * @brief signalDragFiles   外部文件拖拽至列表信号
     * @param listFiles         外部拖拽文件
     */
    void signalDragFiles(const QStringList &listFiles);

protected Q_SLOTS:
    /**
     * @brief slotPreClicked    返回上一级
     */
    virtual void slotPreClicked() = 0;
    /**
     * @brief slotShowRightMenu     显示右键菜单
     * @param pos   右键位置
     */
    virtual void slotShowRightMenu(const QPoint &pos) = 0;
    /**
     * @brief handleDoubleClick    处理双击事件
     * @param index 双击的index
     */
    virtual void handleDoubleClick(const QModelIndex &index) = 0;
    /**
     * @brief slotDeleteFile    删除待压缩文件
     */
    virtual void slotDeleteFile() = 0;
    /**
     * @brief slotRenameFile    重命名待压缩文件
     */
    virtual void slotRenameFile() = 0;

protected:

    DataModel *m_pModel;

    int m_iLevel = 0;       // 目录层级
    QString m_strCurrentPath = QDir::separator();   // 当前目录

    TreeHeaderView *m_pHeaderView; // 表头
    QVector<QString> m_vPre; // 记录上层文件夹名，用于返回上一级焦点设置
    QPointF m_lastTouchBeginPos; // 触摸屏点击位置
    bool m_isPressed = false; // 触摸按下标志  true: 按下; false: 松开
    QTime m_lastTouchTime;

private:
    Qt::FocusReason m_reson;
    QItemSelectionModel *m_selectionModel =  nullptr;

};

#endif // DATATREEVIEW_H
