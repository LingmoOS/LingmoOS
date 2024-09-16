#include "startwidget.h"
#include "../type_defines.h"
#include <net/helper/transferhepler.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QCheckBox>
#include <QTextBrowser>

StartWidget::StartWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

StartWidget::~StartWidget()
{
}

void StartWidget::initUI()
{
    setStyleSheet(".StartWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);

    QLabel *iconLabel = new QLabel(this);
    iconLabel->setPixmap(QIcon(":/icon/picture-home.png").pixmap(200, 160));
    iconLabel->setAlignment(Qt::AlignBottom | Qt::AlignHCenter);

    QLabel *titileLabel = new QLabel(tr("UOS data transfer"), this);
    StyleHelper::setAutoFont(titileLabel, 24, QFont::DemiBold);
    titileLabel->setAlignment(Qt::AlignCenter);

    QLabel *textLabel2 = new QLabel(tr("UOS transfer tool enables one click migration of your files, personal data, and applications to\nUOS, helping you seamlessly replace your system."), this);
    textLabel2->setAlignment(Qt::AlignTop | Qt::AlignCenter);
    StyleHelper::setAutoFont(textLabel2, 14, QFont::Normal);

    ButtonLayout *buttonLayout = new ButtonLayout();
    buttonLayout->setCount(1);
    QPushButton *nextButton = buttonLayout->getButton1();
    nextButton->setText(tr("Next"));
    connect(nextButton, &QPushButton::clicked, this, &StartWidget::nextPage);

    mainLayout->addSpacing(50);
    mainLayout->addWidget(iconLabel);
    mainLayout->addWidget(titileLabel);
    mainLayout->addWidget(textLabel2);
    mainLayout->addSpacing(60);
    mainLayout->addLayout(buttonLayout);
}

void StartWidget::nextPage()
{
    emit TransferHelper::instance()->changeWidget(PageName::choosewidget);
}

void StartWidget::themeChanged(int theme)
{
    //light
    if (theme == 1) {
        setStyleSheet(".StartWidget{background-color: white; border-radius: 10px;}");
    } else {
        //dark
        setStyleSheet(".StartWidget{background-color: rgb(37, 37, 37); border-radius: 10px;}");
    }
}
