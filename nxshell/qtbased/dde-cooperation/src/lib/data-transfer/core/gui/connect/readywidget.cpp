#include "readywidget.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QToolButton>
#include <QStackedWidget>
#include <QCheckBox>
#include <QTextBrowser>
#include <QLineEdit>
#include <QRegularExpressionValidator>
#include <QTimer>

#include <net/helper/transferhepler.h>

ReadyWidget::ReadyWidget(QWidget *parent)
    : QFrame(parent)
{
    initUI();
}

ReadyWidget::~ReadyWidget() {}

void ReadyWidget::clear()
{
    ipInput->clear();
    captchaInput->clear();
    tiptextlabel->setVisible(false);
    setnextButEnable(false);
    tiptextlabel->setStyleSheet(StyleHelper::textStyle(StyleHelper::normal));
    tiptextlabel->setText(tr("connect..."));
}

void ReadyWidget::initUI()
{
    setStyleSheet(".ReadyWidget{background-color: white; border-radius: 10px;}");

    // init timer
    timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(3000);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    QLabel *titileLabel = new QLabel(tr("Ready to connect"), this);
    StyleHelper::setAutoFont(titileLabel, 24, QFont::DemiBold);
    titileLabel->setAlignment(Qt::AlignTop | Qt::AlignCenter);

    QLabel *ipLabel = new QLabel(tr("IP"), this);
    QHBoxLayout *ipLayout = new QHBoxLayout(this);
    ipLayout->addSpacing(190);
    ipLayout->addWidget(ipLabel);
    ipLayout->setAlignment(Qt::AlignBottom);

    ipInput = new QLineEdit(this);
    ipInput->setPlaceholderText(tr("Please input the IP of UOS"));
    ipInput->setStyleSheet("border-radius: 8px;"
                           "opacity: 1;"
                           "padding-left: 10px;"
                           "background-color: rgba(0,0,0, 0.08);");
    ipInput->setFixedSize(340, 36);

    QRegularExpressionValidator *ipValidator = new QRegularExpressionValidator(QRegularExpression(
            "^((\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])\\.){3}(\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])$"));

    ipInput->setValidator(ipValidator);

    QObject::connect(ipInput, &QLineEdit::textChanged, [=]() {
        bool isEmpty = ipInput->text().isEmpty();
        ipInput->setClearButtonEnabled(!isEmpty);
    });
    QHBoxLayout *editLayout1 = new QHBoxLayout(this);
    editLayout1->setAlignment(Qt::AlignCenter);
    editLayout1->addWidget(ipInput);

    QLabel *cue = new QLabel(tr("Please open data transfer on UOS, and get the IP"), this);
    QHBoxLayout *cueLayout = new QHBoxLayout(this);
    cueLayout->addSpacing(190);
    cueLayout->addWidget(cue);
    cueLayout->setAlignment(Qt::AlignTop);

    QLabel *Captcha = new QLabel(tr("Connect code"), this);
    QHBoxLayout *captchaLayout = new QHBoxLayout(this);
    captchaLayout->addSpacing(190);
    captchaLayout->addWidget(Captcha);
    captchaLayout->setAlignment(Qt::AlignTop);

    captchaInput = new QLineEdit(this);
    QRegularExpressionValidator *captchaValidator =
            new QRegularExpressionValidator(QRegularExpression("^\\d{6}$"));
    captchaInput->setValidator(captchaValidator);
    captchaInput->setPlaceholderText(tr("Please input the connect code on UOS"));
    captchaInput->setStyleSheet("border-radius: 8px;"
                                "opacity: 1;"
                                "padding-left: 10px;"
                                "background-color: rgba(0,0,0, 0.08);");
    captchaInput->setFixedSize(340, 36);
    QObject::connect(captchaInput, &QLineEdit::textChanged, [=]() {
        bool isEmpty = captchaInput->text().isEmpty();
        captchaInput->setClearButtonEnabled(!isEmpty);
    });

    QHBoxLayout *editLayout2 = new QHBoxLayout(this);
    editLayout2->setAlignment(Qt::AlignCenter);
    editLayout2->addWidget(captchaInput);

    tiptextlabel = new QLabel(this);
    tiptextlabel->setStyleSheet(StyleHelper::textStyle(StyleHelper::normal));
    tiptextlabel->setText(tr("connect..."));
    tiptextlabel->setVisible(false);
    tiptextlabel->setAlignment(Qt::AlignBottom|Qt::AlignHCenter);

    ButtonLayout *buttonLayout = new ButtonLayout();
    QPushButton *backButton = buttonLayout->getButton1();
    backButton->setText(tr("Back"));
    nextButton = buttonLayout->getButton2();
    nextButton->setText(tr("Connect"));
    nextButton->setEnabled(false);
    setnextButEnable(false);

    connect(nextButton, &QToolButton::clicked, this, &ReadyWidget::tryConnect);
    connect(backButton, &QToolButton::clicked, this, &ReadyWidget::backPage);

    IndexLabel *indelabel = new IndexLabel(1, this);
    indelabel->setAlignment(Qt::AlignCenter);
    QHBoxLayout *indexLayout = new QHBoxLayout(this);
    indexLayout->addWidget(indelabel, Qt::AlignCenter);

    mainLayout->addSpacing(40);
    mainLayout->addWidget(titileLabel);
    mainLayout->addSpacing(30);
    mainLayout->addLayout(ipLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(editLayout1);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(cueLayout);
    mainLayout->addSpacing(50);
    mainLayout->addLayout(captchaLayout);
    mainLayout->addSpacing(10);
    mainLayout->addLayout(editLayout2);
    mainLayout->addSpacing(100);
    mainLayout->addWidget(tiptextlabel);
//    mainLayout->addSpacing(10);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(4);
    mainLayout->addLayout(indexLayout);

    QObject::connect(ipInput, &QLineEdit::textChanged, this, &ReadyWidget::onLineTextChange);
    QObject::connect(captchaInput, &QLineEdit::textChanged, this, &ReadyWidget::onLineTextChange);
    QObject::connect(TransferHelper::instance(), &TransferHelper::connectSucceed, this,
                     &ReadyWidget::nextPage);
    QObject::connect(TransferHelper::instance(), &TransferHelper::connectFailed, this,
                     &ReadyWidget::connectFailed);

    // Connect the timer's timeout signal to hiding the label
    QObject::connect(timer, &QTimer::timeout, [this]() {
        connectFailed();
    });
}

void ReadyWidget::tryConnect()
{
    tiptextlabel->setText(
            QString("<font size='3' color='#000000'>%1</font>").arg(tr("connect...")));
    tiptextlabel->setVisible(true);
    setnextButEnable(false);

    TransferHelper::instance()->tryConnect(ipInput->text(), captchaInput->text());
    timer->start();
}

void ReadyWidget::setnextButEnable(bool enabel)
{
    if (enabel) {
        nextButton->setEnabled(true);
        nextButton->setStyleSheet(".QPushButton{border-radius: 8px;"
                                  "opacity: 1;"
                                  "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                                  "rgba(37, 183, 255, 1), stop:1 rgba(0, 152, 255, 1));"
                                  "font-family: \"SourceHanSansSC-Medium\";"
                                  "font-size: 14px;"
                                  "font-weight: 500;"
                                  "color: rgba(255,255,255,1);"
                                  "font-style: normal;"
                                  "text-align: center;"
                                  "}");
    } else {
        nextButton->setEnabled(false);
        nextButton->setStyleSheet(".QPushButton{border-radius: 8px;"
                                  "opacity: 1;"
                                  "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                                  "rgba(37, 183, 255, 0.6), stop:1 rgba(0, 152, 255, 0.6));"
                                  "font-family: \"SourceHanSansSC-Medium\";"
                                  "font-size: 14px;"
                                  "font-weight: 500;"
                                  "color: rgba(255,255,255,0.6);"
                                  "font-style: normal;"
                                  "text-align: center;"
                                  "}");
    }
}

void ReadyWidget::nextPage()
{
    // tiptextlabel->setVisible(false);
    QStackedWidget *stackedWidget = qobject_cast<QStackedWidget *>(this->parent());
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(stackedWidget->currentIndex() + 1);
    } else {
        WLOG << "Jump to next page failed, qobject_cast<QStackedWidget *>(this->parent()) = "
                "nullptr";
    }

    clear();
}

void ReadyWidget::backPage()
{
    QStackedWidget *stackedWidget = qobject_cast<QStackedWidget *>(this->parent());
    if (stackedWidget) {
        stackedWidget->setCurrentIndex(stackedWidget->currentIndex() - 1);
    } else {
        WLOG << "Jump to next page failed, qobject_cast<QStackedWidget *>(this->parent()) = "
                "nullptr";
    }

    clear();
}

void ReadyWidget::onLineTextChange()
{
    QRegularExpression ipRegex(
            "^((\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])\\.){3}(\\d{1,2}|1\\d{2}|2[0-4]\\d|25[0-5])$");
    QRegularExpressionMatch ipMatch = ipRegex.match(ipInput->text());

    if (!ipMatch.hasMatch()) {
        setnextButEnable(false);
        return;
    }

    QRegularExpression captchaRegex("^\\d{6}$");
    QRegularExpressionMatch captchaMatch = captchaRegex.match(captchaInput->text());

    if (!captchaMatch.hasMatch()) {
        setnextButEnable(false);
        return;
    }

    setnextButEnable(true);
}

void ReadyWidget::connectFailed()
{
    tiptextlabel->setStyleSheet(StyleHelper::textStyle(StyleHelper::error));
    tiptextlabel->setText(tr("Failed to connect, please check your input"));
}
