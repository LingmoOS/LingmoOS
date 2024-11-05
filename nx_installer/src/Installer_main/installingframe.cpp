#include "installingframe.h"
#include <QGridLayout>
#include <QEvent>
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QApplication>
#include <sys/stat.h>
#include <QThread>
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/kcommand.h"
#include "QProcess"
#define CFG_PATH    "/var/log/installer/lingmo-os-installer.log"
#include "QTextCursor"

#define PROGRESS_RATIO 0.8
#define SHOWDETAILINSTALLMSG    1
#define SHOWSIMPLEINSTALLMSG    0
#define SHOWERRORINSTALLMSG     2

namespace KInstaller {
using namespace KServer;
InstallingFrame::InstallingFrame(QWidget *parent) : QWidget(parent),
    tmpvalue(0)
{
    initUI();
    addStyleSheet();
    progressBarCtrl = new ShowProgressBar(this);
     mywatcher=new QFileSystemWatcher(this);
     mywatcher->addPath(CFG_PATH);
    initAllConnect(); 
    //恢复鼠标正常状态
    QApplication::restoreOverrideCursor();
    showMsgDetail();
    slide->setFocus();
    translateStr();
}

InstallingFrame::~InstallingFrame()
{

}

void InstallingFrame::initUI()
{
    gridLayout = new QGridLayout;
    this->setLayout(gridLayout);
    gridLayout->setColumnStretch(0, 1);
    gridLayout->setColumnStretch(1, 10);
    gridLayout->setColumnStretch(2, 1);
    slide = new SlideShow(nullptr);
    slide->setFocus();
    gridLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1, 1, 1);
    gridLayout->addWidget(slide, 0, 1, 1, 1, Qt::AlignCenter);
    littleTitle = new QLabel;
    littleTitle->setObjectName("littleTitle");
    gridLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum), 1, 1, 1, 1);
    gridLayout->addWidget(littleTitle, 2, 1, 1, 1, Qt::AlignCenter);
    gridLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Minimum), 3, 1, 1, 1);
    m_installProgressBar = new myprobar(this);

    QRect rect = KServer::GetScreenRect();
    m_installProgressBar->setFixedWidth(rect.width() * PROGRESS_RATIO);
    m_installProgressBar->setObjectName(QStringLiteral("installProgressBar"));
    m_installProgressBar->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Minimum);
    gridLayout->addWidget(m_installProgressBar, 4, 1, 1, 1);

    //显示日志详情按键及日志简略显示
    {
    textDetail=new QPushButton;
    textDetail->setObjectName("textDetail");
     QHBoxLayout *hbox= new QHBoxLayout(this);
    textDetail->setLayout( hbox);
    stext=new QLabel;
    hbox->addWidget(stext,0,Qt::AlignCenter);
    spic=new QLabel;
    QImage *img=new QImage;
    img->load(":/data/png/pan-down-symbolic.svg");
    spic->setPixmap(QPixmap::fromImage(*img));
    hbox->addWidget(spic,1,Qt::AlignRight);

    MsgContext=new QLabel;
    MsgContext->setObjectName("MsgContext");
    MsgContext->setMaximumSize(1000,20);
    MsgContext->setSizePolicy(QSizePolicy::Maximum,QSizePolicy::Maximum);
    QHBoxLayout *hbox1= new QHBoxLayout(this);
    hbox1->addWidget(MsgContext,0,Qt::AlignLeft);
    hbox1->addWidget(textDetail,1,Qt::AlignRight);
    gridLayout->addLayout(hbox1,5, 1, 1, 1, Qt::AlignRight);
    }

    //日志详情界面
    {
        QHBoxLayout *textbox= new QHBoxLayout;
        textbox->setContentsMargins(0,0,0,0);
        textContext=new QLabel;
        textContext->setObjectName("textContext");
        textContext->setMaximumSize(921,581);
        textContext->setMinimumSize(920,580);
        textContext->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
        gridLayout->addLayout(textbox,6,1,1,1);//,Qt::AlignCenter);
        textbox->addWidget(textContext);
        textContext->setVisible(false);//初始化隐藏日志
        QGridLayout *hbox2= new QGridLayout;
        hbox2->setColumnStretch(0, 32);
        hbox2->setColumnStretch(1, 856);
        hbox2->setColumnStretch(2, 32);
        textContext->setLayout(hbox2);

        textMsg=new QTextBrowser;
        textMsg->setObjectName("textMsg");
        textMsg->setContextMenuPolicy(Qt::ContextMenuPolicy::NoContextMenu);//解决bugI9R5KF
        hbox2->addItem(new QSpacerItem(32,40, QSizePolicy::Minimum, QSizePolicy::Minimum), 0, 1, 5, 1);
        hbox2->addWidget(textMsg,1,1,1,1);
        hbox2->addItem(new QSpacerItem(32,40,QSizePolicy::Minimum, QSizePolicy::Minimum), 2, 1, 5, 1);

        textMsg->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
        fi=new QFile(CFG_PATH);
        if(!fi->open(QFile::ReadWrite|QFile::Text))
        qDebug()<<"log open fail";
        currline = fi->size();
        fi->close();
     }
}

void InstallingFrame::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void InstallingFrame::addStyleSheet()
{
    QFile file(":/data/qss/InstallingFrame.css");
    file.open(QFile::ReadOnly|QFile::Text);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void InstallingFrame::translateStr()
{
//    m_mainTitle->setText((tr("Installing")));//安装进行中
    littleTitle->setText(tr("The system is being installed, please do not turn off the computer"));//系统正在安装，请不要关闭计算机
    stext->setText(tr("Log"));
}

void InstallingFrame::initAllConnect()
{
    connect(progressBarCtrl, &ShowProgressBar::signelProgressValue, this, &InstallingFrame::updateProgressValue);
    connect(textDetail,&QPushButton::clicked,this,&InstallingFrame::showMsgDetail);
    connect(mywatcher,&QFileSystemWatcher::fileChanged,this,&InstallingFrame::flashMsg);
}
void InstallingFrame::clickNextButton()
{

}

void InstallingFrame::showErrorFrame()
{
   // return;
     textContext->setVisible(true);//初始化隐藏日志

     slide->setParent(nullptr);
   //  textContext->setParent(nullptr);
     MsgContext->setParent(nullptr);
     textDetail->setParent(nullptr);
  //   m_installProgressBar->setParent(nullptr);
     m_installProgressBar->setVisible(false);
     littleTitle->setParent(nullptr);
   //  textbox->setParent(nullptr);
  //   gridLayout->removeItem(gridLayout->itemAtPosition(0,1));
     gridLayout->removeItem(gridLayout->itemAtPosition(1,1));
     gridLayout->removeItem(gridLayout->itemAtPosition(2,1));
     gridLayout->removeItem(gridLayout->itemAtPosition(3,1));
  //   gridLayout->removeItem(gridLayout->itemAtPosition(4,1));
     gridLayout->removeItem(gridLayout->itemAtPosition(5,1));

     QLabel* m_mainTitle1=new QLabel;
     m_mainTitle1->setObjectName("mainTitle1");
     m_mainTitle1->setText(tr("Install faild"));
     gridLayout->addWidget(m_mainTitle1,0,1,1,1,Qt::AlignCenter);

    QLabel* littleTitle1=new QLabel;
    littleTitle1->setText(tr("Sorry, System cannot continue the installation. Please feed back the error log below so that we can better solve the problem for you."));
    gridLayout->addWidget(littleTitle1,1,1,1,1,Qt::AlignCenter);

    gridLayout->addItem(new QSpacerItem(32,53, QSizePolicy::Minimum, QSizePolicy::Preferred),7,1,1,1);
    m_restartBtn = new QPushButton;
    m_restartBtn->setObjectName("restartBtn");
    m_restartBtn->setText(tr("Restart"));
    gridLayout->addWidget(m_restartBtn, 8, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    connect(m_restartBtn,&QPushButton::clicked,this,&InstallingFrame::clickRestartBtn);
    is_faild=1;//安装失败标志
    translateStr();
    addStyleSheet();
}

QLabel *InstallingFrame::geterrorContext()
{
    return textContext;
}

void InstallingFrame::findFile()
{
    qDebug()<<"finish find File+++++++++++++++";
    m_installProgressBar->setValue(100);
    emit signalFinished();
}

void InstallingFrame::updateProgressValue(int value)
{
    m_installProgressBar->setValue(value);
    if (value == 90) {
        qDebug() << "updateProgressValue" << value;
//        timer->start();
        m_installProgressBar->setValue(96);
    }
}

void InstallingFrame::closeW()
{
    progressBarCtrl->timer->stop();
}

void InstallingFrame::showMsgDetail()
{
    if(flag)
    {
        gridLayout->setRowStretch(0,1);
        slide->setVisible(true);
        gridLayout->setRowStretch(6,0);
        textContext->setVisible(false);//隐藏日志
        QImage *img=new QImage;
        img->load(":/data/png/pan-down-symbolic.svg");
        spic->setPixmap(QPixmap::fromImage(*img));
        gridLayout->removeItem(gridLayout->itemAtPosition(7,1));
        flag=0;
    }
    else
    {
        gridLayout->setRowStretch(0,0);
        slide->setVisible(false);
      //  textbox->setContentsMargins(0,0,0,0);
        gridLayout->setRowStretch(6,1);
        textContext->setVisible(true);//显示日志
        QImage *img=new QImage;
        img->load(":/data/png/pan-up-symbolic.svg");
        spic->setPixmap(QPixmap::fromImage(*img));
        gridLayout->addItem(new QSpacerItem(32,150, QSizePolicy::Minimum, QSizePolicy::Preferred),7,1,1,1);
        flag=1;
   }
}


void InstallingFrame::flashMsg()
{
    if(!fi->open(QFile::ReadOnly|QFile::Text))
        qDebug()<<"log open fail";
    if(fi->size() == currline)//过滤掉无内容改变的触发
    {
        qDebug()<<"QFileSystemWatcher::fileChanged void!";
        fi->close();
        mywatcher->addPath(CFG_PATH);
        return;
    }

    fi->seek(currline-1);
    currline=fi->size()+1;
    QString strT;
    while(!fi->atEnd())
    {
        //strT = QString::fromLocal8Bit(fi->readLine());
        strT = fi->readLine();
        if(strT.isEmpty() ||strT.size() == 1)//过滤掉内容改变不完整的触发
            continue;
        if('\n' == strT[0])
        {
            strT.remove(0,1);
        }

        MsgContext->setText(strT);
        textMsg->append(strT);
//        MsgContext->setText(fi->readLine());
//        textMsg->append(MsgContext->text());
     }
    fi->close();
    mywatcher->addPath(CFG_PATH);
  //  textMsg->moveCursor(QTextCursor::End);
}


void InstallingFrame::clickRestartBtn()
{
    QProcess process;
    process.startDetached("systemctl reboot");
}

void InstallingFrame::keyPressEvent(QKeyEvent *event)
{
    if((event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)&& is_faild)
        clickRestartBtn();
    else
        QWidget::keyPressEvent(event);
}

}

