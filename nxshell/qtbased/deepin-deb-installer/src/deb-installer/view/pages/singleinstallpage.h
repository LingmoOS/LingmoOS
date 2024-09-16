// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEINSTALLPAGE_H
#define SINGLEINSTALLPAGE_H

#include "view/widgets/infocontrolbutton.h"
#include "view/widgets/installprocessinfoview.h"
#include "view/widgets/debinfolabel.h"
#include "manager/packagesmanager.h"

#include <DLabel>
#include <DProgressBar>
#include <DPushButton>
#include <DTextEdit>
#include <DSpinner>
#include <DDialog>
#include <DCommandLinkButton>

#include <QWidget>
#include <QProcess>
#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <QApt/DebFile>
#include <unistd.h>
#include <stdlib.h>

class AbstractPackageListModel;
class WorkerProgress;
class SingleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit SingleInstallPage(AbstractPackageListModel *model, QWidget *parent = nullptr);

public:
    /**
     * @brief afterGetAutherFalse 授权失败或者取消后的界面处理函数
     */
    void afterGetAutherFalse();

    /**
     * @brief setEnableButton 授权框弹出/取消后 ，设置按钮禁用/可用
     * @param bEnable
     */
    void setEnableButton(bool bEnable);

    /**
     * @brief DealDependResult 根据依赖安装的进程标识，处理依赖安装的流程显示
     * @param authStatus  依赖安装的集成标识
     * @param dependName    安装的依赖名称
     */
    void DealDependResult(int authStatus, QString dependName);

public slots:
    /**
     * @brief uninstallCurrentPackage 显示卸载页面
     */
    void slotUninstallCurrentPackage();

protected:
    /**
       @brief 每次切换展示当前页面，复位焦点状态
     */
    void showEvent(QShowEvent *e) override;
    void paintEvent(QPaintEvent *event) override;

signals:

    /**
     * @brief back 返回fileChooseWidget界面
     */
    void signalBacktoFileChooseWidget() const;

    /**
     * @brief requestUninstallConfirm 卸载按钮信号，显示卸载页面
     */
    void signalRequestUninstallConfirm() const;

private:
    /**
     * @brief The Operate enum 当前进行的操作
     */
    enum Operate {
        Unknown,
        Install,    // 安装
        Uninstall,  // 卸载
        Reinstall   // 重新安装
    };

private:
    /**
     * @brief initUI 初始化UI
     */
    void initUI();

    /**
     * @brief initContentLayout 初始化主布局
     */
    void initContentLayout();

    /**
     * @brief initInstallWineLoadingLayout 初始化wine依赖安装加载动画布局
     */
    void initInstallWineLoadingLayout();

    /**
     * @brief initPkgInfoView 初始化 包信息视图布局
     * @param fontinfosize  字体大小
     */
    void initPkgInfoView(int fontinfosize);

    /**
     * @brief initPkgInstallProcessView 初始化安装进程信息布局
     * @param fontinfosize  字体大小
     */
    void initPkgInstallProcessView(int fontinfosize);

    /**
     * @brief initPkgDependsInfoView 初始化依赖信息显示布局
     */
    void initPkgDependsInfoView();

    /**
     * @brief initConnections 初始化链接信号与槽
     */
    void initConnections();

    /**
     * @brief initLabelWidth 根据字体的大小调整标签的宽度
     * @param fontinfo 字体大小
     * @return  label的宽度
     */
    int initLabelWidth(int fontinfo);

    /**
     * @brief initControlAccessibleName 初始化控件的AccessibleName
     */
    void initControlAccessibleName();

private:
    /**
     * @brief initTabOrder 设置tab切换焦点的顺序
     */
    void initTabOrder();

    /**
     * @brief initButtonFocusPolicy 设置按钮的焦点策略
     */
    void initButtonFocusPolicy();

    /**
     * @brief initButtonAutoDefault 设置按钮的回车触发
     */
    void initButtonAutoDefault();

private:
    /**
     * @brief showPackageInfo 获取并显示deb包的信息
     */
    void showPackageInfo();

    /**
     * @brief setAuthConfirm 依赖下载授权确认后的界面处理函数
     * @param dependName
     */
    void setAuthConfirm(QString dependName);

    /**
     * @brief setAuthBefore 依赖下载授权前的界面处理函数
     */
    void setAuthBefore();

    /**
     * @brief setCancelAuthOrAuthDependsErr 授权取消或依赖安装结果显示的界面处理函数
     */
    void setCancelAuthOrAuthDependsErr();

    /**
     * @brief setAuthDependsSuccess 暂未使用
     */
    void setAuthDependsSuccess();

private slots:

    /**
     * @brief install 点击安装按钮后的界面处理，并触发安装进程
     */
    void slotInstall();

    /**
     * @brief reinstall 点击重新安装按钮后的界面处理，并触发安装进程
     */
    void slotReinstall();

    /**
     * @brief showInfomation 显示安装详细信息的视图
     */
    void slotShowInfomation();

    /**
     * @brief hideInfomation 隐藏安装过程详细信息的视图
     */
    void slotHideInfomation();

    /**
     * @brief slotShowDependsInfo 显示依赖关系的视图
     */
    void slotShowDependsInfo();

    /**
     * @brief slotHideDependsInfo  隐藏依赖关系的视图
     */
    void slotHideDependsInfo();

    /**
     * @brief showInfo 显示信息的界面调整
     */
    void slotShowInfo();

    /**
     * @brief onOutputAvailable 向安装详细信息展示窗口添加安装进度信息
     * @param output 安装过程信息
     */
    void slotOutputAvailable(const QString &output);

    /**
     * @brief onWorkerFinished 安装或卸载结束后的界面调整
     */
    void slotWorkerFinished();

    /**
     * @brief onWorkerProgressChanged 安装进度变化槽函数
     * @param progress 实际的安装进度
     */
    void slotWorkerProgressChanged(const int progress);

    /**
     * @brief slotDependPackages  缺失依赖显示
     * @param dependPackages  依赖包存储
     * @param installWineDepends 是否进入wine依赖配置
     */
    void slotDependPackages(Pkg::DependsPair dependPackages, bool installWineDepends);

    // refresh the single package depend info.
    void slotRefreshSinglePackageDepends();

private:
    Operate m_operate = Unknown;   // 当前的操作
    bool m_workerStarted = false;  // 安装是否开始
    bool m_upDown = false;         // 当前是详细信息是展开还是收缩

    AbstractPackageListModel *m_packagesModel = nullptr; //model类

    QWidget *m_contentFrame = nullptr;   // 主布局
    QWidget *m_itemInfoFrame = nullptr;  // 包信息框架
    QWidget *m_progressFrame = nullptr;  // 安装进度框架
    QWidget *m_btnsFrame = nullptr;      // 按键框架

    DLabel *m_packageIcon = nullptr;           // 包的图标
    DebInfoLabel *m_packageName = nullptr;     // 包名
    DebInfoLabel *m_packageVersion = nullptr;  // 包的版本
    DLabel *m_packageDescription = nullptr;    // 包的描述
    DebInfoLabel *m_tipsLabel = nullptr;       // 包的状态提示label

    WorkerProgress *m_progress = nullptr;                    // 安装进度条
    InstallProcessInfoView *m_installProcessView = nullptr;  // 安装进度的详细信息
    InstallProcessInfoView *m_showDependsView = nullptr;     // 依赖关系显示

    InfoControlButton *m_infoControlButton = nullptr;  // 安装信息显示/隐藏控制按钮
    InfoControlButton *m_showDependsButton = nullptr;  // 显示依赖关系按钮
    DPushButton *m_installButton = nullptr;            // 安装按钮
    DPushButton *m_uninstallButton = nullptr;          // 卸载按钮
    DPushButton *m_reinstallButton = nullptr;          // 重新安装按钮
    DPushButton *m_confirmButton = nullptr;            // 确认按钮
    DPushButton *m_backButton = nullptr;               // 返回按钮
    DPushButton *m_doneButton = nullptr;               // 完成按钮

private:
    QVBoxLayout *m_contentLayout = nullptr;
    QVBoxLayout *m_centralLayout = nullptr;  // 主布局

    QString m_description = "";         // 包的描述文本
    QString m_pkgNameDescription = "";  // 包名的文本
    QString m_versionDescription = "";  // 包版本的文本

    DSpinner *m_pDSpinner = nullptr;  // 依赖安装加载动画

    // fix bug:33999 change DebInfoLabel to DCommandLinkButton for Activity color
    DCommandLinkButton *m_pLoadingLabel = nullptr;  // 依赖安装提示信息
    int dependAuthStatu = -1;                       // 存储依赖授权状态

    bool resetButtonFocus = true;  // 展示包信息前，复位焦点状态，切换后第一次显示有效
};

#endif  // SINGLEINSTALLPAGE_H
