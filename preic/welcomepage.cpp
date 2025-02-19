#include "welcomepage.h"
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include "translator.h"
#include <QGraphicsDropShadowEffect>
#include "ripplebutton.h"
#include <QIcon>
#include <QHBoxLayout>
#include <QEvent>
#include <QStackedWidget>

WelcomePage::WelcomePage(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    connect(Translator::instance(), &Translator::languageChanged,
            this, &WelcomePage::updateText);
    connect(nextButton, &RippleButton::clicked,
            this, &WelcomePage::onNextClicked);
}

void WelcomePage::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);
    
    // 创建中心容器
    QWidget *centerWidget = new QWidget(this);
    centerWidget->setStyleSheet(
        "QWidget {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    border: 1px solid rgba(0,0,0,0.1);"
        "}"
    );
    
    stackedWidget = new QStackedWidget(this);
    
    // 创建语言选择页面和安装页面
    languagePage = new QWidget(this);
    installPage = new QWidget(this);
    
    setupLanguagePage();
    setupInstallPage();
    
    stackedWidget->addWidget(languagePage);
    stackedWidget->addWidget(installPage);
    
    QVBoxLayout *centerLayout = new QVBoxLayout(centerWidget);
    centerLayout->addWidget(stackedWidget);
    
    // 设置中心容器大小和位置
    centerWidget->setFixedSize(800, 600);
    layout->addWidget(centerWidget, 0, Qt::AlignCenter);
    
    currentPage = LanguagePage;
}

void WelcomePage::setupLanguagePage()
{
    QVBoxLayout *layout = new QVBoxLayout(languagePage);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 40, 40, 40);
    
    // Logo
    QLabel *logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/images/logo.svg");
    if (!logoPixmap.isNull()) {
        logoLabel->setFixedSize(48, 48);
        logoLabel->setScaledContents(true);
        logoLabel->setPixmap(logoPixmap);
    }
    
    // 标题
    welcomeLabel = new QLabel(tr("欢迎使用 Lingmo OS"), this);
    welcomeLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 24px;"
        "    font-weight: 500;"
        "    color: rgba(0,0,0,0.87);"
        "    border: none;"
        "}"
    );
    
    // 语言选择
    languageCombo = new QComboBox(this);
    languageCombo->addItem("简体中文");
    languageCombo->addItem("English");
    languageCombo->addItem("Español");
    
    // 下一步按钮
    nextButton = new RippleButton(tr("下一步"), languagePage);
    nextButton->setFixedSize(120, 36);
    nextButton->setCursor(Qt::PointingHandCursor);
    
    layout->addWidget(logoLabel, 0, Qt::AlignLeft);
    layout->addWidget(welcomeLabel);
    layout->addWidget(languageCombo);
    layout->addStretch();
    layout->addWidget(nextButton, 0, Qt::AlignRight);
}

void WelcomePage::setupInstallPage()
{
    QVBoxLayout *layout = new QVBoxLayout(installPage);
    layout->setSpacing(20);
    layout->setContentsMargins(40, 40, 40, 40);
    
    // 返回按钮
    QPushButton *backButton = new QPushButton(tr("返回"), this);
    backButton->setStyleSheet(
        "QPushButton {"
        "    color: #8B6B2E;"
        "    border: none;"
        "    font-size: 16px;"
        "    text-align: left;"
        "    padding: 0;"
        "    padding-left: 24px;"
        "}"
    );
    backButton->setIcon(QIcon(":/images/back.svg"));
    backButton->setCursor(Qt::PointingHandCursor);
    connect(backButton, &QPushButton::clicked, [this]() {
        currentPage = LanguagePage;
        stackedWidget->setCurrentWidget(languagePage);
        updateText();
    });
    
    // Logo
    QLabel *logoLabel = new QLabel(this);
    QPixmap logoPixmap(":/images/logo.svg");
    if (!logoPixmap.isNull()) {
        logoLabel->setFixedSize(48, 48);
        logoLabel->setScaledContents(true);
        logoLabel->setPixmap(logoPixmap);
    }
    
    // 标题
    installWelcomeLabel = new QLabel(tr("开始使用 Lingmo OS"), this);
    installWelcomeLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 24px;"
        "    font-weight: 500;"
        "    color: rgba(0,0,0,0.87);"
        "    border: none;"
        "}"
    );
    
    // 说明文本
    QLabel *descLabel = new QLabel(tr("为获得最佳体验，请将 Lingmo OS 安装到您的内部磁盘。您也可等到需要时再从登录屏幕安装它。"), this);
    descLabel->setStyleSheet(
        "QLabel {"
        "    font-size: 16px;"
        "    color: rgba(0,0,0,0.6);"
        "    margin: 20px 0;"
        "    border: none;"
        "}"
    );
    descLabel->setWordWrap(true);
    
    // 创建一个容器来包含选项和按钮
    QWidget *contentContainer = new QWidget(installPage);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setSpacing(20);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    
    // 选项容器
    QWidget *optionsContainer = new QWidget(contentContainer);
    optionsContainer->setObjectName("optionsContainer");
    optionsContainer->setStyleSheet(
        "QWidget#optionsContainer {"
        "    background-color: white;"
        "    border-radius: 8px;"
        "    padding: 20px;"
        "    border: 1px solid rgba(0,0,0,0.08);"
        "}"
    );
    
    // 添加阴影效果
    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect(optionsContainer);
    shadow->setBlurRadius(10);
    shadow->setColor(QColor(0, 0, 0, 30));
    shadow->setOffset(0, 2);
    optionsContainer->setGraphicsEffect(shadow);
    
    QVBoxLayout *optionsLayout = new QVBoxLayout(optionsContainer);
    optionsLayout->setSpacing(12);
    optionsLayout->setContentsMargins(20, 20, 20, 20);
    
    // 安装选项
    installOption = new QWidget(optionsContainer);
    installOption->setObjectName("installOption");
    installOption->setProperty("class", "OptionWidget");  // 添加类名
    installOption->setCursor(Qt::PointingHandCursor);
    installOption->setFixedHeight(56);
    
    QHBoxLayout *installLayout = new QHBoxLayout(installOption);
    installLayout->setContentsMargins(16, 0, 16, 0);  // 减小左右边距
    installLayout->setSpacing(12);  // 减小图标和文本的间距
    
    // 安装选项图标
    QLabel *installIcon = new QLabel(this);
    QPixmap installPixmap(":/images/install.svg");
    if (!installPixmap.isNull()) {
        installIcon->setFixedSize(20, 20);  // 减小图标尺寸
        installIcon->setScaledContents(true);
        installIcon->setPixmap(installPixmap);
    }
    
    QLabel *installText = new QLabel(tr("安装 Lingmo OS"), this);
    installText->setStyleSheet(
        "QLabel {"
        "    font-size: 15px;"  // 调整字体大小
        "    color: rgba(0,0,0,0.87);"
        "    border: none;"
        "}"
    );
    
    // 选中图标
    QLabel *checkIcon = new QLabel(this);
    QPixmap checkPixmap(":/images/check.svg");
    if (!checkPixmap.isNull()) {
        checkIcon->setFixedSize(18, 18);
        checkIcon->setScaledContents(true);
        checkIcon->setPixmap(checkPixmap);
        checkIcon->setObjectName("checkIcon");  // 添加对象名以便识别
    }
    
    installLayout->addWidget(installIcon);
    installLayout->addWidget(installText, 1);
    installLayout->addWidget(checkIcon);
    
    // 试用选项
    tryOption = new QWidget(optionsContainer);
    tryOption->setObjectName("tryOption");
    tryOption->setProperty("class", "OptionWidget");  // 添加类名
    tryOption->setCursor(Qt::PointingHandCursor);
    tryOption->setFixedHeight(72);
    
    QHBoxLayout *tryLayout = new QHBoxLayout(tryOption);
    tryLayout->setContentsMargins(16, 12, 16, 12);  // 调整边距以容纳两行文本
    tryLayout->setSpacing(12);
    
    // 试用选项图标
    QLabel *tryIcon = new QLabel(this);
    QPixmap tryPixmap(":/images/try.svg");
    if (!tryPixmap.isNull()) {
        tryIcon->setFixedSize(20, 20);
        tryIcon->setScaledContents(true);
        tryIcon->setPixmap(tryPixmap);
    }
    
    QVBoxLayout *tryTextLayout = new QVBoxLayout;
    tryTextLayout->setSpacing(4);
    
    QLabel *tryText = new QLabel(tr("先试用一下"), this);
    tryText->setStyleSheet(
        "QLabel {"
        "    font-size: 15px;"
        "    color: rgba(0,0,0,0.87);"
        "    border: none;"
        "}"
    );
    
    QLabel *tryDesc = new QLabel(tr("请从 USB（而非您的操作系统）中运行 Lingmo OS。"), this);
    tryDesc->setStyleSheet(
        "QLabel {"
        "    font-size: 13px;"  // 减小描述文字大小
        "    color: rgba(0,0,0,0.6);"
        "    border: none;"
        "}"
    );
    tryDesc->setWordWrap(true);
    
    tryTextLayout->addWidget(tryText);
    tryTextLayout->addWidget(tryDesc);
    
    tryLayout->addWidget(tryIcon, 0, Qt::AlignTop);  // 图标顶部对齐
    tryLayout->addLayout(tryTextLayout, 1);
    
    // 试用选项的选中图标
    QLabel *tryCheckIcon = new QLabel(this);
    tryCheckIcon->setPixmap(checkPixmap);
    tryCheckIcon->setFixedSize(18, 18);
    tryCheckIcon->setScaledContents(true);
    tryCheckIcon->setObjectName("checkIcon");  // 添加对象名以便识别
    
    tryLayout->addWidget(tryCheckIcon);
    
    // 添加选项到容器
    optionsLayout->addWidget(installOption);
    optionsLayout->addWidget(tryOption);
    
    // 下一步按钮
    RippleButton *installNextButton = new RippleButton(tr("下一步"), contentContainer);
    installNextButton->setFixedSize(120, 36);
    installNextButton->setCursor(Qt::PointingHandCursor);
    connect(installNextButton, &RippleButton::clicked, this, &WelcomePage::onNextClicked);
    
    // 将选项容器和按钮添加到内容容器
    contentLayout->addWidget(optionsContainer);
    contentLayout->addWidget(installNextButton, 0, Qt::AlignRight);
    
    // 添加到主布局
    layout->addWidget(backButton, 0, Qt::AlignLeft);
    layout->addSpacing(20);
    layout->addWidget(logoLabel, 0, Qt::AlignLeft);
    layout->addSpacing(20);
    layout->addWidget(installWelcomeLabel);
    layout->addWidget(descLabel);
    layout->addSpacing(20);
    layout->addWidget(contentContainer, 1);
    
    // 设置选项的事件过滤器
    installOption->installEventFilter(this);
    tryOption->installEventFilter(this);
    
    // 设置初始状态
    isInstallSelected = true;
    updateOptionStyles();

    // 初始化选中图标的显示状态
    checkIcon->setVisible(true);      // 默认显示安装选项的选中图标
    tryCheckIcon->setVisible(false);  // 默认隐藏试用选项的选中图标
}

void WelcomePage::updateOptionStyles()
{
    QString selectedStyle = QString(
        "QWidget#installOption, QWidget#tryOption {"
        "    background-color: %1;"
        "    border-radius: 4px;"
        "    border: 1px solid rgba(0,0,0,0.08);"
        "}"
        "QWidget > QWidget {"
        "    border: none;"
        "    background: transparent;"
        "}"
    );

    // 更新选中图标的可见性
    QList<QLabel *> installCheckIcons = installOption->findChildren<QLabel *>("checkIcon");
    if (!installCheckIcons.isEmpty()) {
        installCheckIcons.first()->setVisible(isInstallSelected);
    }
    
    QList<QLabel *> tryCheckIcons = tryOption->findChildren<QLabel *>("checkIcon");
    if (!tryCheckIcons.isEmpty()) {
        tryCheckIcons.first()->setVisible(!isInstallSelected);
    }

    installOption->setStyleSheet(selectedStyle.arg(isInstallSelected ? "#F5EFE0" : "white"));
    tryOption->setStyleSheet(selectedStyle.arg(!isInstallSelected ? "#F5EFE0" : "white"));
}

bool WelcomePage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == installOption || watched == tryOption) {
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (!widget) return false;
        
        switch (event->type()) {
            case QEvent::Enter:
                if (!isInstallSelected && watched == installOption) {
                    widget->setStyleSheet(widget->styleSheet().replace(
                        "background-color: white",
                        "background-color: rgba(0,0,0,0.04)"
                    ));
                } else if (isInstallSelected && watched == tryOption) {
                    widget->setStyleSheet(widget->styleSheet().replace(
                        "background-color: white",
                        "background-color: rgba(0,0,0,0.04)"
                    ));
                }
                return true;
                
            case QEvent::Leave:
                updateOptionStyles();
                return true;
                
            case QEvent::MouseButtonPress:
                if (watched == installOption && !isInstallSelected) {
                    isInstallSelected = true;
                    updateOptionStyles();
                    emit installOptionSelected();
                } else if (watched == tryOption && isInstallSelected) {
                    isInstallSelected = false;
                    updateOptionStyles();
                    emit tryOptionSelected();
                }
                return true;
                
            default:
                break;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void WelcomePage::onNextClicked()
{
    if (currentPage == LanguagePage) {
        Translator::instance()->switchLanguage(languageCombo->currentText());
        switchToInstallPage();
    } else {
        setProperty("isInstallSelected", isInstallSelected);
        emit languageSelected();
    }
}

void WelcomePage::switchToInstallPage()
{
    currentPage = InstallPage;
    stackedWidget->setCurrentWidget(installPage);
    updateText();
}

void WelcomePage::updateText()
{
    if (currentPage == LanguagePage) {
        welcomeLabel->setText(tr("欢迎使用 Lingmo OS"));
        nextButton->setText(tr("下一步"));
    } else {
        installWelcomeLabel->setText(tr("开始使用 Lingmo OS"));
    }
} 