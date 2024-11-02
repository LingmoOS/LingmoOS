#include "finishedInstall.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QProcess>
#include <QEvent>
#include <QMessageBox>
#include "../PluginService/kcommand.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include "../PluginService/ksystemsetting_unit.h"
using namespace KServer;
namespace KInstaller {
FinishedFrame::FinishedFrame(QWidget *parent) : QWidget(parent)
{
    initUI();
    initAllConnect();
    addStyleSheet();
    this->setFocus();
}

void FinishedFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 2);
    gLayout->setColumnStretch(2, 3);

    QRect rect = KServer::GetScreenRect();

    QWidget* widget = new QWidget;
    m_mainTitle = new QLabel(widget);
    m_mainTitle->setObjectName("mainTitle");
    m_mainTitle->setAlignment(Qt::AlignLeft);
    gLayout->addItem(new QSpacerItem(10, rect.height() * 0.3, QSizePolicy::Expanding, QSizePolicy::Minimum), 0, 1, 1, 1);
//    gLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::Align:Top);
//    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum), 2, 1, 1, 1);

    m_finishLable = new QLabel(widget);

    m_finishLable->setAlignment(Qt::AlignRight);
    m_finishLable->setPixmap(QPixmap(":/data/svg/success0.svg"));//TODO
//    gLayout->addWidget(m_errorLable, 3, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
//    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum), 4, 1, 1, 1);

    QHBoxLayout* hlayout = new QHBoxLayout;
    hlayout->setAlignment(Qt::AlignCenter);
    hlayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    hlayout->addWidget(m_finishLable);
    hlayout->addWidget(m_mainTitle);
    hlayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Minimum));
    gLayout->addLayout(hlayout, 1, 1, 1, 1, Qt::AlignCenter);
//    gLayout->addWidget(m_errorLable, 1, 1, 1, 1, Qt::AlignCenter);

    gLayout->setRowStretch(4, 3);

    m_restartBtn = new QPushButton;
    m_restartBtn->setObjectName("restartBtn");
    gLayout->addWidget(m_restartBtn, 5, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 120, QSizePolicy::Expanding, QSizePolicy::Minimum), 6, 1, 1, 1);
    translateStr();

}

void FinishedFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void FinishedFrame::translateStr()
{
//    m_tipInfo->setText(tr("The system installation is complete.\n"
//                          "But after the system restarts, any changes you made and saved files will not be retained"));

    m_mainTitle->setText(tr("Installation Finished"));
    m_restartBtn->setText(tr("Restart"));
}

void FinishedFrame::addStyleSheet()
{
    QFile file(":/data/qss/InstallErrorFrame.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void FinishedFrame::initAllConnect()
{
    connect(m_restartBtn, &QPushButton::clicked, this, &FinishedFrame::clickRestartBtn);
    connect(this, &FinishedFrame::enterKeyPressed, this, &FinishedFrame::clickRestartBtn);
}

void FinishedFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO ;
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        emit enterKeyPressed();
    else if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control) {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}

void FinishedFrame::clickNextButton()
{

}

void FinishedFrame::clickRestartBtn()
{
        QProcess::execute("reboot");
}
}
