// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef RECORDERTABLET_H
#define RECORDERTABLET_H

#include <QObject>
#include <QTimer>

class RecorderTablet : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.TabletScreenRecorder")
public:
    explicit RecorderTablet(QObject *parent = nullptr);
    ~RecorderTablet();

    void start();
    void stop();
    void startStatusBar();
    static const QString getRecorderNormalIcon();
signals:
    Q_SCRIPTABLE void StatusBarIconChange(const QString &in0);
    Q_SCRIPTABLE void DropDownIconChange(const QString &in0);
    void finished();
public slots:
    void update();
    void updateStatusBar();
private:
    void sendRecorderState(const bool state);
public:
    // 平板模式下 资源配置路径
    static const QString RESOURCES_PATH;


private:
    int showCountdownCounter = 0;
    int flashTrayIconCounter = 0;
    QTimer *m_statusBarTimer = nullptr;
    QTimer *m_changeTimer = nullptr;
};

#endif // RECORDERTABLET_H
