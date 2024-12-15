// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <DApplication>

class AppInfo;

#if defined(dApp)
#undef dApp
#endif
#define dApp (static_cast<Application *>(QCoreApplication::instance()))

DWIDGET_USE_NAMESPACE

class Application : public DApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);

    ~Application() override;

    void emitSheetChanged();

signals:
    void sigShowAnnotTextWidget();

    void sigSetPasswdFocus();

protected:
    /**
     * @brief handleQuitAction
     * 通过菜单选项关闭所有
     */
    void handleQuitAction() override;

    /**
     * @brief notify
     * 让无法响应的对象强行响应
     * @param object
     * @param event
     * @return
     */
    bool notify(QObject *object, QEvent *event) override;
};

#endif  // APPLICATION_H_
