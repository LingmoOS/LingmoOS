#include "imgframe.h"
#include "QStyleFactory"
#include <QDebug>

#include "../PluginService/klinguist.h"
#include <QTextCodec>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>

#include <QFileDialog>
#include "../PluginService/ksystemsetting_unit.h"
#include "../PluginService/ui_unit/xrandrobject.h"
using namespace KServer;
namespace KInstaller {


ImgFrame::ImgFrame(QWidget *parent) : MiddleFrameManager(parent)
{
    m_nextBtn->setEnabled(true);
    initUI();
    initAllConnect();
    addStyleSheet();
    translateStr();
    this->setFocus();
    liveChoice->setChecked(true);
}

void ImgFrame::initUI()
{

    QGridLayout *chiceimgupLayout = new QGridLayout();
//    m_Widget->setLayout(chiceimgupLayout);
    m_Widget->setLayout(chiceimgupLayout);
    chiceimgupLayout->setContentsMargins(0, 0, 0, 0);
    chiceimgupLayout->setSpacing(0);

    chiceimgupLayout->setColumnStretch(0, 3);
    chiceimgupLayout->setColumnStretch(1, 1);
    chiceimgupLayout->setColumnStretch(2, 3);

    m_mainTitle = new QLabel();
    m_mainTitle->setObjectName("mainTitle");

    chiceimgupLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    chiceimgupLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop );
    chiceimgupLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    liveChoice = new QRadioButton(this);
    liveChoice->setObjectName("liveChoice");
    liveChoice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    QHBoxLayout *liveChoicelayout=new QHBoxLayout();
    liveChoicelayout->setContentsMargins(0, 0, 0, 0);
    liveChoicelayout->setSpacing(0);
    liveChoicelayout->addWidget(liveChoice,Qt::AlignLeft| Qt::AlignBottom);
    liveChoicelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    chiceimgupLayout->addLayout(liveChoicelayout, 3, 1, 1,1,Qt::AlignCenter);
    chiceimgupLayout->addItem(new QSpacerItem(10, 30, QSizePolicy::Expanding, QSizePolicy::Preferred), 4, 1, 1, 1);

    ghostChoice = new QRadioButton(this);
    ghostChoice->setObjectName("ghostChoice");
    ghostChoice->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
    QHBoxLayout *ghostChoicelayout=new QHBoxLayout();
    ghostChoicelayout->setContentsMargins(0, 0, 0, 0);
    ghostChoicelayout->setSpacing(0);
    ghostChoicelayout->addWidget(ghostChoice,Qt::AlignLeft| Qt::AlignBottom);
    ghostChoicelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
    chiceimgupLayout->addLayout( ghostChoicelayout,5,1,1,1,Qt::AlignCenter);
     chiceimgupLayout->addItem(new QSpacerItem(10, 15, QSizePolicy::Expanding, QSizePolicy::Preferred), 6, 1, 1, 1);


     filePath = new QLineEdit(this);
     filePath->setObjectName("filePath"); 

  chiceimgupLayout->addWidget( filePath, 7, 1, 1,1,Qt::AlignCenter );

  openFile = new QPushButton(this);
  openFile->setObjectName("openFile");
  openFile->setEnabled(false);
  openFile->setSizePolicy(QSizePolicy::Minimum,QSizePolicy::Minimum);
  QHBoxLayout *filelayout=new QHBoxLayout();
  filelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Preferred));
  filelayout->addWidget(openFile);
  filelayout->addSpacerItem(new QSpacerItem(10, 10, QSizePolicy::Expanding, QSizePolicy::Preferred));
  chiceimgupLayout->addLayout(filelayout, 7, 2, 1,1);
 openFile->setStyleSheet("QPushButton{color: rgb(100, 100, 100)}");

 chiceimgupLayout->addItem(new QSpacerItem(10, 100, QSizePolicy::Expanding, QSizePolicy::Expanding), 8, 1, 1,1 );
}


void ImgFrame::initAllConnect()
{
    connect(openFile,&QPushButton::clicked,this,&ImgFrame::SetFilePath);
    connect(liveChoice,&QRadioButton::clicked,this,&ImgFrame::ChangeLoad1);
    connect(ghostChoice,&QRadioButton::clicked,this,&ImgFrame::ChangeLoad2);
}
void ImgFrame::addStyleSheet()
{
    QFile file(":/res/qss/KChoiceImg.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();
    this->setStyleSheet(stylesheet);
}

void ImgFrame::SetFilePath()
{
    imgPath=QFileDialog::getOpenFileName(nullptr,tr("open file"),"/","Img Files(*.img *.IMG *.kyimg)");



    if(imgPath!=nullptr)
        {
            filePath->setText(imgPath);
         m_nextBtn->setEnabled(true);
         WriteSettingToIni("setting","ghost","true");
         WriteSettingToIni("setting","ghost-path",imgPath);
         }

}

void ImgFrame::ChangeLoad1()
{
    openFile->setEnabled(false);
    openFile->setStyleSheet("QPushButton{color: rgb(100, 100, 100)}");
    m_nextBtn->setEnabled(true);
    WriteSettingToIni("setting","ghost","false");
    openFile->update();
}

void ImgFrame::ChangeLoad2()
{
    openFile->setEnabled(true);
   openFile->setStyleSheet("QPushButton{color: rgb(255, 255, 255)}");
   if(imgPath==nullptr)
        m_nextBtn->setEnabled(false);
   openFile->update();
}

void ImgFrame::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_Enter)
    {
        if(m_nextBtn->isEnabled()==false)
            return;
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Return)
    {
        emit enterpressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
    }
    else
        QWidget::keyPressEvent(event);
}

void ImgFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
       update();
    } else {
        QWidget::changeEvent(event);
    }
}



void ImgFrame::translateStr()
{
    m_mainTitle->setText(tr("Select install way"));//选择安装路径
    ghostChoice->setText(tr("install from ghost"));//ghost安装
    liveChoice->setText(tr("install form live"));//live安装
    openFile->setText(tr("open file"));
    m_nextBtn->setText(tr("Next"));
}


void ImgFrame::clickNextButton()
{
    if(m_nextBtn->isEnabled()==false)
        return;
    if(ghostChoice->isChecked()==true)
    emit signalStackPageChanged(100);
    else
    emit signalStackPageChanged(1);
}

}
