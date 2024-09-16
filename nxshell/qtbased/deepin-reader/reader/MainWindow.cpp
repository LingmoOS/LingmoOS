// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MainWindow.h"
#include "TitleMenu.h"
#include "TitleWidget.h"
#include "Central.h"
#include "CentralDocPage.h"
#include "Application.h"
#include "Utils.h"
#include "DocSheet.h"
#include "DBusObject.h"
#include "SaveDialog.h"
#include "eventlogutils.h"

#include <DTitlebar>
#include <DWidgetUtil>
#include <DGuiApplicationHelper>
#include <DFileDialog>
#include <DDialog>

#if (DTK_VERSION_MAJOR > 5                                 \
     || ((DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR > 2) \
         || (DTK_VERSION_MAJOR == 5 && DTK_VERSION_MINOR == 2 && DTK_VERSION_PATCH > 2)))
#    include <DWindowQuitFullButton>
#else
#    include "dwindowquitfullbutton.h"   //libdtkwidget=5.2.2.10头文件引用错误，这里直接引用dwindowquitfullbutton.h
#endif

#include <QDir>
#include <QStandardPaths>
#include <QSettings>
#include <QTimer>
#include <QDesktopWidget>
#include <QPropertyAnimation>
#include <QJsonObject>
#include <QLibraryInfo>

extern "C" {
#include "load_libs.h"
}
DWIDGET_USE_NAMESPACE

QList<MainWindow *> MainWindow::m_list;
MainWindow::MainWindow(QStringList filePathList, DMainWindow *parent)
    : DMainWindow(parent), m_initFilePathList(filePathList)
{
    initBase();
    initUI();

    if (!filePathList.isEmpty())
        foreach (const QString &filePath, m_initFilePathList) {
            if (QFile(filePath).exists())   //过滤不存在的文件,需求中不含有提示文件不存在的文案
                addFile(filePath);
        }
}

MainWindow::MainWindow(DocSheet *sheet, DMainWindow *parent)
    : DMainWindow(parent)
{
    initBase();

    initUI();

    addSheet(sheet);

    QJsonObject obj {
        { "tid", EventLogUtils::Start },
        { "version", QCoreApplication::applicationVersion() },
        { "mode", 1 }
    };
    EventLogUtils::get().writeLogs(obj);
}

MainWindow::~MainWindow()
{
    m_list.removeOne(this);

    if (m_list.count() <= 0) {
        DBusObject::instance()->unRegister();
    }
}

void MainWindow::addSheet(DocSheet *sheet)
{
    if (nullptr == m_central)
        return;

    m_central->addSheet(sheet);
}

bool MainWindow::hasSheet(DocSheet *sheet)
{
    if (nullptr == m_central)
        return false;

    return m_central->hasSheet(sheet);
}

void MainWindow::activateSheet(DocSheet *sheet)
{
    if (nullptr == m_central)
        return;

    m_central->showSheet(sheet);

    this->setWindowState((this->windowState() & ~Qt::WindowMinimized) | Qt::WindowActive);

    this->activateWindow();
}

bool MainWindow::handleClose(bool needToBeSaved)
{
    if ((nullptr != m_central) && (!m_central->handleClose(needToBeSaved)))
        return false;

    this->close();
    qDebug() << __FUNCTION__ << "关闭文档查看器主窗口！";
    return true;
}

void MainWindow::addFile(const QString &filePath)
{
    if (nullptr == m_central)
        return;

    m_central->addFileAsync(filePath);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (m_central && !m_central->handleClose(true)) {
        event->ignore();
        return;
    }

    QSettings settings(QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("config.conf"), QSettings::IniFormat, this);
    qDebug() << "配置文件路径: ***" /* << QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("config.conf")*/;
    settings.setValue("LASTWIDTH", QString::number(width()));

    settings.setValue("LASTHEIGHT", QString::number(height()));

    qDebug() << __FUNCTION__ << "关闭文档查看器主窗口！";
    DMainWindow::closeEvent(event);
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        if (event->type() == QEvent::HoverMove) {
            QHoverEvent *mouseEvent = dynamic_cast<QHoverEvent *>(event);
            bool isFullscreen = this->windowState().testFlag(Qt::WindowFullScreen);
            if (isFullscreen && m_FullTitleWidget && !m_central->docPage()->isSlide()) {
                if (m_TitleAnimation == nullptr) {
                    m_TitleAnimation = new QPropertyAnimation(m_FullTitleWidget, "geometry");
                    m_TitleAnimation->setEasingCurve(QEasingCurve::OutCubic);
                    connect(m_TitleAnimation, &QPropertyAnimation::finished, this, &MainWindow::onTitleAniFinished);
                }

                if (m_TitleAnimation->state() != QPropertyAnimation::Running) {
                    m_TitleAnimation->stop();
                    int duration = 200 * (50 + m_FullTitleWidget->pos().y()) / 50;
                    duration = duration <= 0 ? 200 : duration;
                    m_TitleAnimation->setDuration(duration);
                    m_TitleAnimation->setStartValue(QRect(0, m_FullTitleWidget->pos().y(), dApp->primaryScreen()->size().width(), m_FullTitleWidget->height()));

                    if (m_FullTitleWidget->pos().y() >= 0 && mouseEvent->pos().y() > m_FullTitleWidget->height()) {
                        m_TitleAnimation->setEndValue(QRect(0, -m_FullTitleWidget->height(), dApp->primaryScreen()->size().width(), m_FullTitleWidget->height()));
                        m_TitleAnimation->start();
                    } else if (m_FullTitleWidget->pos().y() < 0 && mouseEvent->pos().y() < 2) {
                        setTitleBarFocusEnable(true);
                        if (m_docTabWidget && m_FullTitleWidget->height() > titlebar()->height())
                            m_docTabWidget->setVisible(true);
                        else if (m_docTabWidget && m_FullTitleWidget->height() <= titlebar()->height())
                            m_docTabWidget->setVisible(false);

                        m_TitleAnimation->setEndValue(QRect(0, 0, dApp->primaryScreen()->size().width(), m_FullTitleWidget->height()));
                        m_TitleAnimation->start();
                    }
                }
            }
        }
    }

    if (event->type() == QEvent::Resize) {
        onUpdateTitleLabelRect();
    }
    return DMainWindow::eventFilter(obj, event);
}

void MainWindow::initUI()
{
    qDebug() << __FUNCTION__ << "正在初始化UI界面...";
    m_central = new Central(this);

    connect(m_central, SIGNAL(sigNeedClose()), this, SLOT(close()));

    m_central->setMenu(m_menu);

    setCentralWidget(m_central);

    titlebar()->setIcon(QIcon::fromTheme("deepin-reader"));

    titlebar()->setTitle("");

    titlebar()->addWidget(m_central->titleWidget(), Qt::AlignLeft);

    titlebar()->addWidget(m_central->docPage()->getTitleLabel(), Qt::AlignLeft);

    titlebar()->setAutoHideOnFullscreen(false);

    Utils::setObjectNoFocusPolicy(this);

    setFocusPolicy(Qt::StrongFocus);

    QTimer::singleShot(100, this, SLOT(onUpdateTitleLabelRect()));

    titlebar()->installEventFilter(this);

    m_central->titleWidget()->installEventFilter(this);

    DIconButton *optBtn = titlebar()->findChild<DIconButton *>("DTitlebarDWindowOptionButton");
    if (optBtn && optBtn->parentWidget()) {
        optBtn->parentWidget()->installEventFilter(this);
    }

    DWindowQuitFullButton *quitFullBtn = titlebar()->findChild<DWindowQuitFullButton *>("DTitlebarDWindowQuitFullscreenButton");
    if (quitFullBtn) {
        connect(quitFullBtn, &DWindowQuitFullButton::clicked, this, [&]() {
            handleMainWindowExitFull();
        });
    }
    qDebug() << __FUNCTION__ << "UI界面初始化已完成";
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [=](DGuiApplicationHelper::SizeMode sizeMode) {
        if (sizeMode == DGuiApplicationHelper::NormalMode) {
            titlebar()->setFixedHeight(50);
            handleMainWindowFull();
        } else {
            titlebar()->setFixedHeight(40);
            handleMainWindowFull();
        }
    });
#endif

#if _ZPD_
    //确定移除
    initDynamicLibPath();
#endif
    QJsonObject obj {
        { "tid", EventLogUtils::Start },
        { "version", QCoreApplication::applicationVersion() },
        { "mode", 1 }
    };
    EventLogUtils::get().writeLogs(obj);
}

void MainWindow::setDocTabBarWidget(QWidget *widget)
{
    if (m_FullTitleWidget == nullptr) {
        m_FullTitleWidget = new BaseWidget(this);

        this->stackUnder(m_FullTitleWidget);

        m_FullTitleWidget->setFocusPolicy(Qt::NoFocus);

        m_FullTitleWidget->show();

        setTitleBarFocusEnable(false);
    }

    m_docTabWidget = widget;
}

void MainWindow::onTitleAniFinished()
{
    if (m_FullTitleWidget->pos().y() < 0)
        setTitleBarFocusEnable(false);
}

void MainWindow::handleMainWindowFull()
{
    if (m_FullTitleWidget == nullptr || m_docTabWidget == nullptr)
        return;

    m_lastWindowState = Qt::WindowFullScreen;
    if (this->menuWidget()) {
        this->menuWidget()->setParent(nullptr);
        this->setMenuWidget(nullptr);
    }

    setTitleBarFocusEnable(false);

    bool tabbarVisible = m_docTabWidget->isVisible();

    titlebar()->setParent(m_FullTitleWidget);

    m_docTabWidget->setParent(m_FullTitleWidget);

    titlebar()->show();

    m_docTabWidget->setVisible(tabbarVisible);

    titlebar()->setGeometry(0, 0, dApp->primaryScreen()->size().width(), titlebar()->height());

    m_docTabWidget->setGeometry(0, titlebar()->height(), dApp->primaryScreen()->size().width(), 37);

    int fulltitleH = tabbarVisible ? titlebar()->height() + 37 : titlebar()->height();

    m_FullTitleWidget->setMinimumHeight(fulltitleH);

    m_FullTitleWidget->setGeometry(0, -fulltitleH, dApp->primaryScreen()->size().width(), fulltitleH);

    updateOrderWidgets(this->property("orderlist").value<QList<QWidget *>>());
}

void MainWindow::handleMainWindowExitFull()
{
    if (m_FullTitleWidget == nullptr)
        return;

    if (m_lastWindowState == Qt::WindowFullScreen) {
        m_lastWindowState = static_cast<int>(this->windowState());

        if (m_central->docPage()->getCurSheet())
            m_central->docPage()->getCurSheet()->closeFullScreen(true);

        this->setMenuWidget(titlebar());

        setTitleBarFocusEnable(true);

        m_FullTitleWidget->setGeometry(0, -m_FullTitleWidget->height(), dApp->primaryScreen()->size().width(), m_FullTitleWidget->height());

        updateOrderWidgets(this->property("orderlist").value<QList<QWidget *>>());
    }
}

void MainWindow::setTitleBarFocusEnable(bool enable)
{
    auto updateWidgetFocus = [&](const QString &name, Qt::FocusPolicy policy) {
        if (!this->titlebar())
            return;
        QWidget *w = this->titlebar()->findChild<QWidget *>(name);
        if (!w)
            return;
        w->setFocusPolicy(policy);
    };

    updateWidgetFocus("Thumbnails", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("SP_DecreaseElement", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("scaleEdit", enable ? Qt::StrongFocus : Qt::NoFocus);
    updateWidgetFocus("editArrowBtn", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("SP_IncreaseElement", enable ? Qt::TabFocus : Qt::NoFocus);

    updateWidgetFocus("DTitlebarDWindowOptionButton", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("DTitlebarDWindowMinButton", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("DTitlebarDWindowQuitFullscreenButton", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("DTitlebarDWindowMaxButton", enable ? Qt::TabFocus : Qt::NoFocus);
    updateWidgetFocus("DTitlebarDWindowCloseButton", enable ? Qt::TabFocus : Qt::NoFocus);
}

void MainWindow::resizeFullTitleWidget()
{
    if (m_FullTitleWidget == nullptr || m_docTabWidget == nullptr)
        return;

    int fulltitleH = m_docTabWidget->isVisible() ? titlebar()->height() + 37 : titlebar()->height();

    m_FullTitleWidget->setMinimumHeight(fulltitleH);

    m_FullTitleWidget->resize(dApp->primaryScreen()->size().width(), fulltitleH);
}

MainWindow *MainWindow::windowContainSheet(DocSheet *sheet)
{
    foreach (MainWindow *window, m_list) {
        if (window->hasSheet(sheet)) {
            return window;
        }
    }

    return nullptr;
}

bool MainWindow::allowCreateWindow()
{
    return m_list.count() < 20;
}

bool MainWindow::activateSheetIfExist(const QString &filePath)
{
    DocSheet *sheet = DocSheet::getSheetByFilePath(filePath);

    if (nullptr == sheet)
        return false;

    MainWindow *window = MainWindow::windowContainSheet(sheet);

    if (nullptr != window) {
        window->activateSheet(sheet);
        return true;
    }

    return false;
}

MainWindow *MainWindow::createWindow(QStringList filePathList)
{
    qDebug() << __FUNCTION__ << "正在创建主窗口...";
    int iCount = MainWindow::m_list.count();   // 获取现有窗口数目
    MainWindow *pMainWindow = new MainWindow(filePathList);   // 创建文档查看器对话框

    // 现有数目大于0时，新创建的文档查看器对话框按照一定的规律偏移显示，每次向右、向下偏移50个像素，达到错开显示的效果，防止一直显示在桌面中间
    if (iCount > 0) {
        int windowOffset = iCount * 50;
        QRect screenGeometry = qApp->desktop()->screenGeometry(QCursor::pos());
        pMainWindow->move(screenGeometry.x() + windowOffset, screenGeometry.y() + windowOffset);
    }
    qDebug() << __FUNCTION__ << "主窗口已创建并显示";
    return pMainWindow;
}

MainWindow *MainWindow::createWindow(DocSheet *sheet)
{
    qDebug() << __FUNCTION__ << "创建窗口！";
    return new MainWindow(sheet);
}

void MainWindow::showDefaultSize()
{
    QSettings settings(QDir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).filePath("config.conf"), QSettings::IniFormat, this);

    int width = settings.value("LASTWIDTH").toInt();
    int height = settings.value("LASTHEIGHT").toInt();

    if (width == 0 || height == 0) {
        resize(1000, 680);
    } else {
        resize(width, height);
    }
}

void MainWindow::initDynamicLibPath()
{
    qDebug() << "正在加载动态库...";
    // 解析ZPD定制需求提供的库 libzpdcallback.so
    LoadLibNames tmp;
    QByteArray documentpr = libPath("libzpdcallback.so").toLatin1();
    if (QFile::exists(documentpr)) {
        tmp.chDocumentPr = documentpr.data();
    } else {
        tmp.chDocumentPr = NULL;
    }
    setLibNames(tmp);
    qDebug() << "动态库已加载";
}

QString MainWindow::libPath(const QString &strlib)
{
    QDir dir;
    QString path = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (strlib + "*"), QDir::NoDotAndDotDot | QDir::Files);   //filter name with strlib

    if (list.contains(strlib))
        return strlib;

    list.sort();
    if (list.size() > 0)
        return list.last();

    return "";
}

void MainWindow::onDelayInit()
{
    initUI();
}

void MainWindow::initBase()
{
    qDebug() << __FUNCTION__ << "正在初始化基础资源...";

    m_list.append(this);

    setTitlebarShadowEnabled(true);

    setMinimumSize(680, 300);

    showDefaultSize();

    Dtk::Widget::moveToCenter(this);

    this->installEventFilter(this);

    this->setProperty("loading", false);

    this->setProperty("windowClosed", false);

    m_menu = new TitleMenu(this);

    m_menu->setAccessibleName("Menu_Title");

    titlebar()->setMenu(m_menu);

    setAttribute(Qt::WA_DeleteOnClose);

    qDebug() << __FUNCTION__ << "基础资源已初始化";
}

void MainWindow::onUpdateTitleLabelRect()
{
    if (nullptr == m_central)
        return;

    QWidget *titleLabel = m_central->docPage()->getTitleLabel();

    int titleWidth = this->width() - m_central->titleWidget()->width() - titlebar()->buttonAreaWidth() - 60;

    if (titleWidth > 0)
        titleLabel->setFixedWidth(titleWidth);
}

void MainWindow::updateOrderWidgets(const QList<QWidget *> &orderlst)
{
    for (int i = 0; i < orderlst.size() - 1; i++) {
        QWidget::setTabOrder(orderlst.at(i), orderlst.at(i + 1));
    }
}
