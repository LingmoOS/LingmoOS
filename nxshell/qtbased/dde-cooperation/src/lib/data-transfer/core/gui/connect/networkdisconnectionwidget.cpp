#include "networkdisconnectionwidget.h"

#include <QLabel>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QDebug>
#include "../type_defines.h"
#include <net/helper/transferhepler.h>
NetworkDisconnectionWidget::NetworkDisconnectionWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

NetworkDisconnectionWidget::~NetworkDisconnectionWidget()
{
}

void NetworkDisconnectionWidget::initUI()
{
    setStyleSheet(".NetworkDisconnectionWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);
    mainLayout->addSpacing(30);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/icon/noInternet.png").pixmap(200, 160));
    iconLabel->setAlignment(Qt::AlignHCenter | Qt::AlignBottom);

    QLabel *promptLabel = new QLabel(this);
    promptLabel->setText(tr("The network has been disconnected. Please check your network"));
    promptLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    ButtonLayout *buttonLayout = new ButtonLayout();
    QPushButton *backButton = buttonLayout->getButton1();
    QPushButton *retryButton = buttonLayout->getButton2();
    backButton->setText(tr("Back"));
    retryButton->setText(tr("Try again"));
    retryButton->setStyleSheet(StyleHelper::buttonStyle(StyleHelper::gray));

    connect(backButton, &QPushButton::clicked, this, &NetworkDisconnectionWidget::backPage);
    connect(retryButton, &QPushButton::clicked, this, &NetworkDisconnectionWidget::retryPage);

    IndexLabel *indelabel = new IndexLabel(3, this);
    indelabel->setAlignment(Qt::AlignCenter);

    QHBoxLayout *indexLayout = new QHBoxLayout();
    indexLayout->addWidget(indelabel, Qt::AlignCenter);

    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(iconLabel);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(promptLabel);
    mainLayout->addSpacing(170);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(indexLayout);
}

void NetworkDisconnectionWidget::backPage()
{
    emit TransferHelper::instance()->changeWidget(PageName::choosewidget);
}

void NetworkDisconnectionWidget::retryPage()
{
    emit TransferHelper::instance()->changeWidget(PageName::choosewidget);
}

void NetworkDisconnectionWidget::themeChanged(int theme)
{
    //light
    if (theme == 1) {
        setStyleSheet(".NetworkDisconnectionWidget{background-color: white; border-radius: 10px;}");
    } else {
        setStyleSheet(".NetworkDisconnectionWidget{background-color: rgb(37, 37, 37); border-radius: 10px;}");
        //dark
    }
}
