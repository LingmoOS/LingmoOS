// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef MULTIPLEINSTALLPAGE_H
#define MULTIPLEINSTALLPAGE_H

#include "view/widgets/infocontrolbutton.h"
#include "view/widgets/droundbgframe.h"
#include "view/widgets/installprocessinfoview.h"
#include "manager/packagesmanager.h"

#include <DPushButton>
#include <DProgressBar>
#include <DSpinner>
#include <DCommandLinkButton>

#include <QPropertyAnimation>
#include <QWidget>

class AbstractPackageListModel;
class PackagesListView;
class DebListModel;
class WorkerProgress;
class MultipleInstallPage : public QWidget
{
    Q_OBJECT

public:
    explicit MultipleInstallPage(AbstractPackageListModel *model, QWidget *parent = nullptr);

    /**
     * @brief setEnableButton 根据授权框的弹出/取消，设置安装按钮禁用/请
     * @param bEnable 按钮启用与禁用的标识
     */
    void setEnableButton(bool bEnable);

    /**
     * @brief afterGetAutherFalse 授权结束或取消后处理界面情况
     */
    void afterGetAutherFalse();

    /**
     * @brief refreshModel 刷新model
     */
    void refreshModel();

    /**
     * @brief DealDependResult  处理依赖处理授权的结果
     * @param iAuthRes          依赖处理的结果
     * @param dependName        依赖的名称
     */
    void DealDependResult(int iAuthRes, QString dependName);

signals:
    /**
     * @brief back 返回到fileChooseWidget界面的信号
     */
    void signalBackToFileChooseWidget() const;

    /**
     * @brief requestRemovePackage  删除包的信号
     * @param index 要删除的包的下标
     */
    void signalRequestRemovePackage(const int index) const;

private slots:

    /**
     * @brief onWorkerFinshed   批量安装结束，调整页面展示效果
     */
    void slotWorkerFinshed();

    /**
     * @brief onOutputAvailable 批量安装过程中添加安装信息
     * @param output    安装过程信息
     */
    void slotOutputAvailable(const QString &output);

    /**
     * @brief onProgressChanged 批量安装进度变化
     * @param progress 已经增加的进度
     */
    void slotProgressChanged(const int progress);

    /**
     * @brief onRequestRemoveItemClicked 在批量安装的包的时候删除某个包
     * @param index 要删除的包的下标
     */
    void slotRequestRemoveItemClicked(const QModelIndex &index);

    /**
     * @brief showInfo  处理展示详细信息的界面展示
     */
    void slotShowInfo();

    /**
     * @brief hideInfo  处理隐藏详细信息的界面展示
     */
    void slotHideInfo();

    /**
     * @brief onAutoScrollInstallList  自动滚动到当前正在安装的包
     * @param opIndex  当前正在安装的包的位置
     */
    void slotAutoScrollInstallList(int opIndex);

    /**
     * @brief hiddenCancelButton    安装开始后处理界面展示情况
     */
    void slotHiddenCancelButton();

    /**
     * @brief slotShowDependsInfo 显示依赖关系的视图
     */
    void slotShowDependsInfo();

    /**
     * @brief slotHideDependsInfo  隐藏依赖关系的视图
     */
    void slotHideDependsInfo();

    /**
     * @brief slotDependPackages  缺失依赖显示
     * @param dependPackages  依赖包存储
     * @param installWineDepends 是否进入wine依赖配置
     */
    void slotDependPackages(Pkg::DependsPair dependPackages, bool installWineDepends);

private:
    /**
     * @brief initUI 初始化界面布局
     */
    void initUI();

    /**
     * @brief initConnections 初始化链接
     */
    void initConnections();

    /**
     * @brief initContentLayout 初始化中心layout
     */
    void initContentLayout();

    /**
     * @brief initPkgDependsInfoView 初始化依赖信息显示布局
     */
    void initPkgDependsInfoView();

    /**
     * @brief initTabOrder 初始化切换顺序
     */
    void initTabOrder();

    /**
     * @brief setButtonFocusPolicy 设置按钮的焦点策略
     */
    void setButtonFocusPolicy();

    /**
     * @brief setButtonAutoDefault 设置按钮可以被enter和Return键触发
     */
    void setButtonAutoDefault();

    /**
     * @brief initControlAccessibleName 给控件添加AccessibleName
     */
    void initControlAccessibleName();

private:
    DRoundBgFrame *m_appsListViewBgFrame = nullptr;  // listView的背景frame 修改listView的背景样式
    AbstractPackageListModel *m_debListModel = nullptr;          // listModel类
    QWidget *m_contentFrame = nullptr;               // applistview  infoControlButton的frame
    QWidget *m_processFrame = nullptr;               // process 的frame
    QVBoxLayout *m_contentLayout = nullptr;          // applistview  infoControlButton的布局
    QVBoxLayout *m_centralLayout = nullptr;          // 主布局

    PackagesListView *m_appsListView = nullptr;  // listView

    InstallProcessInfoView *m_installProcessInfoView = nullptr;  // 安装进程信息显示窗口
    InstallProcessInfoView *m_showDependsView = nullptr;         // 依赖关系显示

    InfoControlButton *m_showDependsButton = nullptr;  // 显示依赖关系按钮

    WorkerProgress *m_installProgress = nullptr;        // 进度显示
    QPropertyAnimation *m_progressAnimation = nullptr;  // 进度动画

    InfoControlButton *m_infoControlButton = nullptr;  // 展开收缩控制按钮
    DPushButton *m_installButton = nullptr;            // 安装按钮
    DPushButton *m_backButton = nullptr;               // 返回文件选择窗口的按钮
    DPushButton *m_acceptButton = nullptr;             // 确认按钮

    // fix bug:33999 change DebInfoLabel to DCommandLinkButton for Activity color
    DCommandLinkButton *m_tipsLabel = nullptr;  // 依赖安装提示按钮
    DSpinner *m_dSpinner = nullptr;             // 依赖安装动画

    int m_index = -1;      // 当前添加的index
    bool m_upDown = true;  // 展开收缩的标识
};

#endif  // MULTIPLEINSTALLPAGE_H
