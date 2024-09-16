// SPDX-FileCopyrightText: 2011 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "notifycenterwidget.h"
#include "notification/persistence.h"
#include "notification/constants.h"
#include "notification/iconbutton.h"

#include <QDesktopWidget>
#include <QBoxLayout>
#include <QDBusInterface>
#include <QPalette>
#include <QDebug>
#include <QTimer>
#include <QScreen>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

#include <DIconButton>
#include <DLabel>
#include <DFontSizeManager>
#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE

NotifyCenterWidget::NotifyCenterWidget(AbstractPersistence *database, QWidget *parent)
    : DBlurEffectWidget(parent)
    , m_notifyWidget(new NotifyWidget(this, database))
    , m_xAni(new QPropertyAnimation(this, "x"))
    , m_widthAni(new QPropertyAnimation(this, "width"))
    , m_aniGroup(new QSequentialAnimationGroup(this))
    , m_wmHelper(DWindowManagerHelper::instance())
    , m_regionMonitor(new DRegionMonitor(this))
    , m_trickTimer(new QTimer(this))
{
    initUI();
    initConnections();
    initAnimations();

    CompositeChanged();

    m_regionMonitor->setCoordinateType(DRegionMonitor::Original);

    m_trickTimer->setInterval(300);
    m_trickTimer->setSingleShot(true);
}

void NotifyCenterWidget::initUI()
{
    m_notifyWidget->setAccessibleName("NotifyWidget");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool | Qt::MSWindowsFixedSizeDialogHint | Qt::WindowStaysOnTopHint | Qt::X11BypassWindowManagerHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setContentsMargins(0, 0, 0, 0);

    m_headWidget = new QWidget;
    m_headWidget->setAccessibleName("HeadWidget");
    m_headWidget->setFixedSize(Notify::CenterWidth - 2 * Notify::CenterMargin, Notify::CenterTitleHeight);

    DIconButton *bell_notify = new DIconButton(m_headWidget);
    bell_notify->setAccessibleName("BellNotifyBtn");
    bell_notify->setFlat(true);
    bell_notify->setIconSize(QSize(32, 32));
    bell_notify->setFixedSize(32, 32);
    const auto ratio = devicePixelRatioF();
    QIcon icon_pix = QIcon::fromTheme("://icons/notifications.svg").pixmap(bell_notify->iconSize() * ratio);
    bell_notify->setIcon(icon_pix);
    bell_notify->setFocusPolicy(Qt::NoFocus);

    title_label = new DLabel;
    title_label->setAccessibleName("TitleLabel");
    QFont font = title_label->font();
    font.setBold(true);
    font.setWeight(QFont::DemiBold);
    title_label->setFont(font);
    title_label->setText(tr("Notification Center"));
    title_label->setAlignment(Qt::AlignCenter | Qt::AlignBottom);
    title_label->setForegroundRole(QPalette::BrightText);
    DFontSizeManager::instance()->bind(title_label, DFontSizeManager::T5);

    m_clearButton = new IconButton;
    m_clearButton->setAccessibleName("ClearButton");
    m_clearButton->setOpacity(IconButton::RELEASE, 255 * 0.0);
    m_clearButton->setRadius(Notify::CenterTitleHeight / 2);
    m_clearButton->setFixedSize(Notify::CenterTitleHeight, Notify::CenterTitleHeight);
    m_clearButton->setFocusPolicy(Qt::StrongFocus);

    QHBoxLayout *head_Layout = new QHBoxLayout;
    head_Layout->addWidget(bell_notify, Qt::AlignLeft | Qt::AlignTop);
    head_Layout->setMargin(0);
    head_Layout->addStretch();
    head_Layout->addWidget(title_label, Qt::AlignCenter | Qt::AlignBottom);
    head_Layout->addStretch();
    head_Layout->addWidget(m_clearButton, Qt::AlignRight | Qt::AlignTop);
    m_headWidget->setLayout(head_Layout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(Notify::CenterMargin, Notify::CenterMargin, 0, 0);
    mainLayout->addWidget(m_headWidget);
    mainLayout->addWidget(m_notifyWidget);

    setLayout(mainLayout);

    connect(m_clearButton, &IconButton::clicked, this, [ = ]() {
        m_notifyWidget->model()->removeAllData();
    });

    refreshTheme();
}

void NotifyCenterWidget::initAnimations()
{
    m_xAni->setEasingCurve(QEasingCurve::Linear);
    m_xAni->setDuration(AnimationTime / 2);

    m_widthAni->setEasingCurve(QEasingCurve::Linear);
    m_widthAni->setDuration(AnimationTime);

    m_aniGroup->addAnimation(m_xAni);
    m_aniGroup->addAnimation(m_widthAni);
}

void NotifyCenterWidget::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &NotifyCenterWidget::refreshTheme);

    connect(m_widthAni, &QVariantAnimation::valueChanged, this, [ = ](const QVariant & value) {
        int width = value.toInt();
        move(m_notifyRect.x() + m_notifyRect.width() + Notify::CenterMargin - width, m_notifyRect.y());
    });

    connect(m_wmHelper, &DWindowManagerHelper::hasCompositeChanged, this, &NotifyCenterWidget::CompositeChanged, Qt::QueuedConnection);

    connect(m_notifyWidget, &NotifyWidget::focusOnButton, this, [=] {
        qDebug() << "set Focus on clearButton";
        m_clearButton->setFocus();
        m_clearButton->update();
    });
}

void NotifyCenterWidget::updateGeometry(QRect screen, QRect dock, OSD::DockPosition pos, int mode)
{
    // 重新根据字体设置一下标题的高度
    m_headWidget->setFixedHeight(qMax(title_label->fontMetrics().boundingRect(title_label->text()).height(), Notify::CenterTitleHeight));

    m_scale = qApp->primaryScreen()->devicePixelRatio();
    m_dockRect = dock;

    int width = Notify::CenterWidth;
    int height = screen.height() - Notify::CenterMargin * 2;
    if (pos == OSD::DockPosition::Top || pos == OSD::DockPosition::Bottom) {
        if(mode == OSD::DockModel::Fashion) {
            height = screen.height() - Notify::CenterMargin * 2 - dock.height();
            if (dock.height() != 0) {
                height -= OSD::DockMargin * 2;
            }
        } else {
            height = screen.height() - Notify::CenterMargin * 2 - dock.height();
        }
    }

    int x = screen.x() + screen.width() - Notify::CenterWidth - Notify::CenterMargin;
    if (pos == OSD::DockPosition::Right) {
        if(mode == OSD::DockModel::Fashion) {
            x =  screen.x() + screen.width() - (Notify::CenterWidth + dock.width() + OSD::DockMargin * 2 + Notify::CenterMargin);
            if (dock.width() == 0) {
                x += OSD::DockMargin * 2;
            }
        } else {
            x =  screen.x() + screen.width() - (Notify::CenterWidth + dock.width() + Notify::CenterMargin);
        }
    }

    int y = screen.y() + Notify::CenterMargin;
    if (pos == OSD::DockPosition::Top) {
        if(mode == OSD::DockModel::Fashion) {
            y = screen.y() + Notify::CenterMargin + dock.height() + OSD::DockMargin * 2;
        } else {
            y = screen.y() + Notify::CenterMargin + dock.height();
        }
    }

    m_notifyRect = QRect(x, y, width, height);
    setGeometry(m_notifyRect);

    m_notifyWidget->setFixedWidth(m_notifyRect.width() - Notify::CenterMargin);
    setFixedSize(m_notifyRect.size());

    m_xAni->setStartValue(m_notifyRect.x());
    m_xAni->setEndValue(m_notifyRect.x() + Notify::CenterMargin);

    m_widthAni->setStartValue(int(m_notifyRect.width()));
    m_widthAni->setEndValue(0);
}

void NotifyCenterWidget::mouseMoveEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    return; 
}

void NotifyCenterWidget::hideEvent(QHideEvent *event)
{
    unRegisterRegion();

    return DBlurEffectWidget::hideEvent(event);
}

void NotifyCenterWidget::refreshTheme()
{
    QPalette pa = title_label->palette();
    pa.setBrush(QPalette::WindowText, pa.brightText());
    title_label->setPalette(pa);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        m_clearButton->setIcon("://icons/list_icon_clear.svg");
        m_clearButton->setOpacity(IconButton::HOVER, 255 * 0.2);
        m_clearButton->setOpacity(IconButton::PRESS, 255 * 0.3);
    } else {
        m_clearButton->setIcon("://icons/list_icon_clear_dark.svg");
        m_clearButton->setOpacity(IconButton::HOVER, 255 * 0.2);
        m_clearButton->setOpacity(IconButton::PRESS, 255 * 0.1);
    }
}

void NotifyCenterWidget::showAni()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    registerRegion();

    if (!m_hasComposite) {
        setGeometry(QRect(m_notifyRect.x(), m_notifyRect.y(), m_notifyRect.width(), m_notifyRect.height()));
        setFixedSize(m_notifyRect.size());
        show();
        activateWindow();
        return;
    }
    setFixedWidth(0);
    move(m_notifyRect.x() + m_notifyRect.width(), m_notifyRect.y());
    show();
    activateWindow();

    m_aniGroup->setDirection(QAbstractAnimation::Backward);
    m_aniGroup->start();
}

void NotifyCenterWidget::hideAni()
{
    if (m_trickTimer->isActive()) {
        return;
    }

    m_trickTimer->start();

    if (!m_hasComposite) {
        hide();
        return;
    }

    m_aniGroup->setDirection(QAbstractAnimation::Forward);
    m_aniGroup->start();

    QTimer::singleShot(m_aniGroup->duration(), this, [ = ] {
        setVisible(false);
    });
}

void NotifyCenterWidget::registerRegion()
{
    QDBusInterface interface("org.deepin.dde.XEventMonitor1", "/org/deepin/dde/XEventMonitor1",
                             "org.deepin.dde.XEventMonitor1",
                             QDBusConnection::sessionBus());
    if (interface.isValid()) {
        m_regionConnect = connect(m_regionMonitor, &DRegionMonitor::buttonRelease, this, [ = ](const QPoint & p) {
            QPoint pScale(int(qreal(p.x() / m_scale)), int(qreal(p.y() / m_scale)));
            if (!geometry().contains(pScale))
                if (!isHidden()) {
                    hideAni();
                }
        });
    }

    if (!m_regionMonitor->registered()) {
        m_regionMonitor->registerRegion();
    }
}

void NotifyCenterWidget::unRegisterRegion()
{
    if (m_regionMonitor->registered()) {
        m_regionMonitor->unregisterRegion();
        disconnect(m_regionConnect);
    }
}

void NotifyCenterWidget::CompositeChanged()
{
    // 屏蔽应用自己的动画，默认使用窗管窗口显示动画
    m_hasComposite = false;
}

void NotifyCenterWidget::setX(int x)
{
    move(x, m_notifyRect.y());
}

void NotifyCenterWidget::showWidget()
{
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    return isHidden() ? showAni() : hideAni();
}

void NotifyCenterWidget::onlyShowWidget()
{
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    return showAni();
}

void NotifyCenterWidget::onlyHideWidget()
{
    if (m_aniGroup->state() == QAbstractAnimation::Running)
        return;

    return hideAni();
}
