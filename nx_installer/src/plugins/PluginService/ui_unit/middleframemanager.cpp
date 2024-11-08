#include "middleframemanager.h"

#include <QGridLayout>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QEvent>
#include <QLibrary>
#include <QStringList>
#include <QDir>
#include <QWidgetList>
#include <QKeyEvent>

namespace KInstaller {

MiddleFrameManager::MiddleFrameManager(QWidget *parent) : QWidget(parent)
{
    this->initUI();
    this->initAllConnect();
    this->addStyleSheet();
}

MiddleFrameManager::~MiddleFrameManager()
{

}

void MiddleFrameManager::initUI()
{
    //中间整个窗体布局
    QGridLayout* gLayout = new QGridLayout(this);
    gLayout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(gLayout);
    gLayout->setRowStretch(0, 1);
    gLayout->setRowStretch(1, 1);

    m_Widget = new QWidget();
    gLayout->addWidget(m_Widget, 0, 0, 1, 1);

    m_nextBtn = new QPushButton;
    m_nextBtn->setObjectName("nextBtn");
//    m_nextBtn->setFocusPolicy(Qt::StrongFocus);
    gLayout->addWidget(m_nextBtn, 1, 0, 1, 1, Qt::AlignHCenter);
    translateStr();

}
void MiddleFrameManager::translateStr()
{
    m_nextBtn->setText(tr("next"));
}

void MiddleFrameManager::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        translateStr();
    } else {
        QWidget::changeEvent(event);
    }
}


void MiddleFrameManager::initAllConnect()
{
    QObject::connect(m_nextBtn, &QPushButton::clicked, this, &MiddleFrameManager::clickNextButton);
//    QObject::connect(m_preBtn, &QPushButton::clicked, this, &MiddleFrameManager::clickPrevButton);
    QObject::connect(this, &MiddleFrameManager::enterpressed, this, &MiddleFrameManager::clickNextButton);
}

void MiddleFrameManager::addStyleSheet()
{
    QFile file(":/data/qss/MiddleFrameManager.css");
    file.open(QFile::ReadOnly);
    QTextStream filetext(&file);
    QString stylesheet = filetext.readAll();
    file.close();

    this->setStyleSheet(stylesheet);
}



void MiddleFrameManager::keyPressEvent(QKeyEvent *event)
{
    qDebug() << Q_FUNC_INFO << "key pressed in middleframemanager!";
    if(event->key() == Qt::Key_Enter)
    {
        clickNextButton();
    }
    else {
        QWidget::keyPressEvent(event);
    }
}

}
