#include "trayicon.h"

#include <KWindowSystem>

TrayIcon::TrayIcon(CpuHistoryWidget*cpu, MemHistoryWidget*mem, NetHistoryWidget*net,QWidget *parent)
    : m_cpuHisWidget(cpu), m_memHisWidget(mem), m_netHisWidget(net),
      QWidget(parent, Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::WindowStaysOnTopHint)
{
    initTrayIcon();
    initMenu();
    initConnect();
    mSystemTray->setVisible(true);
}


TrayIcon::~TrayIcon()
{
}

void TrayIcon::initTrayIcon()
{
    mSystemTray = new QSystemTrayIcon(this);
    mSystemTray->setIcon(QIcon::fromTheme("lingmo-monitor-data-symbolic"));
    m_fileSystemWorker = new FileSystemWorker();
    QTimer *timer = new QTimer();
    connect(timer, &QTimer::timeout, this, &TrayIcon::updateToolTips);
    timer->start(1000);
}

void TrayIcon::initMenu()
{
    /* 创建菜单和行为实例 */
    mMenu = new QMenu(this);
    mQuitAction = new QAction(mMenu);    //退出
    mOpenAction = new QAction(mMenu);

    mQuitAction->setIcon(QIcon::fromTheme("window-close-symbolic"));
    mQuitAction->setText(tr("Exit"));
    mOpenAction->setIcon(QIcon::fromTheme("lingmo-system-monitor"));
    mOpenAction->setText(tr("Open monitor"));

    mMenu->addAction(mOpenAction);
    mMenu->addSeparator();
    mMenu->addAction(mQuitAction);

    mSystemTray->setContextMenu(mMenu);
}

void TrayIcon::initConnect()
{
    connect(mQuitAction, &QAction::triggered, this, [=](){
        window()->close();
        qApp->exit();
    });

    connect(mOpenAction, &QAction::triggered, this, [=](){
        emit openMonitor(QString());
    });

    connect(mSystemTray, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(ActiveTray(QSystemTrayIcon::ActivationReason)));
}

void TrayIcon::setCpuMemNetInfo(CpuHistoryWidget *cpu, MemHistoryWidget *mem, NetHistoryWidget *net)
{
    m_cpuHisWidget = cpu;
    m_memHisWidget = mem;
    m_netHisWidget = net;
}

void TrayIcon::ActiveTray(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
        case QSystemTrayIcon::DoubleClick:
            qDebug()<<">>>>>>> 鼠标双击~";
            break;
        case QSystemTrayIcon::Trigger:
            qDebug()<<">>>>>>> 鼠标左键点击~";
            emit openMonitor(QString("trayicon"));
            break;
        case QSystemTrayIcon::MiddleClick:
            qDebug()<<">>>>>>> 鼠标中键点击~";
            break;
        case QSystemTrayIcon::Context:
            qDebug()<<">>>>>>> 鼠标右键点击~";
            break;
        default:
            break;
    }
}

void TrayIcon::updateToolTips()
{
    QString network = tr("NET: ") + m_netHisWidget->getNetSendSpeed() + "    "+ m_netHisWidget->getNetLoadSpeed();
    QString cpuAndNet = tr("CPU: ") + m_cpuHisWidget->getCpuOccupancy();
    QString memoryAndNetSend = tr("RAM: ") + m_memHisWidget->getMemOccupancy();
    QString diskOccupancy;

    if (m_fileSystemWorker) {
        m_fileSystemWorker->onFileSystemListChanged();

        const QList<QString> &newDevNameList = m_fileSystemWorker->diskDevNameList();
        for (const auto &devname : newDevNameList) {
            FileSystemData fsData;
            m_fileSystemWorker->getDiskInfo(devname, fsData);
            if (fsData.mountDir() == "/") {
                diskOccupancy = QString::number((fsData.usedCapacityValue() / fsData.totalCapacityValue())*100, 'f', 1) + "%";
            }
        }
    }

    QString diskAndNetLoad = tr("Disk: ") + diskOccupancy;
    mSystemTray->setToolTip(cpuAndNet + "\n" + memoryAndNetSend + "\n" + diskAndNetLoad + "\n" + network);
}
