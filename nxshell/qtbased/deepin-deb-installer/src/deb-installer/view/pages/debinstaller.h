// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DEBINSTALLER_H
#define DEBINSTALLER_H

#include "utils/package_defines.h"
#include "model/packageselectmodel.h"
#include "view/pages/backendprocesspage.h"

#include <DMainWindow>

#include <QPointer>
#include <QSettings>
#include <QStackedLayout>
#include <QApt/DebFile>
#include <QWidget>
#include <QFileInfoList>

class FileChooseWidget;
class AbstractPackageListModel;
class DebListModel;
class SingleInstallPage;
class UninstallConfirmPage;
class SettingDialog;
class PackageSelectModel;
class PackageSelectView;
class DdimErrorPage;
class SelectInstallPage;

using QApt::DebFile;

/**
 * @brief The DebInstaller class
 *
 * 软件包安装器的主窗口
 * 承担各安装界面的切换，包的添加，删除等功能
 * M-V-C 中的 View
 */
class DebInstaller : public Dtk::Widget::DMainWindow
{
    Q_OBJECT

public:
    DebInstaller(QWidget *parent = nullptr);
    virtual ~DebInstaller() Q_DECL_OVERRIDE;

signals:
    void runOldProcess(const QStringList &paths);

protected:
    void dragEnterEvent(QDragEnterEvent *e) Q_DECL_OVERRIDE;  // 拖入事件
    void dropEvent(QDropEvent *e) Q_DECL_OVERRIDE;            // 拖入放下事件
    void dragMoveEvent(QDragMoveEvent *e) Q_DECL_OVERRIDE;    // 拖进事件
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;      // 关闭事件

private slots:
    /**
     * @brief slotPackagesSelected
     * @param packages 安装的包的全路径的列表
     * 添加包时，对包进行处理，去除无效的包，提示已经添加过的包，并根据添加包的数量刷新界面
     */
    void slotPackagesSelected(const QStringList &packages);

    /**
     * @brief slotDdimSelected
     * @param ddimFiles 清单文件
     * 根据清单文件进行解析操作
     */
    void slotDdimSelected(const QStringList &ddimFiles);

    // Append package failed, show floating message based on diff error.
    void slotReceiveAppendFailed(Pkg::AppendFailReason reason);

    /**
     * @brief slotShowInvalidePackageMessage 弹出无效包的消息通知
     */
    void slotShowInvalidePackageMessage();

    /**
     * @brief slotShowNotLocalPackageMessage 弹出不是本地包的消息通知
     */
    void slotShowNotLocalPackageMessage();

    /**
     * @brief slotShowNotInstallablePackageMessage 弹出无安装权限的消息通知
     */
    void slotShowNotInstallablePackageMessage();

    /**
     * @brief slotShowPkgExistMessage 弹出包已存在的消息通知
     */
    void slotShowPkgExistMessage();

    /**
     * @brief slotShowPkgRemovedMessage 弹出包被移动的消息通知
     */
    void slotShowPkgRemovedMessage(QString packageName);

    /**
     * @brief slotShowDdimErrorMessage 处理DDIM相关的错误信息，并跳转至对应界面
     */
    void slotShowDdimErrorMessage(const QString &message);

    /**
     * @brief slotShowDdimFloatingMessage 处理DDIM相关的错误信息，并弹出浮动提示框
     */
    void slotShowDdimFloatingMessage(const QString &message);

    /**
     * @brief slotShowUninstallConfirmPage
     * 卸载按钮的槽函数
     * 显示卸载界面
     */
    void slotShowUninstallConfirmPage();

    /**
     * @brief slotUninstallAccepted
     * 卸载界面确认卸载按钮的槽函数
     * 卸载开始时，返回singleInstallPage 并显示卸载进程。
     */
    void slotUninstallAccepted();

    /**
     * @brief slotUninstallCancel
     * 卸载界面取消卸载按钮的槽函数
     * 取消卸载后返回 singleInstallPage
     */
    void slotUninstallCancel();

    /**
     * @brief slotSetEnableButton
     * @param bEnable 按钮是否可用标志
     * 根据当前的安装/卸载进程来控制singleInstallPage/multiInstallPage按钮的可用性
     */
    void slotSetEnableButton(bool bEnable);

    /**
     * @brief slotShowHiddenButton
     * 授权取消后显示被隐藏的按钮
     */
    void slotShowHiddenButton();

    /**
     * @brief slotSetAuthingStatus
     * @param authing 按钮是否可用的标志
     * 授权框弹出后，设置当前界面为不可用状态
     */
    void slotSetAuthingStatus(const bool authing);

    /**
     * @brief slotDealDependResult
     * @param iAuthRes
     * 根据deepin-wine依赖安装的结果处理界面显示效果
     */
    void slotDealDependResult(int iAuthRes, QString dependName);

    /**
     * @brief slotChangeDragFlag
     * 安装卸载结束后，允许包被拖入程序，并设置关闭按钮可用
     */
    void slotChangeDragFlag();

    /**
     * @brief slotReset
     * 重置当前工作状态、拖入状态、标题栏、页面暂存区，删除卸载页面
     *
     */
    void slotReset();

    /**
     * @brief slotEnableCloseButton
     * @param enable 是否禁用关闭按钮
     * 下载依赖时，关闭按钮不可用，下载依赖成功后，依赖按钮可用
     */
    void slotEnableCloseButton(bool enable);

    /**
     * @brief slotSettingDialogVisiable
     * 显示设置框
     */
    void slotSettingDialogVisiable();

    // TODO(renbin): DBus interface, refactor later
#if 0
    /**
     * @brief PackagesSelected
     * 添加包，但是不显示主窗口
     */
    void PackagesSelected(const QStringList &debPathList);

    /**
     * @brief startInstallPackge
     * 开始安装，包并返回安装信息
     */
    QString startInstallPackge(const QString &debPath);
    /**
     * @brief startUnInstallPackge
     * 卸载包，包并返回卸载信息
     */
    QString startUnInstallPackge(const QString &debPath);

    /**
     * @brief checkInstallStatus
     * 查找包的安装状态
     */
    int checkInstallStatus(const QString &debPath);
    /**
     * @brief checkDependsStatus
     * 查找包的依赖状态
     */
    int checkDependsStatus(const QString &debPath);
    /**
     * @brief checkDigitalSignature
     * 查找包的数字签名
     */
    int checkDigitalSignature(const QString &debPath);
    /**
     * @brief getPackageInfo
     * 查找包信息,返回(包名，包的路径，包的版本，包可用的架构，包的短描述，包的长描述)
     */
    QString getPackageInfo(const QString &debPath);
#endif

    /**
     * @brief slotShowSelectPage
     * @param selectedInfos 用于显示的数据
     * 显示选择页面
     */
    void slotShowSelectPage(const QList<DebIr> &selectedInfos);

    /**
     * @brief slotShowSelectInstallPage
     * @param selectIndexes 对应模型上的索引编号
     * 显示选择页面
     */
    void slotShowSelectInstallPage(const QList<int> &selectIndexes);

    /**
     * @brief slotShowPkgProcessBlockPage
     * @param mode        阻塞模式，当为PROCESS_FIN的时候退出
     * @param currentRate 当前进度值（某些mode下不会生效）
     * @param pkgCount    总进度值（某些mode下不会生效）
     * 显示包处理阻塞界面
     */
    void slotShowPkgProcessBlockPage(BackendProcessPage::DisplayMode mode, int currentRate, int pkgCount);

private:
    /**
     * @brief initUI
     * 初始化界面
     */
    void initUI();

    /**
     * @brief initTitleBar
     * 初始化标题栏
     */
    void initTitleBar();

    /**
     * @brief initConnections
     * 初始化链接信号和槽
     */
    void initConnections();

    /**
     * @brief refreshSingle 刷新单包安装界面
     */
    void refreshSingle();

    /**
     * @brief single2Multi 刷新批量安装界面
     */
    void single2Multi();

    /**
     * @brief refreshMulti 刷新批量安装model
     */
    void refreshMulti();

    /**
     * @brief appendPackageStart 正在添加多个包的界面处理函数
     */
    void appendPackageStart();

    /**
     * @brief appendFinished 批量添加结束界面处理函数
     */
    void appendFinished();

    /**
     * @brief MulRefreshPage 刷新批量安装model
     */
    void MulRefreshPage();

    // Disable/enable close button and exit in menu
    /**
     * @brief disableCloseAndExit
     * 设置退出和关闭按钮为不可用
     */
    void disableCloseAndExit();
    /**
     * @brief enableCloseAndExit
     * 设置退出和关闭按钮可用
     */
    void enableCloseAndExit();

    /**
     * @brief backToSinglePage 返回单包安装界面
     * @return SingleInstallPage* SingleInstallPage的指针
     */
    SingleInstallPage *backToSinglePage();

    /**
     * @brief checkSuffix 检查文件后缀
     * @param filePath 文件路径
     * @return 文件后缀是否是.deb
     */
    bool checkSuffix(QString filePath);

    /**
     * @brief analyzeV10
     * @param ddimobj 清单文件内部的JSON内容
     * 根据JSON进行解析操作，仅处理1.0版本
     */
    DdimSt analyzeV10(const QJsonObject &ddimobj, const QString &ddimDir);

private:
    AbstractPackageListModel *m_fileListModel = nullptr;         // model 类
    FileChooseWidget *m_fileChooseWidget = nullptr;  // 文件选择的widget
    UninstallConfirmPage *m_uninstallPage = nullptr;

    QPointer<QWidget> m_lastPage;               // 存放上一个页面的指针
    QStackedLayout *m_centralLayout = nullptr;  // 单包、批量、卸载的widget
    SettingDialog *m_settingDialog = nullptr;

    PackageSelectModel *m_ddimModel = nullptr;           // ddim处理model
    PackageSelectView *m_ddimView = nullptr;             // ddim处理view
    DdimErrorPage *m_ddimErrorPage = nullptr;            // ddim错误提示界面
    SelectInstallPage *m_ddimInstallPage = nullptr;      // ddim安装界面
    BackendProcessPage *m_backendProcessPage = nullptr;  // 后台处理阻塞界面
    QString lastTitle;

    int m_dragflag = -1;  // 当前是否允许拖入的标志位

    int m_iOptionWindowFlag = 0;    // 判断菜单栏是否手动弹出
    bool bTabFlag = false;          // Control focus is re-identified from titlebar
    bool bActiveWindowFlag = true;  // Window activation id

    enum CurrentPage {
        ChoosePage = -1,
        NonePage = 0,
        MultiPage = 1,
        SinglePage = 2,
        UninstallPage = 3,
    };
    CurrentPage m_Filterflag { ChoosePage };  // Determine the current page      choose:-1;multiple:1;single:2;uninstall:3

    bool m_packageAppending = false;
    int m_wineAuthStatus = -1;  // 记录依赖配置授权状态
};

#endif  // DEBINSTALLER_H
