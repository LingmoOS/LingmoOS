// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef PARTITIONTABLEERRORSINFODIALOG_H
#define PARTITIONTABLEERRORSINFODIALOG_H

#include <DDialog>
#include <DLabel>
#include <DTableView>

#include <QWidget>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class QStandardItemModel;
class PartitionTableErrorsInfoDelegate;
class DiskHealthHeaderView;

/**
 * @class PartitionTableErrorsInfoDialog
 * @brief 分区表错误报告类
 */

class PartitionTableErrorsInfoDialog : public DDialog
{
    Q_OBJECT
public:
    explicit PartitionTableErrorsInfoDialog(const QString &deviceInfo, QWidget *parent = nullptr);
    ~PartitionTableErrorsInfoDialog();

signals:

public slots:

protected:
    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private:
    /**
     * @brief 初始化界面
     */
    void initUI();

    /**
     * @brief 初始化信号连接
     */
    void initConnections();

private:
    DLabel *m_Label; // 磁盘信息提示
    QString m_deviceInfo; // 当前磁盘
    DTableView *m_tableView; // 表格
    QStandardItemModel *m_standardItemModel; // 表格模型
    PartitionTableErrorsInfoDelegate *m_partitionTableErrorsInfoDelegatee; // 表格代理
    DPushButton *pushButton; // 确定按钮
    DiskHealthHeaderView *m_diskHealthHeaderView;
};

#endif // PARTITIONTABLEERRORSINFODIALOG_H
