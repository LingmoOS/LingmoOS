// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFINSTALLERRORDIALOG_H
#define DFINSTALLERRORDIALOG_H

#include "interfaces/dfontbasedialog.h"
#include "dfinstallerrorlistview.h"
#include "dfinstallerroritemmodel.h"

#include <DPushButton>
#include <DSuggestButton>
#include <DDialog>

#include <QResizeEvent>

DWIDGET_USE_NAMESPACE

class DFInstallNormalWindow;
class SignalManager;
class DFInstallErrorListView;
/*************************************************************************
 <Class>         DFInstallErrorDialog
 <Description>   安装验证框页面-应用验证字体信息
 <Author>
 <Note>          null
*************************************************************************/
class DFInstallErrorDialog : public DDialog
{
    Q_OBJECT

public:
    explicit DFInstallErrorDialog(QWidget *parent = nullptr,
                                  const QStringList &errorInstallFontFileList = QStringList());
    ~DFInstallErrorDialog()Q_DECL_OVERRIDE;
    //构造时初始化字体信息列表
    void initData();
    //字体验证框主页面
    void initUI();
    //获取已勾选继续安装项个数
    int getErrorFontCheckedCount();
    //获取可勾选项个数
    int getErrorFontSelectableCount();

private:
    //刷新继续按钮的状态-选中数量大于1时，继续按钮可用
    void resetContinueInstallBtnStatus();
    //重写键盘press事件
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    //重写关闭事件-发送取消继续安装信号
    void closeEvent(QCloseEvent *event) override;

signals:
    //发送取消继续安装重复字体的信号
    void onCancelInstall();
    //发送请求继续安装重复字体的信号
    void onContinueInstall(const QStringList &continueInstallFontFilelList);

public slots:
    //勾选按钮点击或回车选中事件
    void onListItemClicked(const QModelIndex &index);
    //用于处理字体验证框中选择多个字体后使用快捷键改变选中字体的选择状态
    void onListItemsClicked(const QModelIndexList &indexList);
    ///*参数1 正常筛选后筛选出的错误字体  参数2 字体验证框弹出过程中安装的所有字体
    //参数3 新添加到字体验证框中的字体  参数4 之前添加到字体验证框中的字体   */
    void addData(QStringList &errorFileList, QStringList &halfInstalledFilelist,
                 QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles);
    //按钮点击事件
    void onControlButtonClicked(int btnIndex);

#ifdef DTKWIDGET_CLASS_DSizeMode
    //紧凑模式切换
    void slotSizeModeChanged(DGuiApplicationHelper::SizeMode sizeMode);
#endif

private:
    DFInstallNormalWindow *m_parent;
    QWidget *m_mainFrame;
    QVBoxLayout *m_mainLayout;

    DFInstallErrorListView *m_installErrorListView;

    SignalManager *m_signalManager;

    QStringList m_errorInstallFiles;
    QList<DFInstallErrorItemModel> m_installErrorFontModelList;
};


#endif  // DFINSTALLERRORDIALOG_H
