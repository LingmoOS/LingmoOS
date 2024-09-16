// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef LOGAPPLICATION_H
#define LOGAPPLICATION_H
#include <DApplication>

DWIDGET_USE_NAMESPACE
class QKeyEvent;
class LogCollectorMain;
/**
 * @brief The LogApplication class 主application类
 */
class LogApplication: public DApplication
{
public:
    LogApplication(int &argc, char **argv);
    void setMainWindow(LogCollectorMain *iMainWindow);
protected:
    bool notify(QObject *obj, QEvent *event) Q_DECL_OVERRIDE;
private:
    //mainwindow指针
    LogCollectorMain *m_mainWindow{nullptr};
};

#endif // LOGAPPLICATION_H
