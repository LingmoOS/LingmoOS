// Copyright (C) 2018 ~ 2025 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ddlog.h"
#include "mainwindow.h"
#include "common/datacommon.h"
//#include "itemwidget.h"
#include "dbus/dbusayatanainterface.h"
#include "helper.hpp"

#include <DApplication>
#include <DApplicationHelper>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>
#include <DPalette>
#include <DStyleHelper>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScreen>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QDesktopWidget>
#include <QApplication>
#include <QPalette>
#include <QTimer>
#include <QShowEvent>
#include <QHideEvent>

#define DOCK_TOP 0
#define DOCK_RIGHT 1
#define DOCK_BOTTOM 2
#define DOCK_LEFT 3

#define SCREEN_HEIGHT_MAX 1080
#define NOT_USE_QUIT_TIME_INTERVAL 5 * 60 * 1000

const QString KILL_DBUS_COMMAND = "killall deepin-system-monitor-plugin-popup";

using namespace DDLog;

MainWindow::MainWindow(QWidget *parent)
#ifdef DTKWIDGET_CLASS_DBlurEffectWithBorderWidget
    : DBlurEffectWithBorderWidget(parent)
#else
    : DBlurEffectWidget(parent)
#endif
      ,
      m_displayInter(new QDBusInterface(common::systemInfo().DISPLAY_SERVICE,
                                        common::systemInfo().DISPLAY_PATH,
                                        common::systemInfo().DISPLAY_INTERFACE,
                                        QDBusConnection::sessionBus(), this)),
      m_daemonDockInter(new QDBusInterface(common::systemInfo().DOCK_SERVICE,
                                           common::systemInfo().DOCK_PATH,
                                           common::systemInfo().DOCK_INTERFACE,
                                           QDBusConnection::sessionBus(), this)),
      m_dockInter(new DBusDockInterface),
      m_systemMonitorDbusAdaptor(new SystemMonitorDBusAdaptor),
      m_regionMonitor(nullptr),
      m_xAni(new QPropertyAnimation(this, "x")),
      m_widthAni(new QPropertyAnimation(this, "width")),
      m_aniGroup(new QSequentialAnimationGroup(this)),
      m_trickTimer(new QTimer(this)),
      m_processEndTimer(new QTimer(this))
{
    m_trickTimer->setInterval(300);
    m_trickTimer->setSingleShot(true);

    m_processEndTimer->setInterval(NOT_USE_QUIT_TIME_INTERVAL);
    m_processEndTimer->start();

    //在构造函数中存储m_displayInter->monitor()中的内容，解决内存泄漏的问题
    if (m_displayInter->isValid()) {
        auto ss = m_displayInter->property("Monitors");

        QDBusInterface busInterface(common::systemInfo().DISPLAY_SERVICE,
                                    common::systemInfo().DISPLAY_PATH,
                                    "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus());
        QDBusMessage reply = busInterface.call("Get", common::systemInfo().DISPLAY_INTERFACE, "Monitors");
        QVariant v = reply.arguments().first();
        const QDBusArgument &argument = v.value<QDBusVariant>().variant().value<QDBusArgument>();
        while (!argument.atEnd()) {
            argument >> m_dbusPathList;
        }
    }
    initDBus();
    initUI();
    initAni();
    initConnect();

    geometryChanged();
    CompositeChanged();
    registerMonitor();
    installEventFilter(this);

    DBusAyatanaInterface::getInstance();
}

MainWindow::~MainWindow()
{
}

void MainWindow::Toggle()
{
    if (m_hasComposite) {
        if (m_aniGroup->state() == QAbstractAnimation::Running)
            return;
        if (isVisible()) {
            hideAni();
        } else {
            showAni();
        }
    } else {
        if (isVisible()) {
            hideAni();
        } else {
            showAni();
        }
    }
}

void MainWindow::geometryChanged()
{
    adjustPosition();

    //init animation by 'm_rect'
    m_xAni->setStartValue(m_rect.x());
    m_xAni->setEndValue(m_rect.x());

    m_widthAni->setStartValue(m_rect.width());
    m_widthAni->setEndValue(0);
}

void MainWindow::showAni()
{
    if (m_trickTimer->isActive()) {
        return;
    }
    //停止计时
    m_processEndTimer->stop();
    qreal scale = qApp->primaryScreen()->devicePixelRatio();
    m_trickTimer->start();
    if (!m_hasComposite) {
        int dockPos = DOCK_TOP;
        int displayMode = 0;
        if (m_daemonDockInter->isValid()) {
            dockPos = m_daemonDockInter->property("Position").toInt();
            displayMode = m_daemonDockInter->property("DisplayMode").toInt();
        }
        if (m_daemonDockInter->isValid() && dockPos == DOCK_RIGHT) {
            if (displayMode == 0) {
                setGeometry(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - m_rect.width() - m_dockInter->geometry().width() - Globals::WindowMargin - 2 * Globals::DockMargin, m_rect.y(), m_rect.width(), m_rect.height());
            } else {
                setGeometry(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - m_rect.width() - m_dockInter->geometry().width() - Globals::WindowMargin, m_rect.y(), m_rect.width(), m_rect.height());
            }
        } else {
            setGeometry(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - m_rect.width() - Globals::WindowMargin, m_rect.y(), m_rect.width(), m_rect.height());
        }

        setFixedWidth(m_rect.width());
        show();
        activateWindow();

        return;
    }
    setFixedWidth(0);

    show();
    activateWindow();
    m_aniGroup->setDirection(QAbstractAnimation::Backward);
    m_aniGroup->start();
}

void MainWindow::hideAni()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();
    //不再使用开始计时
    m_processEndTimer->start();

    if (!m_hasComposite) {
        hide();
        return;
    }

    m_aniGroup->setDirection(QAbstractAnimation::Forward);
    m_aniGroup->start();

    QTimer::singleShot(m_aniGroup->duration(), this, [=] { setVisible(false); });
}

void MainWindow::startLoader()
{
    QProcess *process = new QProcess(this);
    process->startDetached("deepin-system-monitor-plugin-popup");
    process->waitForStarted();
    process->waitForFinished();
    process->deleteLater();
    process = Q_NULLPTR;
}

void MainWindow::Show()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    if (!isVisible()) {
        showAni();
    }
}

void MainWindow::Hide()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    if (isVisible()) {
        hideAni();
    }
}

void MainWindow::setX(int x)
{
    move(x, m_rect.y());
}

void MainWindow::CompositeChanged()
{
    m_hasComposite = DWindowManagerHelper::instance()->hasComposite();
}

void MainWindow::registerMonitor()
{
    if (m_regionMonitor) {
        delete m_regionMonitor;
        m_regionMonitor = nullptr;
    }
    m_regionMonitor = new DRegionMonitor(this);
    m_regionMonitor->registerRegion(QRegion(QRect()));
    connect(m_regionMonitor, &DRegionMonitor::buttonPress, this, [=](const QPoint &p, const int flag) {
        Q_UNUSED(flag);
        if (!geometry().contains(p))
            if (!isHidden()) {
                hideAni();
            }
    });
}

void MainWindow::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setFixedWidth(Globals::WindowWidth);

    m_cpuMonitor = new CpuWidget(this);
    m_netWidget = new NetWidget(this);
    m_diskWidget = new DiskWidget(this);
    m_memoryWidget = new MemoryWidget(this);
    m_processWidget = new ProcessWidget(this);

    QWidget *parentWidget = new QWidget(this);
    QVBoxLayout *vBoxLayout = new QVBoxLayout(parentWidget);
    vBoxLayout->setContentsMargins(10, 0, 10, 0);

    vBoxLayout->addWidget(m_cpuMonitor);
    vBoxLayout->addSpacing(5);
    vBoxLayout->addWidget(m_netWidget);
    vBoxLayout->addSpacing(5);
    vBoxLayout->addWidget(m_memoryWidget);
    vBoxLayout->addSpacing(5);
    vBoxLayout->addWidget(m_diskWidget);
    vBoxLayout->addSpacing(5);
    vBoxLayout->addWidget(m_processWidget);

    m_scrollArea = new DScrollArea(this);
    m_scrollArea->horizontalScrollBar()->setEnabled(false);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    auto pal = qApp->palette();
    pal.setColor(QPalette::Background, QColor(0, 0, 0, 0));
    m_scrollArea->setPalette(pal);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setWidget(parentWidget);
    m_scrollArea->setWindowFlags(Qt::FramelessWindowHint);
    m_scrollArea->setFrameShape(QFrame::NoFrame);
    m_scrollArea->setWidgetResizable(true);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->addWidget(m_scrollArea);
    setLayout(mainLayout);

    setFocusPolicy(Qt::NoFocus);
}

void MainWindow::initAni()
{
    m_xAni->setEasingCurve(QEasingCurve::Linear);
    m_xAni->setDuration(Globals::AnimationTime / 2);

    m_widthAni->setEasingCurve(QEasingCurve::Linear);
    m_widthAni->setDuration(Globals::AnimationTime);

    m_aniGroup->addAnimation(m_xAni);
    m_aniGroup->addAnimation(m_widthAni);

    auto *dAppHelper = DApplicationHelper::instance();
    connect(dAppHelper, &DApplicationHelper::themeTypeChanged, this, &MainWindow::changeTheme);
    changeTheme(dAppHelper->themeType());
}

void MainWindow::initConnect()
{
    QDBusConnection::sessionBus().connect(m_displayInter->service(), m_displayInter->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));
    connect(m_dockInter, &DBusDockInterface::geometryChanged, this, &MainWindow::geometryChanged, Qt::UniqueConnection);
    connect(this, SIGNAL(signal_geometry(int)), m_processWidget, SLOT(autoHeight(int)));

    connect(m_systemMonitorDbusAdaptor, &SystemMonitorDBusAdaptor::sigSendShowOrHideSystemMonitorPluginPopupWidget,
            this, &MainWindow::slotShowOrHideSystemMonitorPluginPopupWidget);

    //系统监视器弹窗显示状态改变
    connect(this, &MainWindow::sysMonPopVisibleChanged,
            m_systemMonitorDbusAdaptor, &SystemMonitorDBusAdaptor::sysMonPopVisibleChanged);

    connect(DWindowManagerHelper::instance(), &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::CompositeChanged, Qt::QueuedConnection);

    connect(m_widthAni, &QVariantAnimation::valueChanged, this, [=](const QVariant &value) {
        int width = value.toInt();

        //        move(int(std::round(qreal(m_rect.x() + m_rect.width()  + Globals::WindowMargin - width)) / qApp->primaryScreen()->devicePixelRatio()), m_rect.y());
        move(int(std::round(qreal(m_rect.x() + m_rect.width() + Globals::WindowMargin - width))), m_rect.y());
    });

    QDBusServiceWatcher *m_watcher = new QDBusServiceWatcher(common::systemInfo().MONITOR_SERVICE, QDBusConnection::sessionBus());
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, [=](const QString &service) {
        if (common::systemInfo().MONITOR_SERVICE != service)
            return;
        registerMonitor();
    });

    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, [=](const QString &service) {
        if (common::systemInfo().MONITOR_SERVICE != service)
            return;
        disconnect(m_regionMonitor);
    });

    // 设置窗口透明度调节
    QDBusConnection::sessionBus().connect(m_daemonDockInter->service(), m_daemonDockInter->path(), "org.freedesktop.DBus.Properties", "PropertiesChanged", this, SLOT(dbusPropertiesChanged(QString, QVariantMap, QStringList)));

    // 去除通过智能语音助手唤醒时关闭系统监视器窗口
    //    connect(DBusAyatanaInterface::getInstance(), &DBusAyatanaInterface::sigSendCloseWidget, this, [=]() { QTimer::singleShot(0, this, &MainWindow::hideAni); });

    //5分钟没人使用，结束popup进程
    connect(m_processEndTimer, &QTimer::timeout, this, []() {
        qCInfo(app) << "time is up! end process!";
        QProcess::startDetached(KILL_DBUS_COMMAND);
    });
}

void MainWindow::changeTheme(DApplicationHelper::ColorType themeType)
{
    // disable auto fill frame background
    QPalette palette;
    setAutoFillBackground(true);

    switch (themeType) {
    case DApplicationHelper::LightType:
        palette.setColor(QPalette::Background, QColor(210, 210, 210, 75));
        m_scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar::handle:vertical:hover{background-color:rgb(50,50,50);}");
        break;
    case DApplicationHelper::DarkType:
        setAutoFillBackground(false);
        //Dark主题的透明度为204
        palette.setColor(QPalette::Background, QColor(25, 25, 25, 204));
        m_scrollArea->verticalScrollBar()->setStyleSheet("QScrollBar::handle:vertical:hover{background-color:rgb(200,200,200);}");
        break;
    default:
        break;
    }

    setPalette(palette);
    setBackgroundRole(DPalette::Window);
}

void MainWindow::dbusPropertiesChanged(QString interface, QVariantMap maps, QStringList strs)
{
    if ((interface == common::systemInfo().DISPLAY_INTERFACE && maps.contains("PrimaryRect")) || (interface == common::systemInfo().DOCK_INTERFACE && (maps.contains("Position") || maps.contains("DisplayMode")))) {
        geometryChanged();
    } else if (m_daemonDockInter->isValid() && interface == common::systemInfo().DOCK_INTERFACE && maps.contains("Opacity")) {
        this->setMaskAlpha(static_cast<quint8>(m_daemonDockInter->property("Opacity").toDouble() * 255));
    }
}

void MainWindow::adjustPosition()
{
    // 屏幕尺寸
    QRect rect = getDisplayScreen();
    qreal scale = qApp->primaryScreen()->devicePixelRatio();
    rect.setWidth(Globals::WindowWidth + 10);

    int dockHeight = 0;
    int displayHeight = int(std::round(qreal(rect.height()) / scale));
    rect.setHeight(displayHeight);

    QRect dockRect = m_dockInter->geometry();
    dockHeight = dockRect.height();

    // 右上下部分预留的间隙- dockRect.height()
    rect -= QMargins(0, Globals::WindowMargin, Globals::WindowMargin, Globals::WindowMargin);

    // 初始化弹出框位置
    if (m_daemonDockInter->isValid()) {
        int dockPos = m_daemonDockInter->property("Position").toInt();
        int displayMode = m_daemonDockInter->property("DisplayMode").toInt();
        switch (dockPos) {
        case DOCK_TOP:
            rect.moveTop(dockRect.height() + Globals::WindowMargin);
            rect.setHeight(rect.height() - dockHeight);
            rect.moveRight(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - Globals::WindowMargin);
            break;
        case DOCK_BOTTOM:
            rect.setHeight(rect.height() - dockHeight);
            rect.moveRight(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - Globals::WindowMargin);
            break;
        case DOCK_LEFT:
            rect.moveRight(int(std::round(qreal(getDisplayScreen().width())) / scale) - Globals::WindowMargin);
            dockHeight = 0;
            break;
        case DOCK_RIGHT:
            if (displayMode == 0) {
                rect.moveRight(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - dockRect.width() - 2 * Globals::WindowMargin);
            } else {
                rect.moveRight(getDisplayScreen().x() + int(std::round(qreal(getDisplayScreen().width())) / scale) - dockRect.width() - Globals::WindowMargin);
            }
            dockHeight = 0;
            break;
        default:
            break;
        }
    }

    int scrollHeight = rect.height() - 20;
    emit signal_geometry(scrollHeight);

    m_scrollArea->setFixedHeight(scrollHeight);
    m_scrollArea->horizontalScrollBar()->setEnabled(false);

    auto pal = qApp->palette();
    pal.setColor(QPalette::Background, QColor(0, 0, 0, 0));
    m_scrollArea->setPalette(pal);
    m_scrollArea->setWindowFlags(Qt::FramelessWindowHint);
    m_scrollArea->setFrameShape(QFrame::NoFrame);

    // 针对时尚模式的特殊处理
    // 只有任务栏显示的时候, 才额外偏移
    if (m_daemonDockInter->isValid()) {
        int dockPos = m_daemonDockInter->property("Position").toInt();
        int displayMode = m_daemonDockInter->property("DisplayMode").toInt();
        if (displayMode == 0 && dockRect.width() * dockRect.height() > 0) {
            switch (dockPos) {
            case DOCK_TOP:
                rect -= QMargins(0, Globals::WindowMargin, 0, 0);
                break;
            case DOCK_BOTTOM:
                rect -= QMargins(0, 0, 0, Globals::WindowMargin);
                break;
            default:
                break;
            }
        }
    }

    setGeometry(rect);
    m_rect = rect;
    setFixedSize(rect.size());
}

struct PrimaryRect
{
    qint16 x;
    qint16 y;
    qint16 width;
    qint16 height;
};

const QDBusArgument &operator>>(const QDBusArgument &argument, PrimaryRect &rect)
{
    argument.beginStructure();
    argument >> rect.x;
    argument >> rect.y;
    argument >> rect.width;
    argument >> rect.height;
    argument.endStructure();
    return argument;
}

QRect MainWindow::getDisplayScreen()
{
    QRect dockRect = m_dockInter->geometry();
    for (const auto &monitorPath : m_dbusPathList) {
        QDBusInterface monitor(common::systemInfo().DISPLAY_INTERFACE, monitorPath.path(), common::systemInfo().DISPLAYMONITOR_INTERFACE, QDBusConnection::sessionBus());
        if (monitor.isValid()) {
            int curX = m_displayInter->property("X").toInt();
            int curY = m_displayInter->property("X").toInt();
            int curWidth = m_displayInter->property("X").toInt();
            int curHeight = m_displayInter->property("X").toInt();
            QRect screenRect(curX, curY, curWidth, curHeight);
            if (screenRect.contains(dockRect))
                return screenRect;
        }
    }
    if (m_displayInter->isValid()) {
        QDBusInterface busInterface(common::systemInfo().DISPLAY_SERVICE, common::systemInfo().DISPLAY_PATH,
                                    "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus());
        QDBusMessage reply = busInterface.call("Get", common::systemInfo().DISPLAY_INTERFACE, "PrimaryRect");
        QVariant v = reply.arguments().first();
        const QDBusArgument &argument = v.value<QDBusVariant>().variant().value<QDBusArgument>();

        PrimaryRect primaryRect;
        while (!argument.atEnd()) {
            argument >> primaryRect;
        }
        return QRect(primaryRect.x, primaryRect.y, primaryRect.width, primaryRect.height);
    }
    return dockRect;
}

bool MainWindow::initDBus()
{
    // 申请一个总线连接
    if (!QDBusConnection::sessionBus().isConnected()) {
        return false;
    }

    // 在总线连接上挂载服务，这样其他进程才能请求该服务
    if (!QDBusConnection::sessionBus().registerService(Globals::SERVICE_NAME)) {
        return false;
    }

    // 在挂载的服务上注册一个执行服务的对象
    if (!QDBusConnection::sessionBus().registerObject(Globals::SERVICE_PATH, m_systemMonitorDbusAdaptor, QDBusConnection::ExportAllSlots | QDBusConnection::ExportAllSignals)) {
        qCInfo(app) << QDBusConnection::sessionBus().lastError();
        return false;
    }

    return true;
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    // 设置滚动区域显示策略
    if (event->type() == QEvent::MouseMove) {
        QMouseEvent *mouseEvent = dynamic_cast<QMouseEvent *>(event);
        if (m_last_time_move == 0) {
            m_last_time_move = mouseEvent->pos().y();
        }

        int distance = m_last_time_move - mouseEvent->pos().y();
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() + distance);
        m_last_time_move = mouseEvent->pos().y();
    } else if (event->type() == QEvent::MouseButtonRelease) {
        m_last_time_move = 0;
    }
    return DBlurEffectWidget::eventFilter(object, event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //禁止窗口被拖动
    Q_UNUSED(event);
    return;
}

void MainWindow::slotShowOrHideSystemMonitorPluginPopupWidget()
{
    QTimer::singleShot(0, this, &MainWindow::Toggle);
}

void MainWindow::showEvent(QShowEvent *event)
{
    Q_EMIT sysMonPopVisibleChanged(true);
    QWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Q_EMIT sysMonPopVisibleChanged(false);
    QWidget::hideEvent(event);
}
