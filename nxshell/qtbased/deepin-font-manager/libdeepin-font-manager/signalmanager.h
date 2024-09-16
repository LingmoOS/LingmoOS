// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SIGNALMANAGER_H
#define SIGNALMANAGER_H

#include <QObject>
#include <QString>

/*************************************************************************
 <Class>         SignalManager
 <Description>   信号管理类-负责信号分发管理
 <Author>
 <Note>          null
*************************************************************************/
class SignalManager : public QObject
{
    Q_OBJECT
public:
    static SignalManager *instance();
    ~SignalManager();
signals:
    //触发完成安装信号
    void finishFontInstall(const QStringList &fileList);

    //触发刷新用户字体列表信号
    void refreshUserFont();

    //触发大小改变信号
    void sizeChange(int height);
    //触发弹出字体验证框请求
    void popInstallErrorDialog();
    //触发字体验证框隐藏信号
    void hideInstallErrorDialog();
    //触发字体验证框列表刷新信号
    void updateInstallErrorListview(QStringList &errorFileList, QStringList &halfInstalledFiles,
                                    QStringList &addHalfInstalledFiles, QStringList &oldHalfInstalledFiles);
    //触发字体验证框弹出中途安装信号
    void installDuringPopErrorDialog(const QStringList &fileList);
    //触发左侧菜单栏是否可滚动信号
    void setSpliteWidgetScrollEnable(bool isInstalling);
    //触发字体列表改变信号
    void changeView();
    //触发删除确认取消信号
    void cancelDel();
    //触发更新预览大小跟随滑块信号
    void fontSizeRequestToSlider();
    //触发切换焦点至菜单
    void requestSetLeftSiderBarFocus();
    //清空待恢复选中状态索引列表
    void clearRecoverList();
    //更新标志位：是否从字体列表获取焦点
    void setLostFocusState(bool isTrue);
    //请求设置addbutton焦点
    void requestSetTabFocusToAddBtn();
    //右键菜单关闭请求触发操作
    void menuHidden();
private:
    explicit SignalManager(QObject *parent = nullptr);
public:
    static bool m_isOnLoad;
    static bool m_isOnStartupLoad;
    static bool m_isDataLoadFinish;

private:
    static SignalManager *m_signalManager;
};

#endif // SIGNALMANAGER_H
