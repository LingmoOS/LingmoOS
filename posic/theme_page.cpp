#include "theme_page.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QEvent>
#include <QMouseEvent>
#include <QProcess>
#include <QTimer>

ThemePage::ThemePage(QWidget *parent)
    : QWidget(parent)
    , selectedTheme("")
{
    process = new QProcess(this);
    setupUI();
}

void ThemePage::setupUI()
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
    cardLayout->setSpacing(25);

    // 创建标题
    titleLabel = new QLabel(tr("Theme Style"), this);
    titleLabel->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #1a73e8;"
        "background: transparent;"
    );
    titleLabel->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(titleLabel);

    // 创建主题卡片容器
    QFrame *themesContainer = new QFrame(this);
    themesContainer->setObjectName("ThemesContainer");
    QHBoxLayout *themesLayout = new QHBoxLayout(themesContainer);
    themesLayout->setSpacing(20);
    themesLayout->setContentsMargins(20, 20, 20, 20);

    // 添加亮色主题卡片
    lightThemeCard = createThemeCard(tr("Light"), ":/images/light-theme.png", true);
    themesLayout->addWidget(lightThemeCard);

    // 添加暗色主题卡片
    darkThemeCard = createThemeCard(tr("Dark"), ":/images/dark-theme.png", false);
    themesLayout->addWidget(darkThemeCard);

    cardLayout->addWidget(themesContainer);

    // 添加按钮布局
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();  // 只需要一个弹性空间

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
    nextButton->setEnabled(false);
    connect(nextButton, &RippleButton::clicked, this, &ThemePage::nextClicked);

    buttonLayout->addWidget(nextButton);

    cardLayout->addLayout(buttonLayout);

    // 添加阴影效果
    mainCard->setGraphicsEffect(createShadowEffect());
    
    // 将主卡片添加到主布局并居中
    mainLayout->addWidget(mainCard, 0, Qt::AlignCenter);
}

QFrame* ThemePage::createThemeCard(const QString &title, const QString &imagePath, bool isLight)
{
    QFrame *card = new QFrame(this);
    card->setObjectName(isLight ? "LightThemeCard" : "DarkThemeCard");
    card->setStyleSheet(
        QString("QFrame#%1 {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    border: 2px solid transparent;"
        "}"
        "QFrame#%1:hover {"
        "    border: 2px solid #1a73e8;"
        "}"
        "QLabel {"
        "    color: #333333;"
        "}").arg(card->objectName())
    );
    
    QVBoxLayout *layout = new QVBoxLayout(card);
    layout->setContentsMargins(10, 10, 10, 10);
    
    // 添加预览图
    QLabel *previewLabel = new QLabel(card);
    QPixmap preview(imagePath);
    previewLabel->setPixmap(preview.scaled(300, 200, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    previewLabel->setAlignment(Qt::AlignCenter);
    previewLabel->setObjectName("PreviewLabel");
    
    // 添加标题
    QLabel *titleLabel = new QLabel(title, card);
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setObjectName("TitleLabel");
    titleLabel->setStyleSheet(
        "QLabel#TitleLabel {"
        "    font-size: 16px;"
        "    margin-top: 10px;"
        "}"
    );
    
    layout->addWidget(previewLabel);
    layout->addWidget(titleLabel);
    
    card->setFixedSize(320, 240);
    
    // 添加点击事件
    card->installEventFilter(this);
    card->setCursor(Qt::PointingHandCursor);
    
    return card;
}

QGraphicsEffect* ThemePage::createShadowEffect()
{
    QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0, 0, 0, 40));
    shadow->setOffset(0, 2);
    return shadow;
}

bool ThemePage::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (watched == lightThemeCard) {
            updateThemeSelection(true);
            applyTheme(true);
            return true;
        } else if (watched == darkThemeCard) {
            updateThemeSelection(false);
            applyTheme(false);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void ThemePage::updateThemeSelection(bool isLight)
{
    // 更新亮色主题卡片样式
    lightThemeCard->setStyleSheet(
        QString("QFrame#LightThemeCard {"
        "    background-color: %1;"
        "    border-radius: 8px;"
        "    border: 2px solid %2;"
        "}"
        "QLabel#PreviewLabel {"
        "    background: transparent;"
        "}"
        "QLabel#TitleLabel {"
        "    color: %3;"
        "    font-size: 16px;"
        "    margin-top: 10px;"
        "    background: transparent;"
        "}").arg(isLight ? "white" : "#2d2d2d")
             .arg(isLight ? "#1a73e8" : "transparent")
             .arg(isLight ? "#333333" : "#ffffff")
    );

    // 更新暗色主题卡片样式
    darkThemeCard->setStyleSheet(
        QString("QFrame#DarkThemeCard {"
        "    background-color: %1;"
        "    border-radius: 8px;"
        "    border: 2px solid %2;"
        "}"
        "QLabel#PreviewLabel {"
        "    background: transparent;"
        "}"
        "QLabel#TitleLabel {"
        "    color: %3;"
        "    font-size: 16px;"
        "    margin-top: 10px;"
        "    background: transparent;"
        "}").arg(isLight ? "white" : "#2d2d2d")
             .arg(!isLight ? "#1a73e8" : "transparent")
             .arg(isLight ? "#333333" : "#ffffff")
    );

    // 先应用视觉效果
    applyTheme(isLight);

    // 启用下一步按钮
    nextButton->setEnabled(true);
    
    // 发送主题选择信号
    emit themeSelected(isLight);

    // 使用单次定时器延迟执行命令
    QTimer::singleShot(100, this, [this, isLight]() {
        executeThemeCommands(isLight);
    });
}

void ThemePage::applyTheme(bool isLight)
{
    if (isLight) {
        // 应用亮色主题
        setStyleSheet(
            "ThemePage {"
            "    background-image: url(:/images/default.jpg);"
            "    background-position: center;"
            "    background-repeat: no-repeat;"
            "    background-attachment: fixed;"
            "}"
        );

        mainCard->setStyleSheet(
            "QFrame#MainCard {"
            "    background-color: white;"
            "    border-radius: 16px;"
            "    border: 1px solid #e0e0e0;"
            "}"
        );

        titleLabel->setStyleSheet(
            "font-size: 32px;"
            "font-weight: bold;"
            "color: #1a73e8;"
            "background: transparent;"
        );

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

        // 更新主题卡片容器样式
        findChild<QFrame*>("ThemesContainer")->setStyleSheet(
            "QFrame#ThemesContainer {"
            "    background-color: #f5f5f5;"
            "    border-radius: 12px;"
            "    border: 1px solid #e0e0e0;"
            "}"
        );
    } else {
        // 应用暗色主题
        setStyleSheet(
            "ThemePage {"
            "    background-image: url(:/images/dark-default.jpg);"  // 需要添加暗色背景图
            "    background-position: center;"
            "    background-repeat: no-repeat;"
            "    background-attachment: fixed;"
            "}"
        );

        mainCard->setStyleSheet(
            "QFrame#MainCard {"
            "    background-color: #2d2d2d;"
            "    border-radius: 16px;"
            "    border: 1px solid #404040;"
            "}"
        );

        titleLabel->setStyleSheet(
            "font-size: 32px;"
            "font-weight: bold;"
            "color: #8ab4f8;"
            "background: transparent;"
        );
        
        nextButton->setStyleSheet(
            "RippleButton {"
            "    background-color: #8ab4f8;"
            "    color: #2d2d2d;"
            "    border: none;"
            "    border-radius: 8px;"
            "    padding: 12px 24px;"
            "    font-size: 16px;"
            "    min-width: 120px;"
            "}"
            "RippleButton:hover {"
            "    background-color: #99c0ff;"
            "}"
            "RippleButton:disabled {"
            "    background-color: #404040;"
            "    color: #666666;"
            "}"
        );

        // 更新主题卡片容器样式
        findChild<QFrame*>("ThemesContainer")->setStyleSheet(
            "QFrame#ThemesContainer {"
            "    background-color: #1d1d1d;"
            "    border-radius: 12px;"
            "    border: 1px solid #404040;"
            "}"
        );
    }
}

void ThemePage::executeThemeCommands(bool isLight)
{
    if (isLight) {
        // 执行亮色主题命令
        process->start("qdbus", QStringList() << "com.lingmo.Settings" << "/Theme" 
                      << "com.lingmo.Theme.setDarkMode" << "false");
        process->waitForFinished();
        
        process->start("lookandfeeltool", QStringList() << "-a" << "org.lingmo.ocean.desktop");
        process->waitForFinished();
        
        process->start("gsettings", QStringList() << "set" << "org.gnome.desktop.interface" 
                      << "color-scheme" << "prefer-light");
        process->waitForFinished();
    } else {
        // 执行暗色主题命令
        process->start("qdbus", QStringList() << "com.lingmo.Settings" << "/Theme" 
                      << "com.lingmo.Theme.setDarkMode" << "true");
        process->waitForFinished();
        
        process->start("lookandfeeltool", QStringList() << "-a" << "org.lingmo.oceandark.desktop");
        process->waitForFinished();
        
        process->start("gsettings", QStringList() << "set" << "org.gnome.desktop.interface" 
                      << "color-scheme" << "prefer-dark");
        process->waitForFinished();
    }
} 