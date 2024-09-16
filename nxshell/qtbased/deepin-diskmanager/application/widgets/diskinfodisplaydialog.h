// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef DISKINFODISPLAYDIALOG_H
#define DISKINFODISPLAYDIALOG_H

#include <DDialog>
#include <DCommandLinkButton>

#include <QWidget>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE
DTK_USE_NAMESPACE

/**
 * @class DiskInfoDisplayDialog
 * @brief 磁盘信息类
 */

class DiskInfoDisplayDialog : public DDialog
{
    Q_OBJECT
public:
    explicit DiskInfoDisplayDialog(const QString &devicePath, QWidget *parent = nullptr);

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
    QStringList m_diskInfoNameList; // 磁盘属性名称
    QStringList m_diskInfoValueList; // 磁盘属性值
    DCommandLinkButton *m_linkButton; // 导出按钮
    QString m_devicePath; // 当前磁盘路径

};

#endif // DISKINFODISPLAYDIALOG_H
