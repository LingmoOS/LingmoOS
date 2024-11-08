#include "installermainwidget.h"
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
#include "../PluginService/newdemand_unit.h"
#include "../PluginService/kcommand.h"
#include "../PluginService/ui_unit/middleframemanager.h"
#include "../PluginService/base_unit/file_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "qscreen.h"

using namespace KServer;
//桌面背景色
#define BGRGB qRgb(33, 83, 156)

namespace KInstaller {

InstallerMainWidget::InstallerMainWidget(QWidget *parent)
    : QWidget(parent)
      ,m_curIndex(0)
    ,m_InstallMode("")
    ,AllStep(0)
    ,w(nullptr)
    ,keyboard(nullptr)
{
    m_stackPageMap = Ksetting::getInstance()->iPlugins;
    AllStep = m_stackPageMap.count();
    it = m_stackPageMap.begin();
    this->initUI();
    this->initBackground();
    this->initAllConnect();
    this->addStyleSheet();
    addStackWidget();
    changeLabelDotPix(m_curIndex);
    m_backbtn->setDisabled(true);
    m_backbtn->setVisible(false);

   //showInstallingFrame();
  //  m_installingFrame->showErrorFrame();
    //showInstallingErrorFrame("123");
    QScreen* NowScreen=QGuiApplication::primaryScreen();
    SCreenHeight=NowScreen->size().height();
    SCreenWidth=NowScreen->size().width();

    timer = new QTimer(this);
    connect(timer,&QTimer::timeout,this,&InstallerMainWidget::ScreenAdapt);
    timer->start(1000);

 //    QScreen* NowScreen=QGuiApplication::primaryScreen();
  //   connect(NowScreen, SIGNAL(physicalSizeChanged(QSizeF)), this, SLOT(ScreenAdapt(QSizeF)));

}

InstallerMainWidget::~InstallerMainWidget()
{
    if(w != nullptr) {
        w->close();
        w->deleteLater();
    }
}

void InstallerMainWidget::setInstallMode(QString flagstr)
{
    m_InstallMode = flagstr;
}

void InstallerMainWidget::ScreenAdapt()
{
    QScreen* NowScreen=QGuiApplication::primaryScreen();
    if((NowScreen->size().height()!=SCreenHeight) || (NowScreen->size().width()!=SCreenWidth))
    {
        initBackground();
        SCreenHeight=NowScreen->size().height();
        SCreenWidth=NowScreen->size().width();
        this->updateGeometry();
         m_stackWidget->currentWidget()->updateGeometry();
    }
}

void InstallerMainWidget::initUI()
{
    this->setObjectName("InstallerMainWidget");

    QGridLayout* gLayoutl = new QGridLayout();
    gLayoutl->setContentsMargins(24, 24, 24, 24);

    gLayoutl->setColumnStretch(0, 1);
    gLayoutl->setColumnStretch(1, 7);
    gLayoutl->setColumnStretch(2, 1);

    QHBoxLayout* horizontalLayout1 = new QHBoxLayout();
    horizontalLayout1->setMargin(0);
    //加载
    this->m_backbtn = new QPushButton();
    horizontalLayout1->addWidget(m_backbtn);
    m_backbtn->setObjectName("backbtn");
    m_backbtn->setFlat(true);
    m_backbtn->installEventFilter(this);
    horizontalLayout1->addSpacerItem(new QSpacerItem(10, 48, QSizePolicy::Fixed, QSizePolicy::Fixed));

//    m_backbtn->setPixmap(QPixmap(":/data/svg/back.svg"));

    //加载关闭按钮
    m_clostBtn = new QPushButton();
    m_clostBtn->setObjectName("clostBtn");
    horizontalLayout1->addStretch();
//    horizontalLayout1->addWidget(m_clostBtn);
    horizontalLayout1->addSpacing(5);
    m_clostBtn->installEventFilter(this);

    m_clostBtn->setFlat(true);
//    m_clostBtn->setEnabled(false);
//    m_clostBtn->setVisible(false);

    //加载中间窗口显示
   // m_middleFrame = new MiddleFrameManager(this);
    m_stackWidget = new QStackedWidget(this);

    QHBoxLayout* dotlayout = new QHBoxLayout;
    dotlayout->setSpacing(36);
    dotlayout->setMargin(0);
    for (int i = 0; i < AllStep; i++) {
        QLabel* label = new QLabel;
        label->setMaximumHeight(8);
        label->setMinimumHeight(8);
        label->setPixmap(QPixmap(":/data/png/greydot.svg").scaled(QSize(8, 8)));
        m_steps.append(label);
        dotlayout->addWidget(label);
    }
    dotlayout->setAlignment(Qt::AlignHCenter);

    QHBoxLayout* horizontalLayout2 = new QHBoxLayout();
    horizontalLayout2->setMargin(0);

    //加载键盘按钮
    m_keyboardBtn = new QPushButton(this);
    m_keyboardBtn->setObjectName("keyboardBtn");
    m_keyboardBtn->setFlat(true);
    m_keyboardBtn->installEventFilter(this);

    //加载电源关闭按钮
//    m_powerOffBtn = new QPushButton(this);
//    m_powerOffBtn->setObjectName("powerOffBtn");
//    m_powerOffBtn->setFlat(true);
//    m_powerOffBtn->installEventFilter(this);


    horizontalLayout2->addStretch();
    horizontalLayout2->addWidget(m_keyboardBtn);
    m_keyboardBtn->setEnabled(false);
    m_keyboardBtn->setVisible(false);
    horizontalLayout2->addWidget(m_clostBtn);
    horizontalLayout2->addSpacing(5);

    gLayoutl->addLayout(horizontalLayout1, 0, 0, 1, 3);
    gLayoutl->addWidget(m_stackWidget, 1, 1, 1, 1, Qt::AlignHCenter);
    gLayoutl->addItem(new QSpacerItem(10, 36, QSizePolicy::Expanding, QSizePolicy::Minimum), 2, 1, 1, 1);
    gLayoutl->addLayout(dotlayout, 3, 1, 1, 1, Qt::AlignTop | Qt::AlignHCenter);
    gLayoutl->addLayout(horizontalLayout2, 3, 2, 1, 1);

    this->setLayout(gLayoutl);

}
void InstallerMainWidget::initBackground()
{
    // 无边框
    setWindowFlags(Qt::FramelessWindowHint);

    //全屏
   showFullScreen();

    //设置背景blue色
    QRect rect = KServer::GetScreenRect();
    QPixmap pixmap = QPixmap(":/data/png/lmbg.png").scaled(rect.width(), rect.height(), Qt::KeepAspectRatioByExpanding);
    QPalette pal(this->palette());
    pal.setBrush(backgroundRole(), QBrush(pixmap));
    this->setAutoFillBackground(true);
    this->setPalette(pal);
    this->show();
}


void InstallerMainWidget::addStyleSheet()
{
    QFile file(":/data/qss/installermainwidget.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void InstallerMainWidget::initAllConnect()
{
    QObject::connect(m_clostBtn, &QPushButton::clicked, this, &InstallerMainWidget::clickCloseButton);
    QObject::connect(m_powerOffBtn, &QPushButton::clicked, this, &InstallerMainWidget::clickPowerButton);
    QObject::connect(m_keyboardBtn, &QPushButton::clicked, this, &InstallerMainWidget::clickkeyButton);
    QObject::connect(m_backbtn, &QPushButton::clicked, this, &InstallerMainWidget::clickBackButton);

}

//关闭按钮响应槽
void InstallerMainWidget::clickCloseButton()
{
    qDebug() << Q_FUNC_INFO << "test:" << QApplication::focusObject();
    //2021-4-7 add close button
    //当在live系统进行安装时，直接退出安装，继续试用；当直接安装时，则重启机器
    if(w != nullptr) {
        w->close();
    }
    QString info ;
    if(!KServer::KReadFile("/proc/cmdline").contains("live")) {
       info = tr("About to exit the installer, restart the computer.");//即将退出安装程序，重启计算机
    } else {
        info = tr("About to exit the installer and return to the trial interface.");//即将退出安装程序，回到试用界面
    }

    MessageBox *message = new MessageBox(this);
    message->setMessageInfo(info);
    if(message->exec() == QDialog::Accepted && message->execid == 1) {
        qDebug() << Q_FUNC_INFO << "OK";
        if(!KServer::KReadFile("/proc/cmdline").contains("live")) {
            this->close();
            QProcess::execute("systemctl reboot");
        } else {
            delete message;
            this->close();
            this->destroy();
            exit(0);
        }
    }
    delete message;
 //2021-4-29 解决弹出对话框之后键盘不能操作问题 50742
    QWidgetList list = QApplication::allWidgets();
    QApplication::setActiveWindow(m_stackWidget->focusWidget());
    qDebug() << Q_FUNC_INFO << "focusWidget:" <<m_stackWidget->focusWidget();
    foreach (QWidget * w, list) {
        if(w->hasFocus()) {
            qDebug() << Q_FUNC_INFO << "list0:"<< w;
            w->setFocusPolicy(Qt::NoFocus);
        }
    }

    m_stackWidget->currentWidget()->setFocus();
    qDebug() << Q_FUNC_INFO << "currentWidget:" <<m_stackWidget->currentWidget();


}

//键盘操作响应槽
void InstallerMainWidget::clickkeyButton()
{
    //添加虚拟键盘
    if(keyboard != nullptr) {
        keyboard->close();
    }
    keyboard = new VirtualKeyboard(this);
    connect(keyboard, &VirtualKeyboard::aboutToClose, [=]{
        keyboard->close();
    });
    keyboard->show();

}

//电源操作响应槽
void InstallerMainWidget::clickPowerButton()//
{
//    QProcess process;
//    process.startDetached("lingmo-session-tools");
    QString info = tr("Installer is about to exit and the computer will be shut down.");

    MessageBox *message = new MessageBox();
    message->setMessageInfo(info);
    if(message->exec() == QDialog::Accepted && message->execid == 1) {
        QProcess::execute("systemctl poweroff");
    }
    m_stackWidget->currentWidget()->setFocus();
}

//加载所有视图显示的窗口
void InstallerMainWidget::addStackWidget()
{
    for (it; it != m_stackPageMap.end(); ++it) {
        IPlugin* iplugin = (*it).second;
        MiddleFrameManager* frame = (MiddleFrameManager*)iplugin->CreateObject(this);
        m_stackWidget->addWidget(frame);
        connect(frame, &MiddleFrameManager::signalStackPageChanged, this, &InstallerMainWidget::changeStackPage);
        connect(frame, &MiddleFrameManager::signalStackFrameManager, this, &InstallerMainWidget::addStackWidgetPage);
        connect(frame, &MiddleFrameManager::signalKeyboradClose, [=]{
            if(keyboard != nullptr) {
                keyboard->close();
            }
            });
        connect(frame, &MiddleFrameManager::signalStartInstall, this, &InstallerMainWidget::showInstallingFrame);
        connect(frame, &MiddleFrameManager::signalInstallError, this, &InstallerMainWidget::showInstallingErrorFrame);
        connect(frame, &MiddleFrameManager::signalInstalScriptlFinished, [=]{emit signalScriptFinished();});
        connect(frame, &MiddleFrameManager::backspacepressed, this, &InstallerMainWidget::clickBackButton);
        connect(this, &InstallerMainWidget::signalMainFrameSize, frame, &MiddleFrameManager::setSizebyFrame);

//       qDebug() << "signalInstalScriptlFinished:" <<  connect(frame, &MiddleFrameManager::signalInstalScriptlFinished, this, &InstallerMainWidget::getFinishedSignal);
    }
    m_stackCount = m_stackWidget->count();
    m_stackWidget->setCurrentIndex(m_curIndex);
    m_stackWidget->currentWidget()->setFocus();
}

void InstallerMainWidget::setLabelDotVisible(bool flag)
{
    for (int i = 0;i < AllStep; i++) {
        m_steps.at(i)->setVisible(flag);
    }
}

void InstallerMainWidget::changeStackPage(int step)
{
    qDebug()<< "changePage="<<step;
//   showInstallingFrame();
//    m_installingFrame->showErrorFrame();
//    return;
    if(step==100)
    {
        ImgIndex=m_curIndex;
        for(int k=0;k<m_stackWidget->count();k++)
        {
            if(m_stackWidget->widget(k)->objectName().compare("KPartition")==0)
            {
                m_curIndex=k;
                ImgNext=k;
                m_stackWidget->setCurrentIndex(m_curIndex);
                m_stackWidget->currentWidget()->setFocus();
                m_stackWidget->currentWidget()->update();
                break;
            }
        }
    }
    else if (step > 0) {
        ++m_curIndex;
        if(m_curIndex >= AllStep) {
            showOEMInstallingFrame();
            return;
        }

        m_stackWidget->setCurrentIndex(m_curIndex);
        m_stackWidget->currentWidget()->setFocus();

    } else if (step < 0 ) {

        if(m_curIndex==ImgNext)
        {
            m_curIndex=ImgIndex;
            ImgIndex=0;
            ImgNext=0;
        }
        else
            --m_curIndex;

        if (m_curIndex <= 0) {
            m_curIndex = 0;
        }
        m_stackWidget->setCurrentIndex(m_curIndex);
        m_stackWidget->currentWidget()->setFocus();

    }

    if(m_stackWidget->currentWidget()->objectName() == "KPartition"){
        QScreen* NowScreen=QGuiApplication::primaryScreen();
        SCreenHeight=NowScreen->size().height();
        SCreenWidth=NowScreen->size().width();
        emit signalMainFrameSize(this, SCreenWidth, SCreenHeight);
    }

    changeLabelDotPix(m_curIndex);
    if (m_curIndex == 0) {
        m_backbtn->setEnabled(false);
        m_backbtn->setVisible(false);
    } else {
        m_backbtn->setEnabled(true);
        m_backbtn->setVisible(true);
    }

    if(m_stackWidget->currentWidget()->objectName() == "UserFrame") {
        m_keyboardBtn->setEnabled(true);
        m_keyboardBtn->setVisible(true);
    } else {
        m_keyboardBtn->setEnabled(false);
        m_keyboardBtn->setVisible(false);
    }
}

void InstallerMainWidget::clickBackButton()
{
    changeStackPage(-1);
    setLabelDotVisible(true);
}

void InstallerMainWidget::changeLabelDotPix(int step)
{
    for (int j = 0; j < AllStep; j++) {
        if (step == j) {
            m_steps.at(j)->setPixmap(QPixmap(":/data/png/dot.svg").scaled(QSize(8, 8)));
        } else
          m_steps.at(j)->setPixmap(QPixmap(":/data/png/greydot.svg").scaled(QSize(8, 8)));
    }
}

bool InstallerMainWidget::eventFilter(QObject *watched, QEvent *event)
{

    if(watched->objectName() == "keyboardBtn") {
        if(event->type() == QEvent::Enter) {
            if(w != nullptr) {
                w->close();
            }
            w = new ArrowWidget;
            w->setObjectName("ArrowWidget");
            w->setDerection(ArrowWidget::Derection::down);
            w->setText(tr("keyboard"));//        w->setCenterWidget(textLabel);
//            QPoint pos = m_keyboardBtn->mapToGlobal(QPoint());
            w->move(m_keyboardBtn->x() + (m_keyboardBtn->width() - w->width()) / 2, m_keyboardBtn->y() - w->height() - 4);
//            w->move(pos.x() - 20, pos.y() - w->height());
            w->show();
            connect(m_keyboardBtn, &QPushButton::clicked, [=]{w->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(w != nullptr) {
                w->close();
            }
            return false;
        }
    }
    if(watched->objectName() == "clostBtn") {
        if(event->type() == QEvent::Enter) {
            if(w != nullptr) {
                w->close();
            }
            w = new ArrowWidget;
            w->setObjectName("ArrowWidget");
            w->setDerection(ArrowWidget::Derection::down);
            w->setText(tr("quit"));//        w->setCenterWidget(textLabel);
//            QPoint pos = m_clostBtn->mapToGlobal(QPoint());
            w->move(m_clostBtn->x() + (m_clostBtn->width() - w->width()) / 2, m_clostBtn->y() - w->height() - 4);
//            w->move(pos.x() - 20, pos.y() - w->height());
            w->show();
            connect(m_clostBtn, &QPushButton::clicked, [=]{w->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(w != nullptr) {
                w->close();
            }
            return false;
        }
    }
    if(watched->objectName() == "backbtn") {
        if(event->type() == QEvent::Enter) {
            if(w != nullptr) {
                w->close();
            }
            w = new ArrowWidget;
            w->setObjectName("ArrowWidget");
            w->setDerection(ArrowWidget::Derection::up);
            w->setText(tr("back"));//        w->setCenterWidget(textLabel);
//            QPoint pos = m_backbtn->mapToGlobal(QPoint());

            w->move(m_backbtn->x() + (m_backbtn->width() - w->width()) / 2, m_backbtn->y() + m_backbtn->height() + 4);
            w->show();
            connect(m_backbtn, &QPushButton::clicked, [=]{w->close();});
            return true;

        } else if(event->type() == QEvent::Leave){
            if(w != nullptr) {
                w->close();
            }
            return false;
        }
    }
    return QWidget::eventFilter(watched, event);
}


void InstallerMainWidget::addStackWidgetPage(QWidget *widget)
{
    qDebug() << widget->objectName();
    setLabelDotVisible(true);
    for (int i = 0; i < m_stackWidget->count(); i++) {
        if (m_stackWidget->widget(i) == widget) {
            m_stackWidget->setCurrentWidget(widget);
            m_curIndex = i;
            return;
        }
    }
    m_stackWidget->addWidget(widget);
    m_curIndex++;
    m_stackWidget->setCurrentIndex(m_curIndex);
}

void InstallerMainWidget::showInstallingFrame()
{
    setLabelDotVisible(false);
    qDebug() << "showInstallingFrame:InstallingFrame";
    m_installingFrame = new InstallingFrame;
    m_installingFrame->setParent(this);
    m_installingFrame->show();
    m_installingFrame->slide->setFocus();
    m_stackWidget->addWidget(m_installingFrame);
    m_curIndex++;
    m_stackWidget->setCurrentWidget(m_installingFrame);
    connect(m_installingFrame, &InstallingFrame::signalFinished, this, &InstallerMainWidget::showFinishedFrame);
    connect(this, &InstallerMainWidget::signalScriptFinished, m_installingFrame, &InstallingFrame::findFile);
    connect(this, &InstallerMainWidget::signalInstallingClose, m_installingFrame, &InstallingFrame::closeW);

    updateButtonStatus();
}

void InstallerMainWidget::showInstallingErrorFrame(QString str)
{
    InstallAddFileToEFI();
    emit signalInstallingClose();
    setLabelDotVisible(false);
 //   m_installingFrame->showErrorFrame();
    qDebug() << "showInstallingErrorFrame:";
    m_installingFrame->showErrorFrame();
    //m_errorFrame = new InstallErrorFrame();
   // m_errorFrame->setParent(this);
  //  m_errorFrame->show();
  //  m_errorFrame->setFocus();
  //  m_stackWidget->addWidget(m_errorFrame);
 //   m_curIndex++;
//    m_stackWidget->setCurrentWidget(m_errorFrame);

    updateButtonStatus();
}

void InstallerMainWidget::showFinishedFrame()
{
    emit signalInstallingClose();
    setLabelDotVisible(false);
    qDebug() << Q_FUNC_INFO << "showFinishedFrame:";
    m_finishedFrame = new FinishedFrame;
    m_finishedFrame->setParent(this);
    m_finishedFrame->show();
    m_finishedFrame->setFocus();
    m_stackWidget->addWidget(m_finishedFrame);
    m_curIndex++;
    m_stackWidget->setCurrentWidget(m_finishedFrame);
    updateButtonStatus();
}

void InstallerMainWidget::updateButtonStatus()
{
    m_backbtn->setVisible(false);
    m_backbtn->setEnabled(false);

    m_clostBtn->setVisible(false);
    m_clostBtn->setEnabled(false);
    //    m_powerOffBtn->setVisible(false);
    //    m_powerOffBtn->setEnabled(false);

    m_keyboardBtn->setVisible(false);
    m_keyboardBtn->setEnabled(false);
}

bool InstallerMainWidget::showOEMInstallingFrame()
{
    qDebug() << "showOEMInstallingFrame";
    int cur = m_stackWidget->currentIndex();
    int count = m_stackPageMap.length();
    if( cur == count - 1) {
        qDebug() << "m_InstallMode =" <<m_InstallMode;
        if(m_InstallMode == "oem2") {
            setLabelDotVisible(false);
            m_oemFrame = new InstallingOEMConfigFrame;
            m_oemFrame->setParent(this);
            m_oemFrame->show();
            connect(m_oemFrame, &InstallingOEMConfigFrame::signalExecError, this, &InstallerMainWidget::showInstallingErrorFrame);
            m_stackWidget->addWidget(m_oemFrame);
            m_curIndex++;
            m_stackWidget->setCurrentWidget(m_oemFrame);
            updateButtonStatus();
            return true;
        }
    } else
        return false;
}


}
