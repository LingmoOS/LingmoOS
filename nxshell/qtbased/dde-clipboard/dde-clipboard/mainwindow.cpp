// SPDX-FileCopyrightText: 2018 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "displaymanager.h"
#include "constants.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QScrollBar>
#include <QScreen>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

#include <DFontSizeManager>
#include <DGuiApplicationHelper>

#define DOCK_TOP        0
#define DOCK_RIGHT      1
#define DOCK_BOTTOM     2
#define DOCK_LEFT       3

#define MONITOR_SERVICE "org.deepin.dde.XEventMonitor1"

const QString AppearanceService = QStringLiteral("org.deepin.dde.Appearance1");
const QString AppearancePath = QStringLiteral("/org/deepin/dde/Appearance1");
const QString AppearanceInterface = QStringLiteral("org.deepin.dde.Appearance1");

static QColor outerBorderColor = QColor(0, 0, 0, static_cast<int>(0.15 * 255));
static QColor innerBorderColor = QColor(255, 255, 255, static_cast<int>(0.2 * 255));
#define ALPHA_OFFSET 10

MainWindow::MainWindow(QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_displayInter(new DBusDisplay("org.deepin.dde.Display1", "/org/deepin/dde/Display1", QDBusConnection::sessionBus(), this))
    , m_daemonDockInter(new DBusDaemonDock("org.deepin.dde.daemon.Dock1", "/org/deepin/dde/daemon/Dock1", QDBusConnection::sessionBus(), this))
    , m_dockInter(new DBusDockInterface)
    , m_appearanceInter(new DDBusInterface(AppearanceService, AppearancePath, AppearanceInterface, QDBusConnection::sessionBus(), this))
    , m_regionMonitor(nullptr)
    , m_content(new DWidget(parent))
    , m_listview(new ListView(this))
    , m_model(new ClipboardModel(m_listview))
    , m_itemDelegate(new ItemDelegate(m_listview))
    , m_xAni(new QPropertyAnimation(this))
    , m_widthAni(new QPropertyAnimation(this))
    , m_aniGroup(new QSequentialAnimationGroup(this))
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_trickTimer(new QTimer)
    , m_windowHandle(new DPlatformWindowHandle(this, this))
{
    initUI();
    initAni();
    initConnect();

    geometryChanged();
    CompositeChanged();
    registerMonitor();

    installEventFilter(this);
}

MainWindow::~MainWindow()
{

}

double MainWindow::opacity()
{
    return qvariant_cast<double>(m_appearanceInter->property("Opacity"));
}

void MainWindow::Toggle()
{
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    if (isVisible()) {
        hideAni();
    } else {
        showAni();
    }
}

void MainWindow::geometryChanged()
{
    adjustPosition();

    setX(WindowMargin);

    //init animation by 'm_rect'
    m_xAni->setStartValue(WindowMargin);
    m_xAni->setEndValue(0);

    m_widthAni->setStartValue(m_rect.width());
    m_widthAni->setEndValue(0);
}

void MainWindow::showAni()
{
    if (m_trickTimer->isActive()) {
        return;
    }
    m_trickTimer->start();

    if (!m_hasComposite) {
        move(m_rect.x() + WindowMargin, m_rect.y());
        setFixedWidth(m_rect.width());
        show();
        return;
    }

    move(m_rect.x(), m_rect.y());
    setFixedWidth(0);

    show();
    m_aniGroup->setDirection(QAbstractAnimation::Backward);
    m_aniGroup->start();
}

void MainWindow::hideAni()
{
    if (m_trickTimer->isActive()) {
        return;
    }
    m_trickTimer->start();

    if (m_alwaysShow) {
        return;
    }
    if (!m_hasComposite) {
        hide();
        return;
    }
    m_aniGroup->setDirection(QAbstractAnimation::Forward);
    m_aniGroup->start();

    QTimer::singleShot(m_aniGroup->duration(), this, [ = ] {setVisible(false);});
}

void MainWindow::Show()
{
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    if (!isVisible()) {
        showAni();
    }
}

void MainWindow::Hide()
{
    if (m_alwaysShow) {
        return;
    }
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    if (isVisible()) {
        hideAni();
    }
}

void MainWindow::setX(int x)
{
    move(m_rect.x() + x, m_rect.y());
}

void MainWindow::CompositeChanged()
{
    // 屏蔽应用自己的动画，默认使用窗管窗口显示动画
    m_hasComposite = false;
}

void MainWindow::registerMonitor()
{
    if (m_regionMonitor) {
        delete m_regionMonitor;
        m_regionMonitor = nullptr;
    }
    m_regionMonitor = new DRegionMonitor(this);
    m_regionMonitor->registerRegion(QRegion(QRect()));
    connect(m_regionMonitor, &DRegionMonitor::buttonRelease, this, [ = ](const QPoint &p, const int flag) {
        Q_UNUSED(flag);
        if (!geometry().contains(p))
            if (!isHidden()) {
                hideAni();
            }
    });
}

void MainWindow::initUI()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool  | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout *mainLayout = new QVBoxLayout();
    mainLayout->setContentsMargins(0, 0, 0, 10);
    mainLayout->setSpacing(0);

    QWidget *titleWidget = new QWidget;
    QHBoxLayout *titleLayout = new QHBoxLayout(titleWidget);
    titleLayout->setContentsMargins(20, 0, 10, 0);
    titleLayout->setAlignment(Qt::AlignVCenter);

    QLabel *titleLabel = new QLabel(tr("Clipboard"), this);
    DFontSizeManager::instance()->bind(titleLabel, DFontSizeManager::T3);

    m_clearButton = new IconButton(tr("Clear all"), this);
    connect(m_clearButton, &IconButton::clicked, m_model, &ClipboardModel::clear);

    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    titleLayout->addWidget(m_clearButton);
    m_clearButton->setFixedHeight(36);
    m_clearButton->setBackOpacity(200);
    m_clearButton->setRadius(8);
    m_clearButton->setVisible(false);
    titleWidget->setFixedSize(WindowWidth, WindowTitleHeight);

    m_listview->setModel(m_model);
    m_listview->setItemDelegate(m_itemDelegate);
    m_listview->setFixedWidth(WindowWidth);//需固定，否则动画会变形

    mainLayout->addWidget(titleWidget);
    mainLayout->addWidget(m_listview);

    m_content->setLayout(mainLayout);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(m_content);

    setMaskAlpha(static_cast<int>(this->opacity() * 255));
    setFocusPolicy(Qt::NoFocus);
    
    m_trickTimer->setInterval(300);
    m_trickTimer->setSingleShot(true);

    m_cornerRadius = m_windowHandle->windowRadius();
    m_themeType= DGuiApplicationHelper::instance()->themeType();
    m_windowHandle->setBorderWidth(1);
}

void MainWindow::initAni()
{
    m_xAni->setEasingCurve(QEasingCurve::Linear);
    m_xAni->setPropertyName("x");
    m_xAni->setTargetObject(this);
    m_xAni->setDuration(AnimationTime / 2);

    m_widthAni->setEasingCurve(QEasingCurve::Linear);
    m_widthAni->setPropertyName("width");
    m_widthAni->setTargetObject(this);
    m_widthAni->setDuration(AnimationTime);

    m_aniGroup->addAnimation(m_xAni);
    m_aniGroup->addAnimation(m_widthAni);
}

void MainWindow::initConnect()
{
    connect(DisplayManager::instance(), &DisplayManager::screenInfoChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);
    connect(m_displayInter, &DBusDisplay::PrimaryRectChanged, this, &MainWindow::geometryChanged, Qt::QueuedConnection);

    connect(m_model, &ClipboardModel::dataChanged, this, [ = ] {
        m_clearButton->setVisible(m_model->data().size() != 0);
    });

    connect(m_model, &ClipboardModel::dataReborn, this, [ = ] {
        hideAni();
    });

    connect(m_dockInter, &DBusDockInterface::geometryChanged, this, &MainWindow::geometryChanged, Qt::UniqueConnection);

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &MainWindow::CompositeChanged, Qt::QueuedConnection);

    connect(m_widthAni, &QVariantAnimation::valueChanged, this, [ = ](QVariant value) {
        int width = value.toInt();
        m_content->move(width - 300, m_content->pos().y());
    });

    QDBusServiceWatcher *m_watcher = new QDBusServiceWatcher(MONITOR_SERVICE, QDBusConnection::sessionBus());
    connect(m_watcher, &QDBusServiceWatcher::serviceRegistered, this, [ = ](const QString &service){
        if (MONITOR_SERVICE != service)
            return;
        registerMonitor();
    });

    connect(m_watcher, &QDBusServiceWatcher::serviceUnregistered, this, [ = ](const QString &service){
        if (MONITOR_SERVICE != service)
            return;
        disconnect(m_regionMonitor);
    });

    connect(this, &MainWindow::OpacityChanged, this, [this](double alpha) {
        setMaskAlpha(static_cast<int>(alpha * 255));
    });

    auto setOuterBorderColor = [this]() {
        auto outerBorderNewColor = outerBorderColor;
        if (m_themeType == DGuiApplicationHelper::ColorType::DarkType) {
            outerBorderNewColor.setAlpha(maskAlpha() + ALPHA_OFFSET * 2);
        }

        m_windowHandle->setBorderColor(outerBorderNewColor);
    };

    connect(this, &DBlurEffectWidget::maskAlphaChanged, [this, setOuterBorderColor]() {
        setOuterBorderColor();
        update();
    });

    connect(m_windowHandle, &DPlatformWindowHandle::windowRadiusChanged, this, [this](){
        if (m_cornerRadius == m_windowHandle->windowRadius())
            return;

        m_cornerRadius = m_windowHandle->windowRadius();
        update();
    });

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [this, setOuterBorderColor](DGuiApplicationHelper::ColorType type){
        if (!m_windowHandle || type == m_themeType)
            return;

        m_themeType = type;
        setOuterBorderColor();
        update();
    });

    setOuterBorderColor();
}

void MainWindow::adjustPosition()
{
    // 屏幕尺寸
    QRect rect = getDisplayScreen();
    rect.setWidth(WindowWidth);
    rect.setHeight(int(std::round(qreal(rect.height()))));

    QRect dockRect = m_dockInter->geometry();
    dockRect.setWidth(int(std::round(qreal(dockRect.width()))));
    dockRect.setHeight(int(std::round(qreal(dockRect.height()))));

    // 初始化剪切板位置
    switch (m_daemonDockInter->position()) {
    case DOCK_TOP:
        rect.moveTop(rect.top() + dockRect.height());
        rect.setHeight(rect.height() - dockRect.height());
        break;
    case DOCK_BOTTOM:
        rect.setHeight(rect.height() - dockRect.height());
        break;
    case DOCK_LEFT:
        rect.moveLeft(rect.left() + dockRect.width());
        break;
    default:;
    }

    // 左上下部分预留的间隙
    rect -= QMargins(0, WindowMargin, 0, WindowMargin);

    // 针对时尚模式的特殊处理
    // 只有任务栏显示的时候, 才额外偏移
    if(m_daemonDockInter->displayMode() == 0 && dockRect.width() * dockRect.height() > 0) {
        switch (m_daemonDockInter->position()) {
        case DOCK_TOP:
            rect -= QMargins(0, WindowMargin, 0, 0);
            break;
        case DOCK_BOTTOM:
            rect -= QMargins(0, 0, 0, WindowMargin);
            break;
        case DOCK_LEFT:
            rect -= QMargins(WindowMargin, 0, 0, 0);
            break;
        default:;
        }
    }

    setGeometry(rect);
    m_rect = rect;
    setFixedSize(rect.size());
    m_content->setFixedSize(rect.size());
}

QRect MainWindow::getDisplayScreen()
{
    QPoint dockCenterPoint = QRect(m_dockInter->geometry()).center() / qApp->devicePixelRatio();

    for (auto s : qApp->screens()) {
        QRect rect(s->geometry().x() / qApp->devicePixelRatio(),
                   s->geometry().y() / qApp->devicePixelRatio(),
                   s->geometry().width(), s->geometry().height());
        if (rect.contains(dockCenterPoint)) {
            return s->geometry();
        }
    }
    return qApp->primaryScreen() ? qApp->primaryScreen()->geometry() : QRect();
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //禁止窗口被拖动
    Q_UNUSED(event);
    return;
}

void MainWindow::paintEvent(QPaintEvent *e)
{
    DBlurEffectWidget::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen;
    pen.setWidth(1);

    auto innerBorderNewColor = innerBorderColor;
    if (m_themeType != DGuiApplicationHelper::DarkType) {
        innerBorderNewColor.setAlpha(maskAlpha()+ ALPHA_OFFSET);
    }

    pen.setColor(innerBorderNewColor);
    p.setPen(pen);
    p.drawRoundedRect(rect(), m_cornerRadius, m_cornerRadius);
}

void MainWindow::showEvent(QShowEvent *event)
{
    Q_EMIT clipboardVisibleChanged(true);
    DBlurEffectWidget::showEvent(event);
}

void MainWindow::hideEvent(QHideEvent *event)
{
    Q_EMIT clipboardVisibleChanged(false);
    DBlurEffectWidget::hideEvent(event);
}