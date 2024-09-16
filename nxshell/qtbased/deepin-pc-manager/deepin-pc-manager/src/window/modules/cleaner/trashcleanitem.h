// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "src/window/namespace.h"

#include <QList>
#include <QObject>

// 控制向主界面通知文件扫描的间隔，同步主界面上的进度条与文件名
// 每50个文件同步一次
#define PROGRESS_NOTICE_INTERVAL 50

class CleanerResultItemWidget;

// 一组被归类的，待扫描和清理的文件
class TrashCleanItem : public QObject
{
    Q_OBJECT

public:
    // 将文件大小由BYTE转换为KB MB GB，最多保留两位小数
    static QString fileSizeToString(quint64 filesize);

public:
    // parent为空时作为根结点
    explicit TrashCleanItem(TrashCleanItem *parentItem = nullptr);
    ~TrashCleanItem();

    void setSelected(bool);
    void setConfigToScan();

    // 检查项名称与提示
    void setTitle(const QString &);
    void setTip(const QString &);
    // 本项检查需要扫描的文件路径
    void setFilePaths(const QList<QString> &);

    void setPkgName(const QString &);

    // 子检查项操作
    // 增加一个子项
    void addChild(TrashCleanItem *);
    // 删除指定子项
    void removeChild(TrashCleanItem *);
    // 删除所有子项
    void removeChildren();

    // 接受界面信号，重新计算大小与数目
    void recountItems(quint64 *size, uint *account);
    void recount();
    void recountRemainItems(quint64 *size, uint *amount);

    void setZeroSizeForStoppedItem();
    void setItemUncheckable();

    // 由根结点通知主界面，触发主界面刷新显示扫描的文件名和进度条
    void noticeUIFileName(const QString &);
    // 由根结点通知主界面，后台服务删除指定文件
    void NoticeUIRemoveFiles(const QStringList &);
    // 由根结点通知主界面，删除已清理的卸载应用数据库信息
    void NoticeUICleanedApp(const QString &);

    // 用于展示结果和控制的界面
    inline CleanerResultItemWidget *itemWidget() const { return m_itemWidget; }

    inline TrashCleanItem *parent() const { return m_parent; }

    // 本项下，包括所有子项的文件数量与大小
    inline quint64 totalFileSize() const { return m_totalSize; }

    inline uint totalFileAmount() const { return m_totalAmount; }

    // 本项下，不包括子项的文件数量与大小
    inline quint64 itemFileSize() const { return m_itemsize; }

    inline uint itemFileAmount() const { return m_itemAmount; }

    // 检查项标题，用于拼凑显示检查文件路径
    inline QString title() const { return m_title; }

    // 不保存成员变量，直接由方法调用获得状态
    inline bool isSelected() { return m_isChecked; }

    inline uint isShown() const { return m_isShown; }

    inline quint64 cleanedSize() const { return m_cleanedSize; }

    inline int cleanedAmount() const { return m_cleanedAmount; }

public slots:
    // 主界面通知，准备开始扫描，清空所有已经存储的数据
    void prepare();
    // 主界面通知， 开始实际的扫描工作并记录相关数据
    void startScan();
    // 主界面通知，各根扫描项开始展示扫描结果
    void showScanResult();
    // 主界面通知，中止扫描结果展示
    void stopShowResult();
    void setChildSelected(bool);
    void stageChanged(bool);
    // 清理文件
    void clean();

signals:
    // 根结点被选中，将所有子结点设置为选中状态
    void rootSelected(bool);
    void scanFinished();
    void showScanResultFinished();
    void noticeFileName(const QString &);
    void noticeAppName(const QString &);
    void recounted();
    // 控制展示界面变化状态
    void scanStarted(bool);
    void prepareWidget();
    // 将扫描结果写到展示界面上
    void setScanResult(uint, QString);
    void cleanFinished();
    void deleteSystemFiles(QStringList);
    void deleteUserFiles(const QStringList &);
    void setWidgetItemSelected(bool);
    void setCleanDone();
    void setUncheckable();

private:
    void initItemWidget();
    void scanFile(const QString &path);
    TrashCleanItem *findRootItem();
    void recountItems(qint64 size, uint account);
    void scan();
    void resetData();
    void increaseCleanedItemInfo(quint64, int);

private:
    QString m_title;                       // 显示在树上的检查标题
    QString m_tip;                         // 显示在树上的提示信息
    QString m_pkgName;                     // 应用包名，清理卸载时删除数据库记录
    bool m_isToStop;                       // 停止检查信号
    bool m_isShown;                        // 该项已经在界面上显示
    quint64 m_totalSize;                   // 本项检查记录的总文件大小
    uint m_totalAmount;                    // 本项检查记录的总数，包括子项检查
    quint64 m_itemsize;                    // 本项检查记录的单项文件大小
    uint m_itemAmount;                     // 不包括子项检查的文件数
    quint64 m_cleanedSize;                 // 已清理文件的大小
    int m_cleanedAmount;                   // 已清理的文件数理
    QString m_itemName;                    // 本项检查的名称
    QStringList m_scanPaths;               // 依照类型分类检查，不需要记录具体文件
    TrashCleanItem *m_parent;              // 父检查项，没有设为nullPtr
    QList<TrashCleanItem *> m_childItems;  // 下一级检查项
    CleanerResultItemWidget *m_itemWidget; // 对应的UI
    QStringList m_fileList;                // 记录扫描过的文件名
    QStringList m_deletePaths;             // 记录待删除和文件名
    TrashCleanItem *m_root;                // 根结点位置
    bool m_isChecked;                      // 本项是否被选中
    bool m_isAnyChildSelected;             // 检查根项下，是否某个子项被选中
    bool m_unCheckable; // 某些项因为大小为0或已被清理,不再响应选中
};
