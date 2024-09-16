// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DISKHEALTHDETECTIONDIALOG_H
#define DISKHEALTHDETECTIONDIALOG_H

#include "partedproxy/dmdbushandler.h"

#include <DDialog>
#include <DTableView>
#include <DCommandLinkButton>
#include <DLabel>

#include <QWidget>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

class QStandardItemModel;
class DiskHealthDetectionDelegate;
class DiskHealthHeaderView;

/**
 * @class DiskHealthDetectionDialog
 * @brief 磁盘健康检测类
 */

class DiskHealthDetectionDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DiskHealthDetectionDialog(const QString &devicePath, HardDiskStatusInfoList hardDiskStatusInfoList, QWidget *parent = nullptr);
    ~DiskHealthDetectionDialog();

signals:

public slots:

protected:
    /**
     * @brief event:事件变化
     * @param event事件
     * @return 布尔
     */
    bool event(QEvent *event) override;

    /**
     * @brief 键盘按下事件
     * @param event事件
     */
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    /**
     * @brief 导出按钮点击响应的槽函数
     */
    void onExportButtonClicked();

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
    DTableView *m_tableView; // 表格
    QStandardItemModel *m_standardItemModel; // 数据模型
    DCommandLinkButton *m_linkButton; // 导出按钮
    DiskHealthDetectionDelegate *m_diskHealthDetectionDelegate; // 表格代理
    QString m_devicePath; // 磁盘路径
    DLabel *m_serialNumberValue; // 序列号值
    DLabel *m_userCapacityValue;// 用户容量值
    DLabel *m_healthStateValue; // 健康状态值
    DLabel *m_temperatureValue; // 温度值
    DiskHealthHeaderView *m_diskHealthHeaderView;
    HardDiskStatusInfoList m_hardDiskStatusInfoList;

};

#endif // DISKHEALTHDETECTIONDIALOG_H
