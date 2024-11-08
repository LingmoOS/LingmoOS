#include "prepareinstallframe.h"
#include <QFile>
#include <QDebug>
#include <QTextStream>
#include <QApplication>
#include "../PluginService/kcommand.h"
#include "../PluginService/base_unit/file_unit.h"
#include "../PluginService/kdir.h"
#include "../PluginService/kthread.h"
#include "../PluginService/ui_unit/arrowtooltip.h"
#include "../PluginService/ui_unit/xrandrobject.h"
#include <QToolTip>
#include <QTimer>
#include <QPoint>

using namespace KServer;
namespace KInstaller {

PrepareInstallFrame::PrepareInstallFrame(QWidget *parent) : MiddleFrameManager(parent),
    w(nullptr)
{
    initUI();
    initAllConnect();
    addStyleSheet();
    m_nextBtn->setEnabled(false);
     qDebug()<<":PrepareInstallFrame set m_nextBtn false";
    m_nextBtn->setStyleSheet("background-color: #2D6BAA;");
    this->setFocus();
}

PrepareInstallFrame::~PrepareInstallFrame()
{
    if(w != nullptr) {
        w->deleteLater();

    }
}
void PrepareInstallFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout;
    gLayout->setContentsMargins(0, 0, 0, 0);
    m_Widget->setLayout(gLayout);
    gLayout->setColumnStretch(0, 1);
    gLayout->setColumnStretch(1, 5);
    gLayout->setColumnStretch(2, 1);

//    m_nextBtn->setDisabled(true);

    m_mainTitle = new QLabel(this);
    m_mainTitle->setObjectName("mainTitle");
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    gLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    tiplabel = new QLabel(this);
    tiplabel->setObjectName("tiplabel");
    gLayout->addWidget(tiplabel, 3, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);

    gLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 4, 1, 1, 1);
//    partLabel = new QLabel(this);
//    partLabel->setObjectName("partLabel");
//    partLabel->setAlignment(Qt::AlignLeft);
//    gLayout->addWidget(partLabel, 5, 1, 1, 1);

    partInfo = new QTextBrowser;
    partInfo->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    partInfo->setContextMenuPolicy(Qt::NoContextMenu);
    partInfo->setObjectName("partInfo");
    partInfo->setMaximumHeight(KServer::GetScreenRect().height() * 0.35);
    partInfo->setMinimumHeight(KServer::GetScreenRect().height() * 0.35);
    partInfo->setMinimumWidth(KServer::GetScreenRect().width() * 0.45);
    partInfo->setMinimumWidth(KServer::GetScreenRect().width() * 0.45);

    gLayout->addWidget(partInfo, 6, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);



//    QVBoxLayout* ckboxlayout = new QVBoxLayout;
//    ckboxlayout->addStretch();
//    ckboxlayout->setContentsMargins(0, 0, 0, 0);
//    ckboxlayout->setSpacing(0);
    m_tipCheck = new QCheckBox();
//    ckboxlayout->addWidget(m_tipCheck, Qt::AlignCenter);

//    gLayout->addLayout(ckboxlayout, 8, 1, Qt::AlignCenter);
    gLayout->addWidget(m_tipCheck, 7, 1, Qt::AlignCenter);
    gLayout->setRowStretch(8, 2);
    translateStr();
}

void PrepareInstallFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
        if(w != nullptr) {
            w->deleteLater();
        }
    } else {
        QWidget::changeEvent(event);
    }
}

void PrepareInstallFrame::clickNextButton()
{
    if(!m_tipCheck->checkState()) {
        qDebug() << "clickNextButton0:" << m_tipCheck->checkState();
        if(w != nullptr) {
            w->deleteLater();

        }
        w = new ArrowWidget(this);
        w->setObjectName("ArrowWidget");
        w->setDerection(ArrowWidget::Derection::down);
        w->setWindowFlags(w->windowFlags() | Qt::Dialog);
        w->setText(tr("Check it and click [Start Installation]"));//        w->setCenterWidget(textLabel);
        QPoint pos = m_tipCheck->mapToGlobal(QPoint());

        w->move(pos.x() - 20, pos.y() - w->height());
        w->show();
        QTimer::singleShot(1000, [=]{w->close();} );
        connect(m_tipCheck, &QCheckBox::clicked, [=]{w->close();});
        return;
    } else {
        qDebug() << "clickNextButton1:" << m_tipCheck->checkState();
    }

    switch (m_InstallBtnState) {
    case InstallBtn::QuikPart:
        emit signalQuickPart();
        break;
    case InstallBtn::CoexistPart:
        break;
    case InstallBtn::CustomPart:
        emit signalCustomPart();
        break;
    default:
        break;
    }
    if(w != nullptr) {
        w->deleteLater();

    }

//    emit signalNextStartInstalling();
    qDebug() << "clickNextButton:";

    //this->releaseKeyboard();
}
void PrepareInstallFrame::translateStr()
{
//    tiplabel->setText(tr("Please back up the data and confirm the follow-up operation"));//请备份好数据，然后确认后续操作
//    partLabel->setText(tr("The following partitions will be operated:"));
    //以上分区将被格式化
    m_nextBtn->setText(tr("Start Installation"));

}

void PrepareInstallFrame::initAllConnect()
{

//[=](QString str){emit signalInstallError(str); }
    connect(m_tipCheck, &QCheckBox::stateChanged, this, &PrepareInstallFrame::flushNextButton);
    connect(this, &PrepareInstallFrame::enterKeyPressed, this, &PrepareInstallFrame::clickNextButton);
}

void PrepareInstallFrame::getProcessThread(int exitcode)
{
    if(exitcode == 0) {
        emit signalInstalScriptlFinished();
    } else
        emit signalInstallError(QString("%1").arg(exitcode));
}

void PrepareInstallFrame::flushNextButton()
{
    if(m_tipCheck->isChecked()) {
        if(w != nullptr) {
            w->close();

        }
        m_nextBtn->setStyleSheet("background: #1D8EFF;");
         m_nextBtn->setEnabled(true);
    } else {
        m_nextBtn->setEnabled(false);
        m_nextBtn->setStyleSheet("background: #2D6BAA;");
    }
}


void PrepareInstallFrame::showLicenseFrame()
{
   qDebug() << "show license Frame";
}

void PrepareInstallFrame::addStyleSheet()
{
    QFile file(":/res/qss/PrepareInstallFrame.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

void PrepareInstallFrame::keyPressEvent(QKeyEvent *event)
{
    this->setFocus();
    qDebug() << Q_FUNC_INFO << event->key() << "key pressed!, prepare installl";
    if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterKeyPressed();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
    }
    else if(event->key() == Qt::Key_Escape || event->key() == Qt::Key_Control)
    {
        this->setFocus();
    }
    else
        QWidget::keyPressEvent(event);
}
}
