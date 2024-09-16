// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SCREENSHOT_H
#define SCREENSHOT_H

#include "main_window.h"

#include <QObject>


class Screenshot : public QObject
{
    Q_OBJECT

    Q_CLASSINFO("D-Bus Interface", "com.deepin.ScreenRecorder")
public:
    explicit Screenshot(QObject *parent = nullptr);
    ~Screenshot();
public slots:
    void startScreenshot();
    void delayScreenshot(double num);
    void fullscreenScreenshot();
    void topWindowScreenshot();
    void noNotifyScreenshot();
    void OcrScreenshot();
    void ScrollScreenshot();
    void savePathScreenshot(const QString &path);
    void startScreenshotFor3rd(const QString &path);
    void initLaunchMode(const QString &launchmode);
    Q_SCRIPTABLE void stopRecord();
    Q_SCRIPTABLE QString getRecorderNormalIcon();
signals:
    Q_SCRIPTABLE void RecorderState(const bool isStart); // true begin recorder; false stop recorder;

private:
    //void initUI();

//    EventContainer *m_eventContainer = nullptr;
    QString m_launchMode;
    MainWindow m_window;

};

#endif // SCREENSHOT_H
