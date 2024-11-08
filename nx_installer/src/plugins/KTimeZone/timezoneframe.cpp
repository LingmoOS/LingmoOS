#include "timezoneframe.h"
#include <QWidget>
#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QDebug>
#include <QListView>
#include <QFile>
#include <QTextStream>
#include "../PluginService/ksystemsetting_unit.h"
#include "timezonemodel.h"
#include "../PluginService/ui_unit/comboxdelegate.h"

using namespace KServer;

namespace KInstaller {

TimeZoneFrame::TimeZoneFrame(QWidget *parent) : MiddleFrameManager(parent)
{
    initUI();
    addStyleSheet();
    initAllConnect();
    initComboxZone();
    setBoardTabOrder();
    translateStr();
    this->setFocus();
}

TimeZoneFrame::~TimeZoneFrame()
{

}

void TimeZoneFrame::initComboxZone()
{
    m_comboxZone->clear();
    m_comboxZone->setInsertPolicy(QComboBox::InsertAtBottom);
    m_zoneModel = new TimeZoneModel(m_comboxZone);
    m_comboxZone->setModel(m_zoneModel);
#if 0 //时区与语言绑定
//    int index = m_zoneModel->getIndexByCountryCode(readSettingIni()).row();
//    m_curzoneItem = m_zoneModel->getLatLongByTZCity(m_zoneModel->getIndexByCountryCode(readSettingIni()));
#endif
#if 1 //初始时区固定为北京
    int index = m_zoneModel->getIndexByCountryCode("CN").row();
    m_curzoneItem = m_zoneModel->getLatLongByTZCity(m_zoneModel->getIndexByCountryCode("CN"));
#endif
    m_comboxZone->setCurrentIndex(index);
    m_comboxZone->setMaxVisibleItems(5);
    m_mapWid->setZoneList(m_zoneModel->getZoneList());
    emit signalLanguageSetZone(m_curzoneItem);
}

void TimeZoneFrame::setBoardTabOrder()
{
    QWidget::setTabOrder(m_comboxZone, m_nextBtn);
}

void TimeZoneFrame::initAllConnect()
{
    connect(m_mapWid, &MapWidget::signalCurrentZoneChanged, this, &TimeZoneFrame::showCurrentZone);
    connect(this, &TimeZoneFrame::signalLanguageSetZone, m_mapWid, &MapWidget::showLanguageSetZone);
    connect(m_comboxZone, SIGNAL(currentIndexChanged(int)), this, SLOT(showCurrentZoneIndex(int)));
//    [=](int index){
//        m_zoneModel->setCurZone(index);
//        emit signalLanguageSetZone(m_zoneModel->getcurTimeZoneStruct());
//    });
}

void TimeZoneFrame::showCurrentZoneIndex(int index)
{
    m_zoneModel->setCurZone(index);
    m_curzoneItem = m_zoneModel->getcurTimeZoneStruct();
    emit signalLanguageSetZone(m_curzoneItem);
}

void TimeZoneFrame::showCurrentZone(TimeZoneStruct zoneItem)
{
    m_curzoneItem = zoneItem;
    m_comboxZone->setCurrentIndex(m_zoneModel->getIndexByTZCity(zoneItem.TZcity).row());
}

void TimeZoneFrame::initUI()
{
    QGridLayout* gLayout = new QGridLayout();
    gLayout->setContentsMargins(0, 0, 0, 0);
   // widget->setGeometry(0, 0, this->width(), this->height());
    m_Widget->setLayout(gLayout);
    gLayout->setColumnStretch(0, 3);
    gLayout->setColumnStretch(1, 2);
    gLayout->setColumnStretch(2, 3);

    m_mainTitle = new QLabel(this);
    m_mainTitle->setObjectName("mainTitle");

    gLayout->addItem(new QSpacerItem(10, 40, QSizePolicy::Expanding, QSizePolicy::Preferred), 0, 1, 1, 1);
    gLayout->addWidget(m_mainTitle, 1, 1, 1, 1, Qt::AlignCenter | Qt::AlignTop);
    gLayout->addItem(new QSpacerItem(10, 32, QSizePolicy::Expanding, QSizePolicy::Preferred), 2, 1, 1, 1);

    m_comboxZone = new QComboBox(this);
    m_comboxZone->setObjectName("comboxZone");
    m_comboxZone->setView(new QListView());
    m_comboxZone->view()->window()->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    m_comboxZone->view()->window()->setAttribute(Qt::WA_TranslucentBackground);
    gLayout->addWidget(m_comboxZone, 3, 1, 1, 1, Qt::AlignCenter);
    gLayout->addItem(new QSpacerItem(10, 20, QSizePolicy::Expanding, QSizePolicy::Preferred), 4, 1, 1, 1);

    m_mapWid = new MapWidget();
    m_mapWid->setParent(this);
    gLayout->addWidget(m_mapWid, 5, 1, 1, 1, Qt::AlignCenter);
    gLayout->setRowStretch(6, 1);

}

void TimeZoneFrame::translateStr()
{
    m_mainTitle->setText(tr("Select Timezone"));
    m_nextBtn->setText(tr("Next"));
//    m_zoneModel->blockSignals(true);
    m_zoneModel->flushZone();
    m_mapWid->m_TZLists = loadZoneFile();
//    m_zoneModel->blockSignals(false);
}

void TimeZoneFrame::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}

void TimeZoneFrame::addStyleSheet()
{
    QFile file(":/res/qss/KTimeZone.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = QString("");
    stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}

QString TimeZoneFrame::readSettingIni()
{
    QString s = KServer::ReadSettingIni("config", "language");
    return s.split('_').at(1);
}

void TimeZoneFrame::writeSettingIni()
{
    if(!m_curzoneItem.zone.isEmpty() && !m_curzoneItem.TZcity.isEmpty()) {
        QString tmpzone = "";
        tmpzone = QString("%1/%2").arg(m_curzoneItem.zone).arg(m_curzoneItem.encity);
        WriteSettingToIni("config", "timezone", tmpzone);
    }
}

void TimeZoneFrame::clickNextButton()
{
    writeSettingIni();
    emit signalStackPageChanged(1);
}

void TimeZoneFrame::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << event->key() << "key pressed!";
    if(event->key() == Qt::Key_Up)
    {
        if(m_comboxZone->currentIndex() == 0) {
            m_comboxZone->setCurrentIndex(0);
        } else
            m_comboxZone->setCurrentIndex(m_comboxZone->currentIndex() - 1);
    }
    else if(event->key() == Qt::Key_Down)
    {
        m_comboxZone->setCurrentIndex(m_comboxZone->currentIndex() + 1);
    }
    else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
    {
        emit enterpressed();
        //this->releaseKeyboard();
    }
    else if(event->key() == Qt::Key_Backspace)
    {
        emit backspacepressed();
        qDebug()<<"return to language!";
        //this->releaseKeyboard();
    }
    else if(event->key() == Qt::Key_Escape)
    {
        //this->releaseKeyboard();
    }
    else
        QWidget::keyPressEvent(event);
       // this->releaseKeyboard();
}

}
