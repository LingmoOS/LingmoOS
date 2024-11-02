#ifndef TRAYICON_H
#define TRAYICON_H

#include <QWidget>
#include <QSystemTrayIcon>
#include "sysresource/cpuhistorywidget.h"
#include "sysresource/memhistorywidget.h"
#include "sysresource/nethistorywidget.h"
#include "filesystem/filesystemworker.h"

class TrayIcon : public QWidget
{
    Q_OBJECT
public:
    TrayIcon(CpuHistoryWidget*cpu, MemHistoryWidget*mem, NetHistoryWidget*net, QWidget *parent = 0);
    ~TrayIcon();

    void setCpuMemNetInfo(CpuHistoryWidget*cpu, MemHistoryWidget*mem, NetHistoryWidget*net);

private:
    void initTrayIcon();
    void initMenu();
    void initConnect();

    QSystemTrayIcon *mSystemTray;
    QMenu           *mMenu;
    QAction         *mQuitAction;
    QAction         *mOpenAction;

    CpuHistoryWidget *m_cpuHisWidget = nullptr;
    MemHistoryWidget *m_memHisWidget = nullptr;
    NetHistoryWidget *m_netHisWidget = nullptr;
    FileSystemWorker *m_fileSystemWorker;


public slots:
    void ActiveTray(QSystemTrayIcon::ActivationReason reason);
    void updateToolTips();

signals:
    void openMonitor(const QString &);

};

#endif // TRAYICON_H
