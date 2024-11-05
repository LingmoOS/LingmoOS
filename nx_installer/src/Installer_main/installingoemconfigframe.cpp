#include "installingoemconfigframe.h"
#include <QLabel>
#include <QGridLayout>
#include <QEvent>
#include <QThread>
#include <QDebug>
#include <QTimer>
#include "../PluginService/backendThread/oeminstallerprocess.h"
#include "../PluginService/kthread.h"
namespace KInstaller {
using namespace KServer;

InstallingOEMConfigFrame::InstallingOEMConfigFrame(QWidget *parent) : QWidget(parent)
  ,m_backOEMThread(new QThread(this))
  ,m_oemProcess(new OemInstallerProcess),
    var(0)
{
    initUI();
    addStyleSheet();
    m_oemProcess->moveToThread(m_backOEMThread);
    m_backOEMThread->start();
    timer = new QTimer(this);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, this, &InstallingOEMConfigFrame::finishLoadDisk);
    timer->start();
}

InstallingOEMConfigFrame::~InstallingOEMConfigFrame()
{
    quitThreadRun(m_backOEMThread);
}

void InstallingOEMConfigFrame::initUI()
{
    QGridLayout* layout = new QGridLayout;
    this->setLayout(layout);
    layout->setSpacing(50);
    label = new QLabel;
    label->setStyleSheet("QLabel { font-size:42px; color:#FFFFFF; }");
    layout->setRowStretch(0, 3);
    layout->addWidget(label, 1, 0, 1, 1, Qt::AlignCenter);
//    layout->addItem(new QSpacerItem(10, 36, QSizePolicy::Fixed, QSizePolicy::Fixed), 2, 0, 1, 1);
    m_loadpic = new QLabel;
    m_loadpic->setObjectName("loadpic");
    m_loadpic->setPixmap(QPixmap(":/data/svg/loading/lingmo-loading-0.svg").scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    layout->addWidget(m_loadpic, 2, 0, 1, 1, Qt::AlignCenter);
//    label1 = new QLabel;
//    label1->setStyleSheet("QLabel { font-size:14px; color:#FFFFFF; }");
//    layout->addWidget(label1, 3, 0, 1, 1, Qt::AlignCenter);
    layout->setRowStretch(4, 4);

    connect(m_backOEMThread, &QThread::started, m_oemProcess, &OemInstallerProcess::exec);
    connect(m_oemProcess, &OemInstallerProcess::signalProcessStatus, [=]{emit signalExecError("oem erro");});
    translateStr();
}


void InstallingOEMConfigFrame::addStyleSheet()
{

}

void InstallingOEMConfigFrame::translateStr()
{
    label->setText(tr("Progressing system configuration"));
//    label1->setText(tr("Please wait patiently......"));
}

void InstallingOEMConfigFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void InstallingOEMConfigFrame::finishLoadDisk()
{
    qDebug() << "finishLoadDisk";
    var++;
    if(var > 7) {
        var = 0;
    }
    QString s = QString(":/data/svg/loading/lingmo-loading-%1.svg").arg(var);

    m_loadpic->setPixmap(QPixmap(s).scaled(QSize(48, 48), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_loadpic->setScaledContents(true);
    m_loadpic->update();
}
}
