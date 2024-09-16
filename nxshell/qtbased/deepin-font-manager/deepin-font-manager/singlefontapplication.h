// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEFONTAPPLICATION_H
#define SINGLEFONTAPPLICATION_H

#include <DApplication>
#include <DMainWindow>

#include <QSet>

DWIDGET_USE_NAMESPACE

/*************************************************************************
 <Class>         SingleFontApplication
 <Description>   返回一个应用的单例对象
 <Author>
 <Note>          null
*************************************************************************/
class SingleFontApplication : public DApplication
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.FontManager")
public:
    explicit SingleFontApplication(int &argc, char **argv);

    ~SingleFontApplication();
    //激活应用窗口
    void activateWindow();
    //初始化命令行
    bool parseCmdLine(bool bAppExist = false);

public slots:
    //批量安装字体响应
    Q_SCRIPTABLE void installFonts(const QStringList &fontPathList);

private slots:
    //批量安装字体响应
    void slotBatchInstallFonts();
    //安装完成清空列表
    void onFontInstallFinished(const QStringList &fileList);

private:
    QStringList m_selectedFiles;

    QScopedPointer<DMainWindow> m_qspMainWnd;  // MainWindow ptr
    QScopedPointer<DMainWindow> m_qspQuickWnd;  // QuickInstall Window ptr

    QSet<QString>   waitForInstallSet;   /* 可能存在多次安装相同的一个字体，将会接收到多次消息，QSet可以用来剔除重复的安装请求 UT000591 */
};

#endif // SINGLEFONTAPPLICATION_H
