#include "welcome_window.h"
#include <QApplication>
#include <QScreen>
#include <QFont>
#include <QPainter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include "ripple_button.h"
#include <QSizePolicy>

WelcomeWindow::WelcomeWindow(QWidget *parent)
    : QWidget(parent)
{
    // 设置窗口背景色和大小策略
    setStyleSheet("QWidget { background-color: #f8f9fa; }");
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    setupUI();
}

void WelcomeWindow::setupUI()
{
    // 创建主布局
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    
    // 设置窗口背景图片
    setStyleSheet(QString(
        "WelcomeWindow {"
        "    background-image: url(:/images/default.jpg);"
        "    background-position: center;"
        "    background-repeat: no-repeat;"
        "    background-attachment: fixed;"
        "    background-size: cover;"
        "}"
    ));
    
    // 创建主卡片
    mainCard = new QFrame(this);
    mainCard->setObjectName("MainCard");
    mainCard->setStyleSheet(
        "QFrame#MainCard {"
        "    background-color: white;"
        "    border-radius: 16px;"
        "    border: 1px solid #e0e0e0;"
        "}"
    );
    
    // 设置卡片固定大小
    mainCard->setFixedSize(800, 500);
    
    // 创建卡片内的布局
    QVBoxLayout *cardLayout = new QVBoxLayout(mainCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(25);

    // 创建标题
    titleLabel = new QLabel(tr("Welcome to LingmoOS"), this);
    titleLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #1a73e8;"
        "background: transparent;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    // 创建副标题
    subtitleLabel = new QLabel(
        tr("Thank you for choosing LingmoOS! Let's start exploring this beautiful and efficient operating system."), this);
    subtitleLabel->setStyleSheet(
        "font-size: 16px;"
        "color: #5f6368;"
        "background: transparent;"
    );
    subtitleLabel->setWordWrap(true);
    subtitleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(subtitleLabel);

    // 添加图标
    QLabel *iconLabel = new QLabel(this);
    QPixmap icon(":/images/pre.jpg");
    iconLabel->setPixmap(icon.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setStyleSheet("background: transparent;");
    cardLayout->addWidget(iconLabel);

    // 添加弹性空间
    cardLayout->addStretch();

    // 创建下一步按钮
    nextButton = new RippleButton(tr("Next"), this);
    nextButton->setStyleSheet(
        "RippleButton {"
        "    background-color: #1a73e8;"
        "    color: white;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 16px;"
        "    min-width: 120px;"
        "}"
        "RippleButton:hover {"
        "    background-color: #1557b0;"
        "}"
        "RippleButton:pressed {"
        "    background-color: #0d47a1;"
        "}"
    );
    nextButton->setCursor(Qt::PointingHandCursor);
    
    // 连接按钮点击到信号
    connect(nextButton, &RippleButton::clicked, this, &WelcomeWindow::nextClicked);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(nextButton);
    cardLayout->addLayout(buttonLayout);

    // 添加阴影效果
    mainCard->setGraphicsEffect(createShadowEffect());
    
    // 将主卡片添加到主布局并居中
    mainLayout->addWidget(mainCard, 0, Qt::AlignCenter);
}

QGraphicsEffect* WelcomeWindow::createShadowEffect()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 2);
    return shadow;
} 