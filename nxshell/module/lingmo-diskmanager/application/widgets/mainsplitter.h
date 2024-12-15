// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#ifndef MAINSPLITTER_H
#define MAINSPLITTER_H
#include <DSplitter>

DWIDGET_USE_NAMESPACE

class DeviceListWidget;
class InfoShowWidget;

/**
 * @class MainSplitter
 * @brief 主分束器，将页面进行左右布局显示
 */

class MainSplitter : public DSplitter
{
    Q_OBJECT
    Q_DISABLE_COPY(MainSplitter)
public:
    explicit MainSplitter(DWidget *parent = nullptr);

private:

    /**
     * @brief 初始化页面
     */
    void initui();

    /**
     * @brief 初始化连接
     */
    void initConnection();

private:
    InfoShowWidget *m_infoShow; // 分区信息展示窗口
    DeviceListWidget *m_deviceList; // 设备树结构窗口
};

#endif // MAINSPLITTER_H
