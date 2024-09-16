// Copyright (C) 2019 ~ 2022 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TRASHCLEANRESULTHEADERITEM_H
#define TRASHCLEANRESULTHEADERITEM_H

#include <DFrame>
#include <DLabel>
#include <DTipLabel>

#include <QWidget>

class TrashCleanResultHeaderItem : public Dtk::Widget::DFrame
{
    Q_OBJECT
public:
    explicit TrashCleanResultHeaderItem(QWidget *parent = nullptr);

    ~TrashCleanResultHeaderItem() { }

public Q_SLOTS:
    void setScanStarted();
    // 扫描结束时总文件数
    void setScanStopped(const QString &totalFiles);
    // 清理结束时总文件数与大小
    void setCleanFinished(const QString &, const QString &);
    // 扫描中时间与路径变化
    void updateScanPath(const QString &);
    // 扫描中标题变化
    void updateScanTitleInfo(const QString &total, const QString &selected);

    void setTitle(const QString &);
    void setTip(const QString &);
    void setTimeInfo(const QString &);
    void setLogo(const QString &);

private:
    void initUI();

private:
    Dtk::Widget::DLabel *m_infos;
    Dtk::Widget::DTipLabel *m_actionInfo;
    Dtk::Widget::DTipLabel *m_scanTimeInfo;
    Dtk::Widget::DLabel *m_scanIcon;
};

#endif // TRASHCLEANRESULTHEADERITEM_H
