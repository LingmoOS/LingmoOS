#include "dock_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QMouseEvent>
#include <QProcess>
#include <QTimer>

DockPage::DockPage(QWidget *parent)
    : QWidget(parent)
{
    process = new QProcess(this);
    setupUI();
}

void DockPage::setupUI()
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
    cardLayout->setSpacing(30);  // 增加间距

    // 创建标题
    titleLabel = new QLabel(tr("Dock Style"), this);
    titleLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #1a73e8;"
        "background: transparent;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    // 创建选项容器
    QVBoxLayout *optionsLayout = new QVBoxLayout;
    optionsLayout->setSpacing(15);  // 选项之间的间距
    optionsLayout->setContentsMargins(60, 0, 60, 0);  // 左右留出更多空间

    // 添加铺满选项
    fullDockCard = createDockCard(tr("Full"), "", true);
    optionsLayout->addWidget(fullDockCard);

    // 添加居中选项
    centerDockCard = createDockCard(tr("Center"), "", false);
    optionsLayout->addWidget(centerDockCard);

    cardLayout->addLayout(optionsLayout);

    // 添加弹性空间
    cardLayout->addStretch();

    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setContentsMargins(0, 20, 0, 0);  // 增加上边距
    
    // 添加返回按钮
    backButton = new RippleButton(tr("Back"), this);
    backButton->setStyleSheet(
        "RippleButton {"
        "    background-color: #f0f0f0;"
        "    color: #1a73e8;"
        "    border: none;"
        "    border-radius: 8px;"
        "    padding: 12px 24px;"
        "    font-size: 16px;"
        "    min-width: 120px;"
        "}"
        "RippleButton:hover {"
        "    background-color: #e5e5e5;"
        "}"
    );
    backButton->setCursor(Qt::PointingHandCursor);
    connect(backButton, &RippleButton::clicked, this, &DockPage::backClicked);

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
        "RippleButton:disabled {"
        "    background-color: #cccccc;"
        "    color: #666666;"
        "}"
    );
    nextButton->setCursor(Qt::PointingHandCursor);
    nextButton->setEnabled(true);
    connect(nextButton, &RippleButton::clicked, this, &DockPage::nextClicked);

    buttonLayout->addWidget(backButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(nextButton);

    cardLayout->addLayout(buttonLayout);

    // 添加阴影效果
    mainCard->setGraphicsEffect(createShadowEffect());
    
    mainLayout->addWidget(mainCard, 0, Qt::AlignCenter);
}

QFrame* DockPage::createDockCard(const QString &title, const QString &imagePath, bool isFull)
{
    QFrame *card = new QFrame(this);
    card->setObjectName(isFull ? "FullDockCard" : "CenterDockCard");
    
    // 获取当前主题
    bool isLight = mainCard->styleSheet().contains("background-color: white");
    
    card->setStyleSheet(
        QString("QFrame#%1 {"
        "    background-color: %2;"
        "    border-radius: 12px;"
        "    border: none;"
        "}"
        "QFrame#%1:hover {"
        "    background-color: %3;"
        "}")
        .arg(card->objectName())
        .arg(isLight ? "#f5f5f5" : "#383838")
        .arg(isLight ? "#eeeeee" : "#404040")
    );
    
    QHBoxLayout *layout = new QHBoxLayout(card);
    layout->setContentsMargins(25, 0, 25, 0);
    layout->setSpacing(20);
    
    // 添加状态图标
    QLabel *iconLabel = new QLabel(card);
    iconLabel->setObjectName("IconLabel");
    iconLabel->setFixedSize(24, 24);
    iconLabel->setStyleSheet(
        "QLabel#IconLabel {"
        "    background: transparent;"  // 移除初始图标
        "}"
    );
    layout->addWidget(iconLabel);
    
    // 添加标题
    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    titleLabel->setObjectName("TitleLabel");
    
    layout->addWidget(titleLabel, 1);
    
    card->setFixedHeight(65);
    
    card->installEventFilter(this);
    card->setCursor(Qt::PointingHandCursor);
    
    return card;
}

void DockPage::executeDockCommands(bool isFull)
{
    // 使用 qdbus 设置 Dock 样式
    process->start("qdbus", QStringList() 
                  << "com.lingmo.Dock"  // 服务名
                  << "/Dock"            // 对象路径
                  << "com.lingmo.Dock.setStyle"  // 方法名
                  << QString::number(isFull ? 1 : 0));  // 参数：1表示铺满，0表示居中
    process->waitForFinished();
}

QGraphicsEffect* DockPage::createShadowEffect()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 2);
    return shadow;
}

bool DockPage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == fullDockCard) {
            updateDockSelection(true);
            return true;
        } else if (watched == centerDockCard) {
            updateDockSelection(false);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void DockPage::updateDockSelection(bool isFull)
{
    isFullSelected = isFull;
    bool isLight = mainCard->styleSheet().contains("background-color: white");

    // 更新铺满样式卡片
    QString fullStyle = QString(
        "QFrame#FullDockCard {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 2px solid %2;"
        "}"
        "QFrame#FullDockCard:hover {"
        "    background-color: %3;"
        "}"
        "QLabel#TitleLabel {"
        "    color: %4;"
        "    font-size: 18px;"
        "    font-weight: 500;"
        "    background: transparent;"
        "}"
        "QLabel#IconLabel {"
        "    image: url(%5);"
        "    background: transparent;"
        "    min-width: 24px;"
        "    min-height: 24px;"
        "}")
        .arg(isFullSelected ? (isLight ? "#e8f0fe" : "#404040") : (isLight ? "#f5f5f5" : "#383838"))
        .arg(isFullSelected ? "#1a73e8" : "transparent")
        .arg(isLight ? "#eeeeee" : "#404040")
        .arg(isLight ? "#202124" : "white")
        .arg(isFullSelected ? ":/images/check.svg" : "");

    // 更新居中样式卡片
    QString centerStyle = QString(
        "QFrame#CenterDockCard {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 2px solid %2;"
        "}"
        "QFrame#CenterDockCard:hover {"
        "    background-color: %3;"
        "}"
        "QLabel#TitleLabel {"
        "    color: %4;"
        "    font-size: 18px;"
        "    font-weight: 500;"
        "    background: transparent;"
        "}"
        "QLabel#IconLabel {"
        "    image: url(%5);"
        "    background: transparent;"
        "    min-width: 24px;"
        "    min-height: 24px;"
        "}")
        .arg(!isFullSelected ? (isLight ? "#e8f0fe" : "#404040") : (isLight ? "#f5f5f5" : "#383838"))
        .arg(!isFullSelected ? "#1a73e8" : "transparent")
        .arg(isLight ? "#eeeeee" : "#404040")
        .arg(isLight ? "#202124" : "white")
        .arg(!isFullSelected ? ":/images/check.svg" : "");

    fullDockCard->setStyleSheet(fullStyle);
    centerDockCard->setStyleSheet(centerStyle);

    // 执行命令
    QTimer::singleShot(100, this, [this, isFull]() {
        executeDockCommands(isFull);
    });
}

void DockPage::applyTheme(bool isLight)
{
    // 设置背景
    setStyleSheet(
        QString("DockPage {"
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

    // 设置返回按钮
    backButton->setStyleSheet(
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
        .arg(isLight ? "#f0f0f0" : "#404040")
        .arg(isLight ? "#1a73e8" : "#8ab4f8")
        .arg(isLight ? "#e5e5e5" : "#505050")
    );

    // 设置下一步按钮
    nextButton->setStyleSheet(
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
        "}"
        "RippleButton:disabled {"
        "    background-color: %4;"
        "    color: %5;"
        "}")
        .arg(isLight ? "#1a73e8" : "#8ab4f8")
        .arg(isLight ? "white" : "#2d2d2d")
        .arg(isLight ? "#1557b0" : "#99c0ff")
        .arg(isLight ? "#cccccc" : "#404040")
        .arg(isLight ? "#666666" : "#666666")
    );

    // 更新选项卡片样式
    QString fullStyle = QString(
        "QFrame#FullDockCard {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 2px solid %2;"
        "}"
        "QFrame#FullDockCard:hover {"
        "    background-color: %3;"
        "}"
        "QLabel#TitleLabel {"
        "    color: %4;"
        "    font-size: 18px;"
        "    font-weight: 500;"
        "    background: transparent;"
        "}"
        "QLabel#IconLabel {"
        "    image: url(%5);"
        "    background: transparent;"
        "    min-width: 24px;"
        "    min-height: 24px;"
        "}")
        .arg(isFullSelected ? (isLight ? "#e8f0fe" : "#404040") : (isLight ? "#f5f5f5" : "#383838"))
        .arg(isFullSelected ? "#1a73e8" : "transparent")
        .arg(isLight ? "#eeeeee" : "#404040")
        .arg(isLight ? "#202124" : "white")
        .arg(isFullSelected ? ":/images/check.svg" : "");

    QString centerStyle = QString(
        "QFrame#CenterDockCard {"
        "    background-color: %1;"
        "    border-radius: 12px;"
        "    border: 2px solid %2;"
        "}"
        "QFrame#CenterDockCard:hover {"
        "    background-color: %3;"
        "}"
        "QLabel#TitleLabel {"
        "    color: %4;"
        "    font-size: 18px;"
        "    font-weight: 500;"
        "    background: transparent;"
        "}"
        "QLabel#IconLabel {"
        "    image: url(%5);"
        "    background: transparent;"
        "    min-width: 24px;"
        "    min-height: 24px;"
        "}")
        .arg(!isFullSelected ? (isLight ? "#e8f0fe" : "#404040") : (isLight ? "#f5f5f5" : "#383838"))
        .arg(!isFullSelected ? "#1a73e8" : "transparent")
        .arg(isLight ? "#eeeeee" : "#404040")
        .arg(isLight ? "#202124" : "white")
        .arg(!isFullSelected ? ":/images/check.svg" : "");

    fullDockCard->setStyleSheet(fullStyle);
    centerDockCard->setStyleSheet(centerStyle);
}