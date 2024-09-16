// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEVICETABLEPAGE_H
#define DEVICETABLEPAGE_H

#include <QObject>
#include <DWidget>

class TableWidget;

using namespace Dtk::Widget;

/**
 * @brief The PageTableHeader class
 * UI 表头的封装
 */
class PageTableHeader : public DWidget
{
    Q_OBJECT
public:
    explicit PageTableHeader(QWidget *parent = nullptr);
    ~PageTableHeader();

    /**
     * @brief updateTable:更新表格
     * @param lst : 表格内容
     */
    void updateTable(const QList<QStringList> &lst, const QList<QStringList>& lstMenuControl, bool resizeTable = false, int step = 0);

    /**
     * @brief setColumnAverage:设置每列等宽
     */
    void setColumnAverage();

signals:
    /**
     * @brief itemClicked:点击item发出信号
     * @param row : 点击某行的行号
     */
    void itemClicked(int row);

    /**
     * @brief refreshInfo:刷新信息发出信号
     */
    void refreshInfo();

    /**
     * @brief exportInfo:导出信息发出信号
     */
    void exportInfo();

    /**
     * @brief enableDevice 判断设备是否可以启用
     * @param row 行号
     * @param enable 禁用/启用
     */
    void enableDevice(int row, bool enable);
    void uninstallDriver(int row);
    void installDriver(int row);
    void wakeupMachine(int row, bool wakeup);

    /**
     * @brief signalCheckPrinterStatus 判断dde-printer是否被安装
     */
    void signalCheckPrinterStatus(int row, bool &isPrinter, bool &isInstalled);

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    /**
     * @brief initWidgets : 初始化界面布局
     */
    void initWidgets();

private:
    TableWidget               *mp_Table;
};

#endif // DEVICETABLEPAGE_H
