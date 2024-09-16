#include "custommessagebox.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>
#include <QPainter>
#include <QPainterPath>

CustomMessageBox::CustomMessageBox(const QString &mainTitle, const QString &subTitle,
                                   QWidget *parent)
    : QDialog(parent), message1(mainTitle), message2(subTitle)
{
    initUi();
}

void CustomMessageBox::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(220, 220, 220));
    painter.setPen(Qt::NoPen);

    QPainterPath path;
    path.addRoundedRect(rect(), 10, 10);
    painter.drawPath(path);
}

void CustomMessageBox::initUi()
{
    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(385, 160);
    setStyleSheet(".CustomMessageBox{"
                  "border-radius: 10px;"
                  "border: none;"
                  "}");
    QLabel *message1Label = new QLabel(message1, this);
    message1Label->setAlignment(Qt::AlignCenter);
    message1Label->setWordWrap(true);
    message1Label->setGeometry(50, 20, 200, 50);
    message1Label->setStyleSheet(".QLabel{"
                                 "opacity:1;"
                                 "color: rgba(0,0,0,0.9);"
                                 "font-family: \"SourceHanSansSC-Medium\";"
                                 "font-size: 14px;"
                                 "font-weight: 500;"
                                 "font-style: normal;"
                                 " text-align: center;}");
    QLabel *message2Label = new QLabel(message2, this);
    message2Label->setAlignment(Qt::AlignCenter);
    message2Label->setWordWrap(true);
    message2Label->setGeometry(50, 20, 200, 50);
    message2Label->setStyleSheet(".QLabel{"
                                 "opacity: 1;"
                                 " color: rgba(0,0,0,0.7);"
                                 "font-family: \"SourceHanSansSC-Medium\";"
                                 "font-size: 14px;"
                                 "font-weight: 400;"
                                 "font-style: normal;"
                                 " text-align: center;}");

    QVBoxLayout *mainLayout = new QVBoxLayout();
    setLayout(mainLayout);

    QToolButton *cancelButton = new QToolButton(this);
    cancelButton->setText(tr("Reselect"));
    cancelButton->setFixedSize(170, 35);
    cancelButton->setStyleSheet(".QToolButton{border-radius: 8px;"
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
                                ";}");
    QToolButton *continueButton = new QToolButton(this);
    continueButton->setText(tr("Continue"));
    continueButton->setFixedSize(170, 35);
    continueButton->setStyleSheet(".QToolButton{"
                                  "border-radius: 8px;"
                                  "border: 1px solid rgba(0,0,0, 0.03);"
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

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addSpacing(10);
    buttonLayout->addWidget(continueButton);

    QLabel *mainLabel = new QLabel(this);
    mainLabel->setStyleSheet("QWidget {"
                             "border-top-left-radius: 20px;"
                             "}");
    mainLabel->setFixedSize(40, 40);

    QPixmap pixmap = QIcon(QString(":/icon/dialog-warning.svg")).pixmap(mainLabel->width(), mainLabel->height());
    mainLabel->setPixmap(pixmap);

    QToolButton *closeButton = new QToolButton(this);
    closeButton->setStyleSheet("QWidget {"
                               "border-top-right-radius: 20px;"
                               "}");
    closeButton->setFixedSize(50, 50);
    closeButton->setIcon(QIcon(":/icon/close_normal.svg"));
    closeButton->setIconSize(QSize(50, 50));

    QHBoxLayout *titleLayout = new QHBoxLayout();

    titleLayout->addWidget(mainLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(closeButton);
    mainLayout->setContentsMargins(5, 0, 0, 5);
    mainLayout->setSpacing(0);
    mainLayout->addLayout(titleLayout);
    mainLayout->addWidget(message1Label);
    mainLayout->addWidget(message2Label);
    mainLayout->addSpacing(20);
    mainLayout->addLayout(buttonLayout);
    QObject::connect(closeButton, &QPushButton::clicked, this, &CustomMessageBox::reject);
    QObject::connect(continueButton, &QPushButton::clicked, this, &CustomMessageBox::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, this, &CustomMessageBox::reject);
}

bool CustomMessageBox::SelectContinueTransfer()
{
    CustomMessageBox messageBox(tr("The presence of outstanding transfer tasks between you and the target device has been detected."), tr("Do you want to continue with the last transfer?"));
    if (messageBox.exec() == QDialog::Accepted)
        return true;
    else
        return false;
}
