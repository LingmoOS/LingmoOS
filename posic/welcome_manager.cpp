#include "welcome_manager.h"
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QParallelAnimationGroup>
#include <QFile>
#include <QTextStream>
#include <QDate>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QSizePolicy>
#include <QApplication>
#include <QScreen>

WelcomeManager::WelcomeManager(QWidget *parent)
    : QStackedWidget(parent)
    , betaLabel(nullptr)  // 初始化为 nullptr
{
    // 设置窗口属性
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    
    // 设置全屏并调整大小
    showFullScreen();
    
    // 检查是否是 Beta 版本
    QFile osRelease("/etc/os-release");
    bool isBeta = false;
    QString version;
    QString releaseDate = QDate::currentDate().toString("yyyyMMdd");
    
    if (osRelease.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&osRelease);
        QString line;
        while (!in.atEnd()) {
            line = in.readLine();
            if (line.startsWith("RELEASE=")) {
                isBeta = line.contains("Beta");
            } else if (line.startsWith("VERSION_BUILD=")) {
                version = line.split("=")[1].trimmed();
                version.remove('"');  // 移除可能存在的引号
            }
        }
        osRelease.close();
    }
    
    // 如果是 Beta 版本，添加标签
    if (isBeta) {
        QString labelText = QString("LingmoOS_beta.%1_OS-build%2").arg(releaseDate).arg(version);
        betaLabel = new QLabel(labelText, this);
        betaLabel->setStyleSheet(
            "QLabel {"
            "    color: #666666;"
            "    font-size: 12px;"  // 调整字体大小
            "    background: transparent;"  // 确保背景透明
            "    margin: 0;"  // 移除边距
            "    padding: 0;"  // 移除内边距
            "}"
        );
        betaLabel->setAlignment(Qt::AlignRight | Qt::AlignBottom);
        betaLabel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);  // 允许自由调整大小
        betaLabel->adjustSize();  // 根据内容调整大小
        
        // 直接设置标签位置在右下角
        betaLabel->move(width() - betaLabel->width() - 20,
                       height() - betaLabel->height() - 20);
        
        // 确保标签始终在最上面
        betaLabel->raise();
    }

    // 创建欢迎页面
    welcomePage = new WelcomeWindow(this);
    connect(welcomePage, &WelcomeWindow::nextClicked, this, &WelcomeManager::nextPage);
    
    // 创建主题选择页面
    themePage = new ThemePage(this);
    
    // 创建 Dock 设置页面
    dockPage = new DockPage(this);
    connect(dockPage, &DockPage::backClicked, [this]() {
        slideToPage(themePage);
    });
    connect(dockPage, &DockPage::nextClicked, [this]() {
        slideToPage(finishPage);
    });

    // 创建完成页面
    finishPage = new FinishPage(this);
    connect(finishPage, &FinishPage::finished, [this]() {
        close();
    });
    
    // 连接主题变化信号到所有需要的页面
    connect(themePage, &ThemePage::themeSelected, dockPage, &DockPage::applyTheme);
    connect(themePage, &ThemePage::themeSelected, finishPage, &FinishPage::applyTheme);
    
    // 连接主题页面的下一步到 Dock 页面
    connect(themePage, &ThemePage::nextClicked, [this]() {
        slideToPage(dockPage);
    });

    // 添加页面到堆栈
    addWidget(welcomePage);
    addWidget(themePage);
    addWidget(dockPage);
    addWidget(finishPage);
    
    // 设置初始页面
    setCurrentWidget(welcomePage);
}

void WelcomeManager::resizeEvent(QResizeEvent *event)
{
    QStackedWidget::resizeEvent(event);  // 调用父类的实现
    
    // 更新 Beta 标签位置
    if (betaLabel) {
        betaLabel->move(width() - betaLabel->width() - 20,
                       height() - betaLabel->height() - 20);
        betaLabel->raise();
    }
}

void WelcomeManager::nextPage()
{
    if (currentWidget() == welcomePage) {
        slideToPage(themePage);
    }
}

void WelcomeManager::slideToPage(QWidget* page)
{
    if (page == currentWidget()) return;
    
    // 获取当前页面和新页面的主卡片
    QFrame* currentCard = currentWidget()->findChild<QFrame*>("MainCard");
    QFrame* newCard = page->findChild<QFrame*>("MainCard");
    
    // 设置新页面为当前页面
    setCurrentWidget(page);
    
    // 创建并行动画组
    QParallelAnimationGroup* group = new QParallelAnimationGroup(this);
    
    // 为当前卡片创建动画
    QGraphicsOpacityEffect* currentEffect = new QGraphicsOpacityEffect(currentCard);
    currentCard->setGraphicsEffect(currentEffect);
    QPropertyAnimation* currentFade = new QPropertyAnimation(currentEffect, "opacity", group);
    currentFade->setDuration(300);
    currentFade->setStartValue(1.0);
    currentFade->setEndValue(0.0);
    group->addAnimation(currentFade);
    
    // 为新卡片创建动画
    QGraphicsOpacityEffect* newEffect = new QGraphicsOpacityEffect(newCard);
    newCard->setGraphicsEffect(newEffect);
    newEffect->setOpacity(0);
    QPropertyAnimation* newFade = new QPropertyAnimation(newEffect, "opacity", group);
    newFade->setDuration(300);
    newFade->setStartValue(0.0);
    newFade->setEndValue(1.0);
    group->addAnimation(newFade);
    
    // 动画结束后清理效果
    connect(group, &QParallelAnimationGroup::finished, [=]() {
        if (currentCard && newCard) {
            currentCard->setGraphicsEffect(nullptr);
            newCard->setGraphicsEffect(nullptr);
        }
        group->deleteLater();
    });
    
    // 开始动画
    group->start(QAbstractAnimation::DeleteWhenStopped);
} 