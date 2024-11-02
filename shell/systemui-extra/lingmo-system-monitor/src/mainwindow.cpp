#include "mainwindow.h"
#include "macro.h"

#include <unistd.h>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtDBus/QtDBus>
#include <QApplication>
#include <QTimer>
#include <QScreen>
#include <QDebug>
#include "lingmostylehelper/lingmostylehelper.h"
#include <KWindowEffects>

MainWindow::MainWindow(QWidget *parent)
    : QFrame(parent)
{
    this->setObjectName("SystemMonitor");
    this->setAttribute(Qt::WA_TranslucentBackground);//背景透明
    this->setAttribute(Qt::WA_AlwaysShowToolTips);
    this->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    this->setWindowTitle(tr("Lingmo System Monitor"));
    this->installEventFilter(this);

    m_lastWndSize.setWidth(MAINWINDOWWIDTH);
    m_lastWndSize.setHeight(MAINWINDOWHEIGHT_DEFAULT);
    m_bIsWndMax = false;

    m_proSettings = new QSettings(LINGMO_COMPANY_SETTING, LINGMO_SETTING_FILE_NAME_SETTING, this);
    m_proSettings->setIniCodec("UTF-8");
    loadSettings();
    this->resize(m_lastWndSize.width(),m_lastWndSize.height());
    if (m_bIsWndMax) {
        QTimer::singleShot(0, this, SLOT(onMaximizeWindow()));
    }
    setMinimumSize(MAINWINDOWWIDTH, MAINWINDOWHEIGHT);  //set the minimum size of the mainwindow
    getOsRelease();
    initOpacityGSettings();
    initUI();
    initConnections();
#ifdef ENBALE_TRAY
    if(!QSystemTrayIcon::isSystemTrayAvailable()) {
        qDebug() << "isSystemTrayAvailable = " << QSystemTrayIcon::isSystemTrayAvailable();
    }

    TrayIcon *m_trayIcon = new TrayIcon(m_leftWidget->getCpuHisWidget(), m_leftWidget->getMemHisWidget(), m_leftWidget->getNetHisWidget());
    connect(m_trayIcon, &TrayIcon::openMonitor, this, &MainWindow::handleMessage);
#endif
}

MainWindow::~MainWindow()
{
    if (m_gsTransOpacity) {
        delete m_gsTransOpacity;
        m_gsTransOpacity = nullptr;
    }
    if (m_proSettings) {
        delete m_proSettings;
        m_proSettings = nullptr;
    }
    if (m_ifSettings) {
        delete m_ifSettings;
        m_ifSettings = nullptr;
    }
}

void MainWindow::initUI()
{
    const QByteArray ifid(MENU_SCHEMA);
    if(QGSettings::isSchemaInstalled(ifid))
    {
        m_ifSettings = new QGSettings(ifid);
    }
    if (m_ifSettings && m_ifSettings->keys().contains(SUPPORT_SERVICE_KEY)) {
        QVariant surport = m_ifSettings->get(SUPPORT_SERVICE);
        if (surport.isValid()) {
            m_isSurportService = (bool)(surport.toInt());
        }
    }
    // 初始化布局
    m_mainLayout = new QHBoxLayout();
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);

    // 初始控件
    initLeftSideBar();
    initRightPanel();

    this->setLayout(m_mainLayout);
    int nDefautPage = 0;
    QTimer::singleShot(0, this, [&,this,nDefautPage](){
        this->switchPage(nDefautPage);
    });
    this->moveCenter();
}

void MainWindow::initConnections()
{
    connect(m_leftWidget, SIGNAL(switchPage(int)), this, SLOT(onSwitchPage(int)));
    connect(m_rightWidget, SIGNAL(maximizeWindow()), this, SLOT(onMaximizeWindow()));
    connect(m_rightWidget, SIGNAL(minimizeWindow()), this, SLOT(onMinimizeWindow()));
    if (m_procWidget) {
        connect(m_rightWidget, &KRightWidget::searchSignal, m_procWidget, &ProcessWidget::searchSignal);
    }
    if (m_svcWidget) {
        connect(m_rightWidget, &KRightWidget::searchSignal, m_svcWidget, &ServiceWidget::searchSignal);
    }
    if (m_fsWidget) {
        connect(m_rightWidget, &KRightWidget::searchSignal, m_fsWidget, &FileSystemWidget::onSearch);
    }


    //平板
    QDBusInterface *m_statusSessionDbus = new QDBusInterface("com.lingmo.statusmanager.interface",
                                                  "/",
                                                  "com.lingmo.statusmanager.interface",
                                                  QDBusConnection::sessionBus(),this);

    if (m_statusSessionDbus->isValid()) {
        QDBusReply<bool> reply = m_statusSessionDbus->call("get_current_tabletmode");
        bool isTabletmode = reply.isValid() ? reply.value() : false;
        tabletmodeDbusSlot(isTabletmode);
        connect(m_statusSessionDbus, SIGNAL(mode_change_signal(bool)), this, SLOT(tabletmodeDbusSlot(bool)));
    } else {
        tabletmodeDbusSlot(false);
        qDebug() << "Create statusmanager Interface Failed When : " << QDBusConnection::systemBus().lastError();
    }

    connect(WindowManager::self(),&WindowManager::windowAdded,this,[=](const WindowId& windowId){
        if (getpid() == WindowManager::getPid(windowId)) {
            m_listWinIds.append(windowId);
        }
    });
    connect(WindowManager::self(),&WindowManager::windowRemoved,this,[=](const WindowId& windowId){
        if (m_listWinIds.contains(windowId)) {
            m_listWinIds.removeOne(windowId);
        }
    });

    connect(m_leftWidget, &KLeftWidget::updateCpuStatus, mCpuDetailsWidget, &CpuDetailsWidget::onUpdateData);
    connect(m_leftWidget, &KLeftWidget::updateMemStatus, mMemDetailsWidget, &MemDetailsWidget::onUpdateMemData);
    connect(m_leftWidget, &KLeftWidget::updateSwapStatus, mMemDetailsWidget, &MemDetailsWidget::onUpdateSwapData);
    connect(m_leftWidget, &KLeftWidget::onUpdateNetStatus,mNetDetailsWidget,&NetDetailsWidget::onUpdateData);
    connect(mCpuDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
    connect(mMemDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
    connect(mNetDetailsWidget, &BaseDetailViewWidget::hideDetails, this, &MainWindow::onSwitchPage);
}

void MainWindow::initOpacityGSettings()
{
    const QByteArray idtrans(THEME_QT_TRANS);
    if(QGSettings::isSchemaInstalled(idtrans)) {
        m_gsTransOpacity = new QGSettings(idtrans);
    }

    if (!m_gsTransOpacity) {
        m_curTransOpacity = 1;
        return;
    }

    connect(m_gsTransOpacity, &QGSettings::changed, this, [=](const QString &key) {
        if (key == "transparency") {
            QStringList keys = m_gsTransOpacity->keys();
            if (keys.contains("transparency")) {
                m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
                repaint();
            }
        }
    });

    QStringList keys = m_gsTransOpacity->keys();
    if(keys.contains("transparency")) {
        m_curTransOpacity = m_gsTransOpacity->get("transparency").toString().toDouble();
    }
}

void MainWindow::initLeftSideBar()
{
    // 初始化左侧列表
    unsigned btnIndex = 0;
    m_leftWidget = new KLeftWidget(this);
    m_mainLayout->addWidget(m_leftWidget, 0, Qt::AlignLeft);
}

void MainWindow::initRightPanel()
{
    //初始化右侧窗口
    unsigned panelIndex = 0;
    m_rightWidget = new KRightWidget(this);

    m_procWidget = new ProcessWidget(m_proSettings);
    m_rightWidget->addPanel(m_procWidget, tr("Processes"), QString(":/img/process_list.svg"), panelIndex++);

    if (m_isSurportService) {
        m_idxService = panelIndex;
        m_svcWidget = new ServiceWidget(m_proSettings);
        m_rightWidget->addPanel(m_svcWidget, tr("Services"), QString(":/img/service_list.svg"), panelIndex++);
    }

    m_fsWidget = new FileSystemWidget();
    m_rightWidget->addPanel(m_fsWidget, tr("Disks"), QString(":/img/filesystem_list.svg"), panelIndex++);

    mCpuDetailsWidget = new CpuDetailsWidget();
    m_rightWidget->addPanel(mCpuDetailsWidget, tr("Cpu Details"), "", panelIndex++);

    mMemDetailsWidget = new MemDetailsWidget();
    m_rightWidget->addPanel(mMemDetailsWidget, tr("Mem Details"), "", panelIndex++);

    mNetDetailsWidget = new NetDetailsWidget();
    m_rightWidget->addPanel(mNetDetailsWidget, tr("Net Details"), "", panelIndex++);

    m_mainLayout->addWidget(m_rightWidget, 1);
}

void MainWindow::handleMessage(const QString &msg)
{
    qDebug() << Q_FUNC_INFO << msg;
    if (this->isActiveWindow() && msg.compare("trayicon") == 0) {
        this->showMinimized();
    } else {
        QString platform = QGuiApplication::platformName();
        if(platform.startsWith(QLatin1String("wayland"),Qt::CaseInsensitive)) {
            if (!m_listWinIds.isEmpty()) {
                WindowManager::activateWindow(m_listWinIds.back());
            }
        } else {
            KWindowSystem::forceActiveWindow(this->winId());
        }
        this->windowShow();
        this->activateWindow();
    }
}

void MainWindow::onSwitchPage(int nIndex)
{
    qDebug()<<"SwitchPage:"<<nIndex;
    switchPage(nIndex);
    if (nIndex == m_idxService && m_svcWidget) {
        Q_EMIT m_svcWidget->switchOnWidget();
    }
}

void MainWindow::onMinimizeWindow()
{
    this->showMinimized();

}

void MainWindow::onMaximizeWindow()
{
    if (this->isMaximized()) {
        this->showNormal();
    }
    else {
        this->showMaximized();
    }
    if (m_rightWidget)
        m_rightWidget->onUpdateMaxBtnState();
}

void MainWindow::switchPage(int nIndex)
{
    if (nIndex < 0)
        return;
    if (m_leftWidget->currentIndex() != nIndex)
        m_leftWidget->onSwichPanel(nIndex);
    if (m_rightWidget->currentIndex() != nIndex) {
        m_rightWidget->onSwichPanel(nIndex);
    }
}

bool MainWindow::dblOnEdge(QMouseEvent *event)
{
    if (!event)
        return false;
    QPoint pos = event->globalPos();
    int globalMouseY = pos.y();

    int frameY = this->y();

    bool onTopEdges = (globalMouseY >= frameY &&
                       globalMouseY <= frameY + MAINWINDOW_DBCLICK_WIDTH);
    return onTopEdges;
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (this == watched) {

        if (event->type() == QEvent::MouseButtonDblClick) {
            QMouseEvent * mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton ){
                bool res = dblOnEdge(dynamic_cast<QMouseEvent*>(event));
                if (res) {
                    onMaximizeWindow();
                }
            }
        }

        if (event->type() == QEvent::WindowStateChange) {
            if (this->windowState() == Qt::WindowMaximized) {
                if (m_rightWidget)
                    m_rightWidget->updateMaxBtn(true);
            } else {
                if (m_rightWidget)
                    m_rightWidget->updateMaxBtn(false);
            }
        }
    }
    return QFrame::eventFilter(watched, event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if(event->key() == Qt::Key_F1) {
        qDebug() << "onF1ButtonClicked";
        QString service_name = "com.lingmoUserGuide.hotel_" + QString::number(getuid());
        QDBusInterface *interface = new QDBusInterface(service_name,
                                                       LINGMO_USER_GUIDE_PATH,
                                                       LINGMO_USER_GUIDE_INTERFACE,
                                                       QDBusConnection::sessionBus(),
                                                       this);
        QDBusMessage msg = interface->call("showGuide", "lingmo-system-monitor");
    }
    if (mFirstKey == 0 && event->key() == Qt::Key_Control) {
        mFirstKey = Qt::Key_Control;
    } else if (mFirstKey == Qt::Key_Control && event->key() == Qt::Key_E) {
        m_rightWidget->getSearchLineEdit()->setFocus();
    } else {
        mFirstKey = 0;
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent *e)
{
    mFirstKey = 0;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_lastWndSize = size();
    m_bIsWndMax = isMaximized();
    if (m_procWidget) {
        m_procWidget->onWndClose();
    }
    if (m_svcWidget) {
        m_svcWidget->onWndClose();
    }
    saveSettings();
    event->accept();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    QFrame::mouseMoveEvent(event);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    QFrame::mousePressEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    QFrame::mouseReleaseEvent(event);
}

bool MainWindow::loadSettings()
{
    bool bSuccess = false;
    if (m_proSettings) {
        m_proSettings->beginGroup("SystemMonitor");
        QVariant wndWidth = m_proSettings->value(MAINWINDOW_SIZE_W);
        QVariant wndHeight = m_proSettings->value(MAINWINDOW_SIZE_H);
        QVariant isWndMax = m_proSettings->value(MAINWINDOW_SIZE_MAX);
        m_proSettings->endGroup();
        if (wndWidth.isValid() && wndWidth.toInt() > 0 &&
            wndHeight.isValid() && wndHeight.toInt() > 0) {
            m_lastWndSize.setWidth(wndWidth.toInt());
            m_lastWndSize.setHeight(wndHeight.toInt());
        }
        if (isWndMax.isValid()) {
            m_bIsWndMax = isWndMax.toBool();
        }
        bSuccess = true;
    }
    return bSuccess;
}

void MainWindow::saveSettings()
{
    if (m_proSettings) {
        m_proSettings->beginGroup("SystemMonitor");
        m_proSettings->setValue(MAINWINDOW_SIZE_W, m_lastWndSize.width());
        m_proSettings->setValue(MAINWINDOW_SIZE_H, m_lastWndSize.height());
        m_proSettings->setValue(MAINWINDOW_SIZE_MAX, m_bIsWndMax);
        m_proSettings->endGroup();
        m_proSettings->sync();
    }
}

void MainWindow::getOsRelease()
{
    QFile file("/etc/lsb-release");
    if (!file.open(QIODevice::ReadOnly)) qDebug() << "Read file Failed.";
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        if (str.contains("DISTRIB_ID")){
            m_osVersion=str.remove("DISTRIB_ID=");
            m_osVersion=str.remove("\n");
        }
    }
}

void MainWindow::moveCenter()
{
    QPoint pos = QCursor::pos();
    QRect primaryGeometry;
    for (QScreen *screen : qApp->screens()) {
        if (screen->geometry().contains(pos)) {
            primaryGeometry = screen->geometry();
        }
    }

    if (primaryGeometry.isEmpty()) {
        primaryGeometry = qApp->primaryScreen()->geometry();
    }

    this->move(primaryGeometry.x() + (primaryGeometry.width() - this->width())/2,
               primaryGeometry.y() + (primaryGeometry.height() - this->height())/2);
}

void MainWindow::tabletmodeDbusSlot(bool isTabletmode)
{
    if (m_rightWidget) {
        m_rightWidget->changeTitleWidget(isTabletmode);
    }
    if (m_leftWidget) {
        m_leftWidget->setTabletMode(isTabletmode);
    }
}

void MainWindow::windowShow()
{
    kdk::LingmoUIStyleHelper::self()->removeHeader(this);
    KWindowEffects::enableBlurBehind(this->winId(), true);
    this->show();
}
