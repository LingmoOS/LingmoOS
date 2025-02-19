#include "finish_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QSvgRenderer>
#include <QPainter>

FinishPage::FinishPage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void FinishPage::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(50, 50, 50, 50);
    
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
    
    mainCard->setFixedSize(800, 500);
    
    // 创建卡片内的布局
    QVBoxLayout *cardLayout = new QVBoxLayout(mainCard);
    cardLayout->setContentsMargins(40, 40, 40, 40);
    cardLayout->setSpacing(30);

    // 创建标题
    titleLabel = new QLabel(tr("Setup Complete"), this);
    titleLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #1a73e8;"
        "background: transparent;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    // 添加一个绿色对号
    QLabel *checkLabel = new QLabel(this);
    checkLabel->setFixedSize(120, 120);
    
    // 创建圆形背景
    QPixmap circlePixmap(120, 120);
    circlePixmap.fill(Qt::transparent);
    
    QPainter circlePainter(&circlePixmap);
    circlePainter.setRenderHint(QPainter::Antialiasing);  // 抗锯齿
    
    // 绘制圆形背景
    circlePainter.setPen(Qt::NoPen);
    circlePainter.setBrush(QColor("#34A853"));  // Google 绿色
    circlePainter.drawEllipse(0, 0, 120, 120);
    
    // 创建对号SVG
    QSvgRenderer *renderer = new QSvgRenderer(QString(
        "<svg viewBox='0 0 24 24'>"
        "<path d='M9 16.17L4.83 12l-1.42 1.41L9 19 21 7l-1.41-1.41L9 16.17z' fill='white'/>"
        "</svg>"
    ).toUtf8());
    
    // 在圆形背景上绘制对号
    renderer->render(&circlePainter, QRectF(24, 24, 72, 72));  // 居中绘制对号
    
    // 设置最终图像
    checkLabel->setPixmap(circlePixmap);
    checkLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(checkLabel, 0, Qt::AlignCenter);
    
    // 添加一些垂直间距
    cardLayout->addSpacing(20);

    // 创建消息标签
    messageLabel = new QLabel(tr("Welcome to LingmoOS! The setup is complete, start enjoying the new experience."), this);
    messageLabel->setStyleSheet(
        "font-size: 16px;"
        "color: #202124;"
        "background: transparent;"
    );
    messageLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(messageLabel);

    // 添加弹性空间
    cardLayout->addStretch();

    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 20, 0, 0);

    // 创建完成按钮
    finishButton = new RippleButton(tr("Get Started"), this);
    finishButton->setStyleSheet(
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
    );
    finishButton->setCursor(Qt::PointingHandCursor);
    connect(finishButton, &RippleButton::clicked, this, &FinishPage::finished);

    buttonLayout->addStretch();
    buttonLayout->addWidget(finishButton);

    cardLayout->addLayout(buttonLayout);

    // 添加阴影效果
    mainCard->setGraphicsEffect(createShadowEffect());
    
    mainLayout->addWidget(mainCard, 0, Qt::AlignCenter);
}

QGraphicsEffect* FinishPage::createShadowEffect()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 2);
    return shadow;
}

void FinishPage::applyTheme(bool isLight)
{
    // 设置背景
    setStyleSheet(
        QString("FinishPage {"
        "    background-image: url(:/images/%1);"
        "    background-position: center;"
        "    background-repeat: no-repeat;"
        "    background-attachment: fixed;"
        "}").arg(isLight ? "default.jpg" : "dark-default.jpg")
    );

    // 设置主卡片
    mainCard->setStyleSheet(
        QString("QFrame#MainCard {"
        "    background-color: %1;"
        "    border-radius: 16px;"
        "    border: 1px solid %2;"
        "}").arg(isLight ? "white" : "#2d2d2d")
          .arg(isLight ? "#e0e0e0" : "#404040")
    );

    // 设置标题
    titleLabel->setStyleSheet(
        QString("font-size: 32px;"
        "font-weight: bold;"
        "color: %1;"
        "background: transparent;")
        .arg(isLight ? "#1a73e8" : "#8ab4f8")
    );

    // 设置消息
    messageLabel->setStyleSheet(
        QString("font-size: 16px;"
        "color: %1;"
        "background: transparent;")
        .arg(isLight ? "#202124" : "white")
    );

    // 设置按钮
    finishButton->setStyleSheet(
        QString("RippleButton {"
        "    background-color: %1;"
        "    color: %2;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 16px;"
        "    min-width: 120px;"
        "}"
        "RippleButton:hover {"
        "    background-color: %3;"
        "}")
        .arg(isLight ? "#1a73e8" : "#8ab4f8")
        .arg(isLight ? "white" : "#2d2d2d")
        .arg(isLight ? "#1557b0" : "#99c0ff")
    );
} 