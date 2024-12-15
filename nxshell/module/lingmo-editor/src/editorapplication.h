// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef EDITORAPPLICATION_H
#define EDITORAPPLICATION_H
#include "environments.h"

#include <DApplication>
#include <DPushButton>
#include <QEvent>
#include <QObject>
#include <DSettingsDialog>
#include <DDialog>
#include <QKeyEvent>
#include <QObject>
#include <QTimer>
#include <QSharedMemory>
#include "startmanager.h"

DWIDGET_USE_NAMESPACE

class EditorApplication:public DApplication
{
    Q_OBJECT
public:
    EditorApplication(int &argc, char *argv[]);
    ~EditorApplication() override;
protected:
    void handleQuitAction()override;
    bool notify(QObject *object, QEvent *event) override;
private:
    // 模拟键盘space键按压
    void pressSpace(DPushButton *pushButton);
};

#endif // EDITORAPPLICATION_H
