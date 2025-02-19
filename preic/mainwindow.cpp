#include "mainwindow.h"
#include "welcomepage.h"
#include "installpage.h"
#include <QFile>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);
    
    welcomePage = new WelcomePage(this);
    installPage = new InstallPage(this);
    
    stackedWidget->addWidget(welcomePage);
    stackedWidget->addWidget(installPage);
    
    // 设置初始透明度效果
    QGraphicsOpacityEffect *opacity = new QGraphicsOpacityEffect(this);
    welcomePage->setGraphicsEffect(opacity);
    opacity->setOpacity(0);
    
    // 设置淡入动画
    fadeInAnimation = new QPropertyAnimation(opacity, "opacity", this);
    fadeInAnimation->setDuration(1000);
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);
    fadeInAnimation->start();
    
    // 页面切换动画
    connect(welcomePage, &WelcomePage::languageSelected, this, [this]() {
        if (welcomePage->property("isInstallSelected").toBool()) {
            // 创建新的动画效果
            QGraphicsOpacityEffect *welcomeOpacity = new QGraphicsOpacityEffect(welcomePage);
            QGraphicsOpacityEffect *installOpacity = new QGraphicsOpacityEffect(installPage);
            
            welcomePage->setGraphicsEffect(welcomeOpacity);
            installPage->setGraphicsEffect(installOpacity);
            welcomeOpacity->setOpacity(1.0);
            installOpacity->setOpacity(0.0);
            
            QPropertyAnimation *fadeOut = new QPropertyAnimation(welcomeOpacity, "opacity");
            fadeOut->setDuration(500);
            fadeOut->setStartValue(1.0);
            fadeOut->setEndValue(0.0);
            
            QPropertyAnimation *fadeIn = new QPropertyAnimation(installOpacity, "opacity");
            fadeIn->setDuration(500);
            fadeIn->setStartValue(0.0);
            fadeIn->setEndValue(1.0);
            
            QParallelAnimationGroup *crossFade = new QParallelAnimationGroup(this);
            crossFade->addAnimation(fadeOut);
            crossFade->addAnimation(fadeIn);
            
            connect(crossFade, &QParallelAnimationGroup::finished, this, [=]() {
                stackedWidget->setCurrentWidget(installPage);
                // 清理动画效果
                welcomePage->setGraphicsEffect(nullptr);
                installPage->setGraphicsEffect(nullptr);
                delete welcomeOpacity;
                delete installOpacity;
            });
            
            crossFade->start(QAbstractAnimation::DeleteWhenStopped);
        } else {
            qApp->quit();
        }
    });
    
    // 设置窗口背景
    setStyleSheet(
        "QMainWindow {"
        "    background-color: #FAFAFA;"
        "}"
    );
    
    // 加载全局样式表
    QFile styleFile(":/styles/style.qss");
    styleFile.open(QFile::ReadOnly);
    QString style = QLatin1String(styleFile.readAll());
    qApp->setStyleSheet(style);
}

// 添加析构函数的实现
MainWindow::~MainWindow()
{
    // 由于所有的子部件都是通过父子关系管理的
    // Qt 会自动删除它们，这里可以为空
} 