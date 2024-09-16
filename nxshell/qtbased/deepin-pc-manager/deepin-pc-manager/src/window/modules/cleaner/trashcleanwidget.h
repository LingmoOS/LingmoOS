// Copyright (C) 2019 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "../../namespace.h"

#include <DDesktopServices>
#include <DGuiApplicationHelper>
#include <DTreeWidget>

#include <QItemDelegate>
#include <QWidget>

DWIDGET_BEGIN_NAMESPACE
class DFrame;
class DLabel;
class DSuggestButton;
class DProgressBar;
DWIDGET_END_NAMESPACE

class CheckItem;
class CleanerItem;
class TrashCleanItem;
class TitleLabel;

namespace def {
namespace widgets {
class SmallLabel;
}
} // namespace def

class QGSettings;

class CleanerDBusAdaptorInterface;

DWIDGET_USE_NAMESPACE

/* 添加树表代理 */
class MyItemDelegateTree : public QItemDelegate
{
    Q_OBJECT
public:
    explicit MyItemDelegateTree(QObject *parent = nullptr);

    virtual ~MyItemDelegateTree() { }

    // 重绘
    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
};

class TrashCleanWidget : public QWidget
{
    Q_OBJECT
public:
    /**
     * @brief ScanStages 枚举控制该控件页面的显示状态，依次为：
     * 检查功能首页
     * 扫描启动
     * 扫描完成
     * 清理启动
     * 清理完成
     */
    enum ScanStages { PREPARING, SCAN_STARTED, SCAN_FINISHED, CLEAN_STARTED, CLEAN_FINISHED };

    explicit TrashCleanWidget(QWidget *parent = nullptr,
                              CleanerDBusAdaptorInterface *interfact = nullptr);
    ~TrashCleanWidget();

    void setServerInterface(CleanerDBusAdaptorInterface *);

    inline quint64 GetScanedSize() const { return m_totalSize; }

    inline quint64 GetCleanedSize() const { return m_cleanedSize; }

    inline int GetScanedAcount() const { return m_totalScannedFiles; }

    inline int GetCleanedAcount() const { return m_cleanedAmount; }

    inline ScanStages GetCurrentStage() const { return m_stage; }

    QString getElidedScanTitle(const QString &) const;

protected:
    void paintEvent(QPaintEvent *event);

private:
    void initHeaderFrame();
    void initConfigFrame();
    void initResultFrame();
    void initBottomFrame();

    void changeStageByLeftButton();
    void changeStageByRightButton();
    void onStageChanged(ScanStages);

    void addRootCheckItems();
    void addSystemChildCheckItems();
    void addAppCheckItems();
    void addUninstalledAppCheckItems();
    void addBrowserCookies();
    void addChildrenCheckItems();
    void
    addChildItem(int rootIndex, const QString &title, const QString &tip, const QStringList &paths);
    void addChildItem(int rootIndex,
                      const QString &title,
                      const QString &pkgName,
                      const QString &tip,
                      const QList<QStringList> &paths,
                      bool isRemained = false);

    void startScan();
    void stopScan();
    void scan();

    void setWidgetPrepare();
    void setWidgetScanStart();
    void setWidgetScanFinish();
    void setWidgetCleanStart();
    void setWidgetCleanFinish();

    void getScanPathsFromDataInterface();
    void setHeadTextWithElidedText(const QString &);

Q_SIGNALS:
    void stageChanged(ScanStages);
    void prepare();
    void startWork();
    // void stopWork();
    void stopShowResult();
    void sendScanFinished();
    void showScanResult();
    void showResult();
    void startClean();

public Q_SLOTS:
    void scanDone();
    void setScanTitleName(QString);
    void showScanResultFinished();
    void recount();
    void setPixmapByTheme(Dtk::Gui::DGuiApplicationHelper::ColorType themeType);
    void onConfigItemClicked();
    void onCleanFinished();
    void DeleteSpecifiedFiles(QStringList paths);
    void DeleteSpecifiedAppRecord(const QString &pkgName);

private:
    // 界面由4个frame拼起来
    DFrame *m_headerFrame;
    DFrame *m_configFrame;
    DFrame *m_resultFrame;
    DFrame *m_bottomFrame;
    DFrame *m_bottomLine;
    // 记录界面状态
    ScanStages m_stage;

    // 使用的控件
    DPushButton *m_leftButton;
    DSuggestButton *m_rightButton;
    DLabel *m_configLogo;
    TitleLabel *m_headerTitle;
    DProgressBar *m_scanProgressBar;
    DLabel *m_tipTitle;
    DLabel *m_headerLogo;
    DTreeWidget *m_treeWidget;

    // 配置界面的选项
    QList<CleanerItem *> m_configItem;
    // 检查界面的选项
    QList<TrashCleanItem *> m_rootItems;

    // 上一次扫描的大小
    QString m_lastScannedSize;
    QString m_lastCleanedSize;

    // 记录检查完成数
    int m_doneItems;
    int m_showItems;
    // 记录进度条值
    int m_currentScanProgressValue;
    // 记录本次扫描垃圾大小
    quint64 m_totalSize;
    int m_totalScannedFiles;
    quint64 m_cleanedSize;
    int m_cleanedAmount;
    bool m_isCleaned;

    // 表格代理
    MyItemDelegateTree *m_delegate;

    // 扫描按钮 标志位
    bool m_isbScan;

    // 扫描时间展示
    DLabel *m_scanTime;
    CleanerDBusAdaptorInterface *m_dataInterface;

    // 是否扫描后首次显示结果
    bool m_isFirstShow;

    QStringList m_trashInfoList;
    QStringList m_cacheInfoLIst;
    QStringList m_logInfoList;
    QStringList m_historyInfoList;
    QString m_appJsonStr;
    QString m_cookiesJsonStr;

    QString m_scanResultStr;
};

DEF_NAMESPACE_END
