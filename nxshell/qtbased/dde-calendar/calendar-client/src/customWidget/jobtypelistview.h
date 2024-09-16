// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef JOBTYPELISTVIEW_H
#define JOBTYPELISTVIEW_H

#include "scheduledatamanage.h"
#include "daccount.h"
#include "accountitem.h"

#include <DListView>
#include <DIconButton>

#include <QWidget>
#include <QList>
#include <QTableView>
#include <DDialog>

DWIDGET_USE_NAMESPACE

class JobTypeListViewStyle;
class JobTypeListView : public QTableView
{
    Q_OBJECT
public:
    explicit JobTypeListView(QWidget *parent = nullptr);
    ~JobTypeListView();

    /**
     * @brief canAdd 是否可以继续新增类型
     */
    bool canAdd();

    void setItemEnabled(bool b);

protected:
    bool viewportEvent(QEvent *event) override;

private:
    void initUI();//初始化
    /**
     * @brief addJobTypeItem 添加item
     * @return 返回item的高度
     */
    int addJobTypeItem(const DScheduleType &info); //新增一行【日程类型】数据

signals:
    /**
     * @brief signalAddStatusChanged 信号：是否可以继续新增类型
     */
    void signalAddStatusChanged(bool);
public slots:
    void slotUpdateJobType();
    void slotDeleteJobType();
    void slotAddScheduleType();
    // 导入ics文件
    void slotImportScheduleType();
    // 导出ics文件
    void slotExportScheduleType();
    bool updateJobType();
    void updateCalendarAccount(QString account_id);

private:
    QStandardItemModel *m_modelJobType {nullptr};
    int m_iIndexCurrentHover = -1;

    friend JobTypeListViewStyle;

    QString m_account_id;
    DDialog *m_waitDialog = nullptr;
};

class JobTypeListViewStyle : public DStyledItemDelegate
{
    Q_OBJECT
public:
    explicit JobTypeListViewStyle(QAbstractItemView *parent = nullptr)
        : DStyledItemDelegate(parent) {}

protected:
    // painting
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
};

#endif // JOBTYPELISTVIEW_H
