#include "installermainwidget_auto.h"
#include <QHBoxLayout>
#include <QGridLayout>
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QMouseEvent>
#include <QProcess>
#include <QDebug>
#include "../PluginService/ui_unit/messagebox.h"
#include <QToolTip>
#include <QDesktopWidget>
#include "ksetting.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../plugins/VirtualKeyboard/src/virtualkeyboard.h"
#include "../plugins/VirtualKeyboard/src/keyboardwidget.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/kthread.h"
using namespace KServer;
//桌面背景色
#define BGRGB qRgb(33, 83, 156)
namespace KInstaller {

InstallerMainWidget_auto::InstallerMainWidget_auto(bool oemflag, QWidget *parent)
    : QWidget(parent),
       bloemflag(oemflag),
       m_backQuickInstallThread(new QThread(this)),
       m_backquickProcess(new QuickInsatllerProcess)
{
    initUI();
    initBackground();
    addStyleSheet();
    initAllConnect();

    if(!bloemflag) {
        showInstallingFrame();
        m_backquickProcess->moveToThread(m_backQuickInstallThread);
        m_backQuickInstallThread->start();
    }

}

void InstallerMainWidget_auto::initAllConnect()
{
    connect(m_backQuickInstallThread, &QThread::started, m_backquickProcess, &QuickInsatllerProcess::exec);
    connect(m_backquickProcess, &QuickInsatllerProcess::signalProcessStatus, [=](int exitcode){
        if(exitcode == 0) {
            showFinishedFrame();
        } else {
            showInstallingErrorFrame();
        }
    });
    connect(m_backQuickInstallThread, &QThread::finished, m_backquickProcess, &QuickInsatllerProcess::deleteLater);
}


InstallerMainWidget_auto::~InstallerMainWidget_auto()
{
   quitThreadRun(m_backQuickInstallThread);
}
void InstallerMainWidget_auto::initUI()
{
    this->setObjectName("InstallerMainWidget_auto");

    QGridLayout* gLayoutl = new QGridLayout();
    gLayoutl->setContentsMargins(40, 140, 40, 80);


    //加载中间窗口显示
    m_stackWidget = new QStackedWidget(this);
//    gLayoutl->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 0, 1, 1);
    gLayoutl->addWidget(m_stackWidget, 0, 0, 1, 1, Qt::AlignCenter);
    if(!bloemflag) {
        m_installingFrame = new InstallingFrame();
        m_installingFrame->show();
        m_stackWidget->addWidget(m_installingFrame);
        m_stackWidget->setCurrentWidget(m_installingFrame);
    } else {
        m_oemFrame = new InstallingOEMConfigFrame;
        m_oemFrame->setParent(this);
        m_oemFrame->show();
        m_stackWidget->addWidget(m_oemFrame);
        m_stackWidget->setCurrentWidget(m_oemFrame);
    }

    this->setLayout(gLayoutl);


}
void InstallerMainWidget_auto::initBackground()
{
    // 无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //全屏
    showFullScreen();
    //设置背景blue色
    QRect rect = KServer::GetScreenRect();
    QPixmap pixmap = QPixmap(":/data/png/lmbg.png").scaled(rect.width(), rect.height());
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}


void InstallerMainWidget_auto::addStyleSheet()
{
    QFile file(":/data/qss/installermainwidget.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void InstallerMainWidget_auto::showInstallingFrame()
{
    connect(m_installingFrame, &InstallingFrame::signalFinished, this, &InstallerMainWidget_auto::showFinishedFrame);
    connect(this, &InstallerMainWidget_auto::signalScriptFinished, m_installingFrame, &InstallingFrame::findFile);
    connect(this, &InstallerMainWidget_auto::signalInstallingClose, m_installingFrame, &InstallingFrame::closeW);

}

void InstallerMainWidget_auto::showInstallingErrorFrame()
{
    emit signalInstallingClose();
    m_installingFrame->showErrorFrame();
}

void InstallerMainWidget_auto::showFinishedFrame()
{
    emit signalInstallingClose();
    QProcess::execute("systemctl reboot");

}


}
