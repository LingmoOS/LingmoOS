#include "promptwidget.h"
#include "../type_defines.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QCheckBox>
#include <QTextBrowser>
#include <net/helper/transferhepler.h>

PromptWidget::PromptWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

PromptWidget::~PromptWidget() {}

void PromptWidget::initUI()
{
    setStyleSheet(".PromptWidget{background-color: white; border-radius: 10px;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);
    mainLayout->setSpacing(0);

    QLabel *titileLabel = new QLabel(tr("Before tranfer"), this);
    StyleHelper::setAutoFont(titileLabel, 17, QFont::DemiBold);

    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QStringList prompts { tr("Data transfer requires some time, to avoid interrupting the migration "
                             "due to low battery, please keep connect to the  power."),
                          tr("Other applications may slowdown the transfer speed. For smoother "
                             "experience, please close other applications."),
                          tr("For the security of your transfer, please use a trusted network.") };

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    for (int i = 0; i < prompts.count(); i++) {
        QLabel *iconlabel = new QLabel(this);
        iconlabel->setPixmap(QIcon(":/icon/dialog-warning.svg").pixmap(14, 14));

        QLabel *textlabel = new QLabel(prompts[i], this);
        StyleHelper::setAutoFont(textlabel, 14, QFont::Normal);
        textlabel->setWordWrap(true);
        textlabel->setFixedSize(500, 50);
        gridLayout->addWidget(iconlabel, i, 0);
        gridLayout->addWidget(textlabel, i, 1);
        gridLayout->setHorizontalSpacing(10);
        gridLayout->setVerticalSpacing(10);
    }

    QHBoxLayout *promptLayout = new QHBoxLayout();
    promptLayout->addSpacing(150);
    promptLayout->addLayout(gridLayout);

    ButtonLayout *buttonLayout = new ButtonLayout();
    QPushButton *backButton = buttonLayout->getButton1();
    backButton->setText(tr("Back"));
    QPushButton *nextButton = buttonLayout->getButton2();
    nextButton->setText(tr("Confirm"));

    connect(backButton, &QToolButton::clicked, this, &PromptWidget::backPage);
    connect(nextButton, &QToolButton::clicked, this, &PromptWidget::nextPage);

    mainLayout->addSpacing(30);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(promptLayout);
    mainLayout->addSpacing(220);
    mainLayout->addLayout(buttonLayout);
}

void PromptWidget::nextPage()
{
#ifdef _WIN32
    emit TransferHelper::instance()->changeWidget(PageName::readywidget);
#else
    emit TransferHelper::instance()->changeWidget(PageName::connectwidget);

#endif
}

void PromptWidget::backPage()
{
    emit TransferHelper::instance()->changeWidget(PageName::choosewidget);
}

void PromptWidget::themeChanged(int theme)
{
    // light
    if (theme == 1) {
        setStyleSheet(".PromptWidget{background-color: white; border-radius: 10px;}");

    } else {
        // dark
        setStyleSheet(".PromptWidget{background-color: rgb(37, 37, 37); border-radius: 10px;}");
    }
}
