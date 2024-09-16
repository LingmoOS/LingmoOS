// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-only


#include "mainwindow.h"
#include "centerwidget.h"
#include "titlewidget.h"
#include "partedproxy/dmdbushandler.h"
#include "common.h"

#include <DTitlebar>
#include <DWidgetUtil>
#include <DWindowCloseButton>

#include <QTimer>
#include <QApplication>
#include <QDesktopWidget>
#include <QRect>
#include <QDebug>
#include <QScreen>

MainWindow *MainWindow::instance()
{
    static MainWindow *m = nullptr;
    if (m == nullptr) {
        m = new MainWindow;
    }
    return m;
}

MainWindow::MainWindow(QWidget *parent)
    : DMainWindow(parent)
{
//    if (false == DeviceInfoParser::instance().getRootPassword()) {
//        exit(-1);
//    }

    m_handler = DMDbusHandler::instance(this);

    initUi();
    initConnection();

    // 窗口大小，默认适应1080P(1080，608)
    #define DEFAULT_SCALE 9 / 16
    const int niceWidth = 1080;
    const int minWidth = 850;
    const int minHeight = 608;

    //适应不同分辨率，保持9:16窗口比例, 默认适应1080P(1080，608)
    QSize normal(niceWidth, minHeight);
    // 获取主屏幕
    QScreen *screen = QGuiApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int scaleWidth = screenGeometry.width() * DEFAULT_SCALE;
    int scaleHeight = screenGeometry.height() * DEFAULT_SCALE;
    if (scaleWidth < normal.width() && scaleHeight < normal.height()) {
        normal.setWidth(scaleWidth);
        normal.setHeight(scaleHeight);
    }

    if (normal.width() < minWidth || normal.height() < minHeight) {
        setMinimumSize(minWidth, minHeight);
    } else {
        setMinimumSize(normal.width(), normal.height());
    }

    resize(normal);

//    m_handler->getDeviceInfo(); //call after initUi
    QTimer::singleShot(200, this, SLOT(getDeviceInfo()));
}

MainWindow::~MainWindow()
{
//    if (nullptr != m_bufferWin) {
//        m_bufferWin->deleteLater();
//    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_central->HandleQuit();
//    m_handler->Quit();
    QProcess proc;
    proc.startDetached("/usr/bin/dbus-send --system --type=method_call --dest=com.deepin.diskmanager /com/deepin/diskmanager com.deepin.diskmanager.Quit");

    DMainWindow::closeEvent(event);
}

void MainWindow::initUi()
{
    m_dialog = new AnimationDialog(this);
    m_dialog->setWindowModality(Qt::ApplicationModal);
    m_dialog->hide();

    m_central = new CenterWidget(this);
    setCentralWidget(m_central);

    m_btnRefresh = new DPushButton;
    QIcon icon = Common::getIcon("refresh");
    m_btnRefresh->setIcon(icon);
    setSizebyMode(m_btnRefresh);
    m_btnRefresh->setToolTip(tr("Refresh"));
    m_btnRefresh->setCheckable(false);
    m_btnRefresh->setObjectName("refresh");
    m_btnRefresh->setAccessibleName("refresh");

    QHBoxLayout *refreshLayout = new QHBoxLayout;
    refreshLayout->addSpacing(10);
    refreshLayout->addWidget(m_btnRefresh);
    refreshLayout->setContentsMargins(0, 0, 0, 0);

    QWidget *widget = new QWidget;
    widget->setLayout(refreshLayout);

    titlebar()->setIcon(QIcon::fromTheme(appName));
    titlebar()->setTitle("");
    // titlebar()->setMenu(m_central->titleMenu());
    titlebar()->addWidget(widget, Qt::AlignLeft);
    titlebar()->addWidget(m_central->getTitleWidget(), Qt::AlignCenter);
    titlebar()->menu();
}

void MainWindow::setSizebyMode(DPushButton *button)
{
#ifdef DTKWIDGET_CLASS_DSizeMode
    button->setFixedSize(QSize(DSizeModeHelper::element(24, 36), DSizeModeHelper::element(24, 36)));
    button->setIconSize(QSize(DSizeModeHelper::element(12, 18), DSizeModeHelper::element(12, 18)));
#else
    button->setFixedSize(QSize(36, 36));
    button->setIconSize(QSize(18, 18));
#endif
}

void MainWindow::initConnection()
{
    connect(m_handler, &DMDbusHandler::showSpinerWindow, this, &MainWindow::onShowSpinerWindow);
    connect(m_btnRefresh, &DPushButton::clicked, this, &MainWindow::onRefreshButtonClicked);
#ifdef DTKWIDGET_CLASS_DSizeMode
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::sizeModeChanged, this, [this]() {
        setSizebyMode(m_btnRefresh);
    });
#endif
}

void MainWindow::onHandleQuitAction()
{
    //ToDo judge exit or no
    qDebug() << __FUNCTION__;

    QProcess proc;
    proc.startDetached("/usr/bin/dbus-send --system --type=method_call --dest=com.deepin.diskmanager /com/deepin/diskmanager com.deepin.diskmanager.Quit");

}

QString MainWindow::getRootLoginResult()
{
    return m_handler->getRootLoginResult();
}

void MainWindow::getDeviceInfo()
{
    m_handler->getDeviceInfo(); //call after initUi
}

void MainWindow::onRefreshButtonClicked()
{
    m_handler->refresh();
}

void MainWindow::onShowSpinerWindow(bool isShow, const QString &title)
{
    if (isShow) {
        Dtk::Widget::moveToCenter(m_dialog);
        m_dialog->setShowSpinner(isShow, title);
        m_dialog->show();
    } else {
        m_dialog->setShowSpinner(isShow, title);
        m_dialog->hide();

        raise();
        activateWindow();
    }
}
