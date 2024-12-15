// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PAGEDRIVERMANAGER_H
#define PAGEDRIVERMANAGER_H

#include "MacroDefinition.h"
#include "driveritem.h"
#include "DriverBackupThread.h"

#include <DWidget>
#include <DFrame>
#include <DLabel>
#include <DStackedWidget>
#include <DDialog>

#include <QObject>
#include <QThread>

class PageDriverTableView;
class DetectedStatusWidget;
class DriverScanWidget;
class QHBoxLayout;
class DriverScanner;
class DriverWidget;
class PageListView;
class PageDriverBackupInfo;
class PageDriverRestoreInfo;
class PageDriverInstallInfo;

//class DFrame;

DWIDGET_USE_NAMESPACE

class PageDriverManager : public DWidget
{
    Q_OBJECT
public:
    explicit PageDriverManager(DWidget *parent = nullptr);
    virtual ~PageDriverManager();

    /**
     * @brief addDriverInfo
     * @param info
     */
    void addDriverInfo(DriverInfo *info);

    /**
     * @brief hasScanned 是否是第一次扫描
     * @return
     */
    bool isFirstScan();

    /**
     * @brief isInstalling：判断当前是否是安装界面
     * @return
     */
    bool isInstalling();

    /**
     * @brief isBackingup：是否正在备份
     */
    bool isBackingup();

    /**
     * @brief isRestoring：是否正在还原
     */
    bool isRestoring();

    /**
     * @brief isScanning：判断当前是否扫描
     * @return
     */
    bool isScanning();

    /**
     * @brief updateListView:更新ListView
     * @param lst:
     */
    void updateListView(const QList<QPair<QString, QString> > &lst);

public slots:

    /**
     * @brief scanDriverInfo 开始扫描驱动
     */
    void scanDriverInfo();

private slots:
    /**
     * @brief slotDriverOperationClicked 处理点击信号安装更新操作处理
     */
    void slotDriverOperationClicked(int index, int itemIndex, DriverOperationItem::Mode mode);

    /**
     * @brief slotItemCheckedClicked 处理选中按钮
     * @param index
     * @param checked
     */
    void slotItemCheckedClicked(int index, bool checked);

    /**
     * @brief slotBackupItemCheckedClicked 处理选中按钮
     * @param index
     * @param checked
     */
    void slotBackupItemCheckedClicked(int index, bool checked);

    /**
     * @brief slotDownloadProgressChanged 驱动下载时回调，返回驱动下载进度、速度、已下载大小信息
     */
    void slotDownloadProgressChanged(QStringList msg);

    /**
     * @brief slotDownloadFinished 驱动下载完成
     */
    void slotDownloadFinished();

    /**
     * @brief slotInstallProgressChanged 驱动安装时回调，返回安装进度信息
     */
    void slotInstallProgressChanged(int progress);

    /**
     * @brief slotInstallProgressFinished 驱动安装完成回调，返回错误信息
     */
    void slotInstallProgressFinished(bool bsuccess, int err);

    /**
     * @brief slotInstallAllDrivers
     */
    void slotInstallAllDrivers();

    /**
     * @brief slotBackupAllDrivers 安装所有驱动
     */
    void slotBackupAllDrivers();

    /**
     * @brief slotScanFinished 扫描结束
     */
    void slotScanFinished(ScanResult sr);

    /**
     * @brief slotUndoInstall 取消安装
     */
    void slotUndoInstall();

    /**
     * @brief slotUndoBackup 取消备份
     */
    void slotUndoBackup();

    /**
     * @brief slotListViewWidgetItemClicked:ListView item点击槽函数
     * @param itemStr:item显示字符串
     */
    void slotListViewWidgetItemClicked(const QString &itemStr);

    /**
     * @brief slotBackupFinished 备份结束
     */
    void slotBackupProgressChanged(int progress);

    /**
     * @brief slotBackupFinished 备份结束
     */
    void slotBackupFinished(bool bsuccess);

    /**
     * @brief slotRestoreProgress 还原进度刷新
     */
    void slotRestoreProgress(int progress, QString strDeatils);

    /**
     * @brief slotRestoreFinished 还原结束
     */
    void slotRestoreFinished(bool success, QString msg);

signals:
    void startScanning();
    void itemClicked(const QString &itemStr);
    void scanInfo(const QString &info, int progress);
    void scanFinished(ScanResult sr);
    void sigBackupFinished(bool bsuccess);

private:
    /**
     * @brief initWidget 初始化界面
     */
    void initWidget();

    /**
     * @brief installNextDriver
     */
    void installNextDriver();

    /**
     * @brief backupNextDriver 备份下个驱动
     */
    void backupNextDriver();

    /**
     * @brief scanDevices 从硬件信息中扫描信息
     */
    void scanDevices();

    /**
     * @brief testScanDevices 测试用
     */
    void testScanDevices();

    /**
     * @brief scanDevices 扫描某个类型的硬件的驱动信息
     * @param deviceType 设备类型
     */
    void scanDevicesInfo(const QString &deviceType, DriverType driverType);

    /**
     * @brief clearAllData 清理数据
     */
    void clearAllData();

    /**
     * @brief addToDriverIndex
     * @param index
     */
    void addToDriverIndex(int index);

    /**
     * @brief addToBackupIndex 添加到备份列表
     * @param index
     */
    void addToBackupIndex(int index);

    /**
     * @brief removeFromDriverIndex
     * @param index
     */
    void removeFromDriverIndex(int index);

    /**
     * @brief removeFromBackupIndex 从备份列表移除
     * @param index
     */
    void removeFromBackupIndex(int index);

    /**
     * @brief failAllIndex
     */
    void failAllIndex();

    /**
     * @brief networkIsOnline
     * @return
     */
    bool networkIsOnline();

    /**
     * @brief getDownloadInfo
     * @param porgress
     * @param speed
     * @param size
     */
    void getDownloadInfo(int porgress, qint64 total, QString &speed, QString &size);

    void testDevices();

    /**
     * @brief getDebBackupInfo 获取备份信息
     */
    void getDebBackupInfo(DriverInfo *info);

private:
    DStackedWidget        *mp_StackWidget;
    PageListView          *mp_ListView;          // 左边的list
    QList<DriverInfo *>   m_ListDriverInfo;  // 保存所有驱动信息
    DriverInfo            *mp_CurDriverInfo;    //当前正在安装的驱动
    DriverInfo            *mp_CurBackupDriverInfo;    //当前正在备份的驱动
    DriverInfo            *mp_CurRestoreDriverInfo;    //当前正在还原的驱动
    int                   m_CurIndex;
    int                   m_CurBackupIndex;
    int                   m_CancelIndex;
    QList<int>            m_ListDriverIndex;    //已勾选驱动需要安装驱动
    QList<int>            m_ListInstallIndex;
    QList<int>            m_ListUpdateIndex;
    QList<int>            m_ListNewIndex;
    QList<int>            m_ListBackupIndex;    //已勾选需要备份驱动
    DriverScanner         *mp_scanner;     // 扫描驱动线程
    bool                  m_IsFirstScan = true;
    bool                  m_Scanning = false;
    QString               m_CurItemStr;          //<! 当前Item内容

    PageDriverInstallInfo *mp_DriverInstallInfoPage;
    PageDriverBackupInfo  *mp_DriverBackupInfoPage;
    PageDriverRestoreInfo *mp_DriverRestoreInfoPage;

    QList<int>            m_ListBackableIndex;    // 可备份列表
    QList<int>            m_ListBackedupeIndex;   // 已备份列表
    QList<int>            m_ListRestorableIndex;  // 可还原列表

    int                   m_backupSuccessNum = 0;
    int                   m_backupFailedNum = 0;

    DDialog               *mp_FailedDialog;     //还原失败弹窗

    DriverBackupThread    *mp_BackupThread;    //备份线程
};

#endif // PAGEDRIVERMANAGER_H
