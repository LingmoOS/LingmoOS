// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MAINTABBAR_H
#define MAINTABBAR_H

#include <DTabBar>
#include <DTabWidget>
#include <QDragEnterEvent>

DWIDGET_USE_NAMESPACE

class CentralDocPage;
class DocSheet;

/**
 * @brief The DocTabBar class
 * 窗体的标签页管理控件
 */
class DocTabBar : public DTabBar
{
    Q_OBJECT
    Q_DISABLE_COPY(DocTabBar)

public:
    explicit DocTabBar(QWidget *parent);

    /**
     * @brief indexOfFilePath
     * 根据路径返回该路径对应的tab索引值
     * @param filePath 文件路径
     * @return 索引值
     */
    int indexOfFilePath(const QString &filePath);

    /**
     * @brief insertSheet
     * 插入一个sheet，添加对应的tab
     * @param sheet
     * @param index -1时添加到末尾
     */
    void insertSheet(DocSheet *sheet, int index = -1);

    /**
     * @brief removeSheet
     * 移除一个sheet对应的tab
     * @param sheet
     */
    void removeSheet(DocSheet *sheet);

    /**
     * @brief showSheet
     * 显示一个sheet对应的tab
     * @param sheet
     */
    void showSheet(DocSheet *sheet);

    /**
     * @brief getSheets
     * 获取包含的所有的sheet(会按打开的倒序排列)
     * @return
     */
    QList<DocSheet *> getSheets();

    /**
     * @brief updateTabWidth
     * 更新每个tab的宽度，使得一样大并填满，并有最小值超过了会出现翻页按钮
     */
    void updateTabWidth();

signals:
    /**
     * @brief sigTabMoveIn
     * 移入sheet通知
     * @param sheet
     */
    void sigTabMoveIn(DocSheet *sheet);

    /**
     * @brief sigTabClosed
     * 关闭sheet通知
     * @param sheet
     */
    void sigTabClosed(DocSheet *sheet);

    /**
     * @brief sigTabMoveOut
     * 移出sheet通知
     * @param sheet
     */
    void sigTabMoveOut(DocSheet *sheet);

    /**
     * @brief sigTabNewWindow
     * 移出窗体创建新窗口通知
     * @param sheet
     */
    void sigTabNewWindow(DocSheet *sheet);

    /**
     * @brief sigTabChanged
     * 当前tab被切换通知
     * @param sheet
     */
    void sigTabChanged(DocSheet *sheet);

    /**
     * @brief sigNeedOpenFilesExec
     * 请求打开文件选择对话框
     */
    void sigNeedOpenFilesExec();

    /**
     * @brief sigNeedActivateWindow
     * 请求活跃当前窗体
     */
    void sigNeedActivateWindow();

private slots:
    /**
     * @brief onDragActionChanged
     * 拖拽处理
     * @param action
     */
    void onDragActionChanged(Qt::DropAction action);

    /**
     * @brief onTabReleased
     * 当tab从bar移除释放
     * @param index
     */
    void onTabReleased(int index);

    /**
     * @brief onTabDroped
     * 当tab添加到其他的bar里释放
     * @param index
     * @param da
     * @param target 目标
     */
    void onTabDroped(int index, Qt::DropAction da, QObject *target);

    /**
     * @brief onSetCurrentIndex
     * 插入后延时设置当前
     * @param index
     */
    void onSetCurrentIndex();

    /**
     * @brief onTabChanged
     * 当前tab改变
     * @param index
     */
    void onTabChanged(int);

    /**
     * @brief onTabCloseRequested
     * 关闭哪个文档
     * @param index
     */
    void onTabCloseRequested(int index);

protected:
    /**
     * @brief createMimeDataFromTab
     * 移动前获取标签需要被传出的数据
     * @param index
     * @param option
     */
    QMimeData *createMimeDataFromTab(int index, const QStyleOptionTab &option) const override;

    /**
     * @brief canInsertFromMimeData
     * 查看是否允许被移入
     * @param index
     * @param source
     * @return
     */
    bool canInsertFromMimeData(int index, const QMimeData *source) const override;

    /**
     * @brief insertFromMimeDataOnDragEnter
     * 只是先生成一个tab,结束后自动删除
     * @param index
     * @param source
     */
    void insertFromMimeDataOnDragEnter(int index, const QMimeData *source) override;

    /**
     * @brief insertFromMimeData
     * 最终完全DROP 需要添加tab并打开对应的文档
     * @param index
     * @param source
     */
    void insertFromMimeData(int index, const QMimeData *source) override;

    void dragEnterEvent(QDragEnterEvent *event) override;

    void resizeEvent(QResizeEvent *e) override;

private:
    int m_delayIndex = -1;
    QTimer *m_intervalTimer = nullptr;
};

#endif // MAINTABWIDGET_H
