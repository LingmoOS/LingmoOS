#include "zipfileprocessresultwidget.h"
#include <utils/optionsmanager.h>
#include <net/helper/transferhepler.h>
#include <common/commonutils.h>
#include "../type_defines.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QDebug>
#include <QLabel>
#include <QDesktopServices>
#include <QApplication>

ZipFileProcessResultWidget::ZipFileProcessResultWidget(QWidget *parent) : QFrame(parent)
{
    initUI();
}

ZipFileProcessResultWidget::~ZipFileProcessResultWidget() { }

void ZipFileProcessResultWidget::initUI()
{
    setStyleSheet("background-color: white; border-radius: 10px;");

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    setLayout(mainLayout);

    successed();
    exitButton = new QToolButton(this);
    exitButton->setText(tr("Exit"));
    exitButton->setFixedSize(120, 35);
    exitButton->setStyleSheet(".QToolButton{border-radius: 8px;"
                              "border: 1px solid rgba(0,0,0, 0.03);"
                              "opacity: 1;"
                              "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                              "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                              "font-family: \"SourceHanSansSC-Medium\";"
                              "font-size: 14px;"
                              "font-weight: 500;"
                              "color: rgba(65,77,104,1);"
                              "font-style: normal;"
                              "text-align: center;"
                              "}");

    QObject::connect(exitButton, &QToolButton::clicked, qApp, &QApplication::quit);
    QToolButton *backButton = new QToolButton(this);
    backButton->setText(tr("Back"));
    backButton->setFixedSize(120, 35);
    backButton->setStyleSheet(".QToolButton{border-radius: 8px;"
                              "border: 1px solid rgba(0,0,0, 0.03);"
                              "opacity: 1;"
                              "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                              "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                              "font-family: \"SourceHanSansSC-Medium\";"
                              "font-size: 14px;"
                              "font-weight: 500;"
                              "color: rgba(65,77,104,1);"
                              "font-style: normal;"
                              "text-align: center;"
                              "}");
    QObject::connect(backButton, &QToolButton::clicked, this,
                     &ZipFileProcessResultWidget::backPage);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(backButton);
    buttonLayout->addWidget(exitButton);
    buttonLayout->setSpacing(10);
    buttonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(buttonLayout);

    QObject::connect(TransferHelper::instance(), &TransferHelper::zipTransferContent, this,
                     &ZipFileProcessResultWidget::upWidgetToFailed);
}

void ZipFileProcessResultWidget::successed()
{
    icon = new QLabel(this);
    icon->setPixmap(QIcon(":/icon/success-128.svg").pixmap(128, 128));
    icon->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    tipLabel1 = new QLabel(tr("Back up succeed"), this);
    QFont font;
    font.setPixelSize(24);
    font.setWeight(QFont::DemiBold);
    tipLabel1->setFont(font);
    tipLabel1->setAlignment(Qt::AlignTop | Qt::AlignHCenter);
    tipLabel2 = new QLabel(tr("Congratulations, Your Information has been Successfully Backed Up."),
                           this);
    tipLabel2->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QString display = QString("<a href=\"https://\" style=\"text-decoration:none;\">%1</a>")
                              .arg(tr("Go to View"));
    displayLabel = new QLabel(display, this);
    displayLabel->setAlignment(Qt::AlignCenter);
    QObject::connect(displayLabel, &QLabel::linkActivated, this,
                     &ZipFileProcessResultWidget::informationPage);

    ((QHBoxLayout *)(this->layout()))->addSpacing(100);
    this->layout()->addWidget(icon);
    ((QHBoxLayout *)(this->layout()))->addSpacing(10);
    this->layout()->addWidget(tipLabel1);
    ((QHBoxLayout *)(this->layout()))->addSpacing(10);
    this->layout()->addWidget(tipLabel2);
    ((QHBoxLayout *)(this->layout()))->addSpacing(100);
    this->layout()->addWidget(displayLabel);
}

void ZipFileProcessResultWidget::upWidgetToFailed(const QString &content, int progressbar,
                                                  int estimatedtime)
{
    Q_UNUSED(estimatedtime)
    if (progressbar != -1) {
        return;
    }
    displayLabel->setVisible(false);
    icon->setPixmap(QIcon(":/icon/fail.svg").pixmap(128, 128));
    tipLabel1->setText(tr("Back up failed"));
    tipLabel2->setText(deepin_cross::CommonUitls::elidedText(content, Qt::ElideMiddle,100));
    exitButton->setStyleSheet(".QToolButton{border-radius: 8px;"
                              "border: 1px solid rgba(0,0,0, 0.03);"
                              "opacity: 1;"
                              "background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 "
                              "rgba(230, 230, 230, 1), stop:1 rgba(227, 227, 227, 1));"
                              "font-family: \"SourceHanSansSC-Medium\";"
                              "font-size: 14px;"
                              "font-weight: 500;"
                              "color: rgba(65,77,104,1);"
                              "font-style: normal;"
                              "text-align: center;"
                              "}");
}

void ZipFileProcessResultWidget::backPage()
{
    emit TransferHelper::instance()->changeWidget(PageName::choosewidget);
}

void ZipFileProcessResultWidget::informationPage()
{
    QString folderPath = OptionsManager::instance()->getUserOption(Options::kBackupFileSavePath)[0];
    QDesktopServices::openUrl(QUrl::fromLocalFile(folderPath));
}
