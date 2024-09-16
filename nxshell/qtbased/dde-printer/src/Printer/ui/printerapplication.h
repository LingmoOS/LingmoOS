// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PRINTERAPPLICATION_H
#define PRINTERAPPLICATION_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QDBusMessage>

class JobManagerWindow;
class DPrintersShowWindow;

class PrinterApplication : public QObject
{
    Q_OBJECT

public:
    static PrinterApplication *getInstance();

    int create();
    int launchWithMode(const QStringList &arguments);
    int stop();

    int showMainWindow(const QString &printerName);
    int showJobsWindow();

public slots:
    void slotMainWindowClosed();

protected slots:
    void slotNewProcessInstance(qint64 pid, const QStringList &arguments);

protected:
    PrinterApplication();
    ~PrinterApplication();

private:
    JobManagerWindow *m_jobsWindow;
    DPrintersShowWindow *m_mainWindow;
    QSystemTrayIcon *m_systemTray;
};

#define g_printerApplication PrinterApplication::getInstance()

#endif //PRINTERAPPLICATION_H
