// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TEXTBROWSER_H
#define TEXTBROWSER_H

#include <QObject>
#include <QWidget>
#include <DTextBrowser>
#include <QDomDocument>
#include <DNotifySender>

#include "DeviceInfo.h"

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

class DeviceBaseInfo;
/**
 * @brief The TextBrowser class
 * 封装富文本控件
 */
class TextBrowser : public DTextBrowser
{
    Q_OBJECT
public:
    explicit TextBrowser(QWidget *parent = nullptr);

    /**
     * @brief showDeviceInfo : 显示设备信息
     * @param info ：需要显示的设备信息
     */
    void showDeviceInfo(DeviceBaseInfo *info);

    /**
     * @brief updateInfo : 点击详细按钮之后的展开和收起操作
     */
    void updateInfo();

    /**
     * @brief setDeviceEnabled
     * @param enable
     */
    EnableDeviceStatus setDeviceEnabled(bool enable);

    /**
     * @brief setWakeupMachine
     * @param wakeup
     */
    void setWakeupMachine(bool wakeup);

    /**
     * @brief updateShowOtherInfo 更新是否需要显示其它信息
     */
    void updateShowOtherInfo();


public slots:
    void fillClipboard();

protected:
    virtual void paintEvent(QPaintEvent *event)override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void focusInEvent(QFocusEvent *e) override;
    virtual void focusOutEvent(QFocusEvent *e) override;

signals:
    void refreshInfo();
    void exportInfo();
    void copyAllInfo();

private slots:
    /**
     * @brief slotShowMenu:右键菜单
     */
    void slotShowMenu(const QPoint &);

    /**
     * @brief slotCloseMenu:右键菜单关闭
     */
    void slotCloseMenu();

    /**
     * @brief slotActionRefresh:刷新
     */
    void slotActionRefresh();

    /**
     * @brief slotActionExport: 导出
     */
    void slotActionExport();

    /**
     * @brief slotActionCopy:复制
     */
    void slotActionCopy();

private:
    void domTitleInfo(QDomDocument &doc, DeviceBaseInfo *info);
    void domTableInfo(QDomDocument &doc, const QList<QPair<QString, QString>> &info);

private:
    bool                    m_ShowOtherInfo;
    DeviceBaseInfo          *mp_Info;
    QAction                 *mp_Refresh;     //<! 右键刷新
    QAction                 *mp_Export;      //<! 右键导出
    QAction                 *mp_Copy;        //<! 拷贝
    QMenu                   *mp_Menu;        //<! 右键菜单

    bool                     m_IsMenuShowing;
};

#endif // TEXTBROWSER_H
