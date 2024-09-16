// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bubbleitem.h"
#include "notification/notificationentity.h"
#include "notification/appicon.h"
#include "notification/appbody.h"
#include "notification/actionbutton.h"
#include "notification/button.h"
#include "notification/icondata.h"
#include "notification/bubbletool.h"
#include "notification/iconbutton.h"
#include "notification/appbodylabel.h"
#include "notifymodel.h"
#include "notifylistview.h"
#include "notification/signalbridge.h"

#include <QTimer>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QDebug>
#include <QProcess>
#include <QMouseEvent>
#include <QScroller>
#include <QPainterPath>

#include <DIconButton>
#include <DStyleHelper>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>

AlphaWidget::AlphaWidget(QWidget *parent)
    : DWidget(parent)
{
}

void AlphaWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QPalette pe = this->palette();
    QColor brushColor(pe.color(QPalette::Base));
    brushColor.setAlpha(m_hasFocus ? m_hoverAlpha : m_unHoverAlpha);
    painter.setBrush(brushColor);

    QPen borderPen;
    borderPen.setColor(Qt::transparent);
    painter.setPen(borderPen);

    QRect rect = this->rect();
    const QPoint topLeft = rect.topLeft();
    const QPoint topRight = rect.topRight();
    const QPoint bottomLeft = rect.bottomLeft();
    const QPoint bottomRight = rect.bottomRight();

    QPainterPath path;
    path.moveTo(topLeft.x() + m_topRedius, topRight.y());
    path.lineTo(topRight.x() - m_topRedius, topRight.y());
    path.arcTo(topRight.x() - 2 * m_topRedius, topRight.y(), 2 * m_topRedius, 2 * m_topRedius, 90, -90);
    path.lineTo(bottomRight.x(), bottomRight.y() - m_bottomRedius);
    path.arcTo(bottomRight.x() - 2 * m_bottomRedius, bottomRight.y() - 2 * m_bottomRedius, 2 * m_bottomRedius, 2 * m_bottomRedius, 0, -90);
    path.lineTo(bottomLeft.x() + m_bottomRedius, bottomLeft.y());
    path.arcTo(bottomLeft.x(), bottomLeft.y() - 2 * m_bottomRedius, 2 * m_bottomRedius, 2 * m_bottomRedius, 270, -90);
    path.lineTo(topLeft.x(), topLeft.y() - m_topRedius);
    path.arcTo(topLeft.x(), topLeft.y(), 2 * m_topRedius, 2 * m_topRedius, 180, -90);
    painter.drawPath(path);
}

BubbleItem::BubbleItem(QWidget *parent, EntityPtr entity)
    : QWidget(parent)
    , m_entity(entity)
    , m_bgWidget(new AlphaWidget(this))
    , m_titleWidget(new AlphaWidget(this))
    , m_bodyWidget(new AlphaWidget(this))
    , m_appNameLabel(new DLabel(this))
    , m_appTimeLabel(new AppBodyLabel(this))
    , m_icon(new AppIcon(this))
    , m_body(new AppBody(this))
    , m_actionButton(new ActionButton(this, OSD::BUBBLEWIDGET))
    , m_closeButton(new DIconButton(DStyle::SP_CloseButton, this))

{
    initUI();
    initContent();
}

BubbleItem::~BubbleItem()
{
}

void BubbleItem::initUI()
{
    m_bgWidget->setAccessibleName("BgWidget");
    m_titleWidget->setAccessibleName("TitleWidget");
    m_bodyWidget->setAccessibleName("BodyWidget");
    m_appNameLabel->setAccessibleName("AppNameLabel");
    m_appTimeLabel->setAccessibleName("AppTimeLabel");
    m_icon->setAccessibleName("AppIcon");
    m_body->setAccessibleName("AppBody");
    m_actionButton->setAccessibleName("ActionButton");
    m_closeButton->setAccessibleName("CloseButton");

    setWindowFlags(Qt::Widget);
    setFocusPolicy(Qt::StrongFocus);
    resize(OSD::BubbleSize(OSD::BUBBLEWIDGET));
    m_icon->setFixedSize(OSD::IconSize(OSD::BUBBLEWIDGET));
    m_closeButton->setFlat(true);
    m_closeButton->setContentsMargins(0, 0, 0, 0);
    m_closeButton->setFixedSize(Notify::BubbleCloseButtonSize - 4, Notify::BubbleCloseButtonSize - 4);
    m_closeButton->setIconSize(QSize(Notify::BubbleCloseButtonSize, Notify::BubbleCloseButtonSize));
    m_closeButton->setVisible(false);

    m_titleWidget->setFixedHeight(BubbleItemTitleHeight);
    m_titleWidget->setObjectName("notification_title");
    m_titleWidget->setRadius(8, 0);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing(0);
    mainLayout->setMargin(0);

    QHBoxLayout *titleLayout = new QHBoxLayout;
    titleLayout->setSpacing(10);
    titleLayout->setContentsMargins(10, 0, 10, 0);
    titleLayout->addWidget(m_icon);
    titleLayout->addWidget(m_appNameLabel);
    titleLayout->addWidget(m_appTimeLabel);

    m_appNameLabel->setForegroundRole(DPalette::TextTitle);
    m_appNameLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_appNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_appTimeLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    m_appTimeLabel->setOpacity(0.6);
    m_appTimeLabel->setForegroundRole(QPalette::BrightText);
    m_appTimeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    m_actionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    DFontSizeManager::instance()->bind(m_appNameLabel, DFontSizeManager::T8);
    DFontSizeManager::instance()->bind(m_appTimeLabel, DFontSizeManager::T8);

    setAlpha(Notify::BubbleDefaultAlpha);
    titleLayout->addWidget(m_closeButton);
    m_titleWidget->setLayout(titleLayout);
    m_titleWidget->setFixedHeight(qMax(m_appNameLabel->fontMetrics().height(), BubbleItemTitleHeight));
    mainLayout->addWidget(m_titleWidget);
    m_body->setStyle(OSD::BUBBLEWIDGET);
    m_body->setObjectName("notification_body");
    QHBoxLayout *bodyLayout = new QHBoxLayout;
    bodyLayout->setSpacing(0);
    bodyLayout->setContentsMargins(10, 0, 10, 0);
    bodyLayout->addWidget(m_body);
    bodyLayout->addWidget(m_actionButton);

    m_bodyWidget->setLayout(bodyLayout);
    m_bodyWidget->setRadius(0, 8);
    mainLayout->addWidget(m_bodyWidget);
    m_bgWidget->setLayout(mainLayout);
    m_bgWidget->setRadius(8, 8);

    QHBoxLayout *l = new QHBoxLayout;
    l->setSpacing(0);
    l->setMargin(0);
    l->addWidget(m_bgWidget);
    setLayout(l);

    BubbleTool::processIconData(m_icon, m_entity);
    m_defaultAction = BubbleTool::processActions(m_actionButton, m_entity->actions());
    setFixedHeight(bubbleItemHeight());
}

void BubbleItem::initContent()
{
    if (m_entity == nullptr) return;

    m_body->setTitle(m_entity->summary());
    m_body->setText(OSD::removeHTML(m_entity->body()));
    m_appNameLabel->setText(BubbleTool::getDeepinAppName(m_entity->appName()));
    onRefreshTime();

    connect(m_actionButton, &ActionButton::buttonClicked, this, [ = ](const QString & id) {
        m_actionId = id;
        if (m_model != nullptr)
            onCloseBubble();
    });
    connect(this, &BubbleItem::havorStateChanged, this, &BubbleItem::onHavorStateChanged);
    connect(m_closeButton, &DIconButton::clicked, this, &BubbleItem::onCloseBubble);
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &BubbleItem::refreshTheme);
    refreshTheme();
}

void BubbleItem::setAlpha(int alpha)
{
    m_titleWidget->setAlpha(alpha);
    m_bodyWidget->setAlpha(0);
    m_bgWidget->setHoverAlpha(alpha * 5);
    m_bgWidget->setUnHoverAlpha(alpha * 3);
}

void BubbleItem::onRefreshTime()
{
    qint64 msec = QDateTime::currentMSecsSinceEpoch() - m_entity->ctime().toLongLong();
    if (msec < 0) {
        return;
    }

    QString text;

    QDateTime bubbleDateTime = QDateTime::fromMSecsSinceEpoch(m_entity->ctime().toLongLong());
    QDateTime currentDateTime = QDateTime::currentDateTime();
    int elapsedDay = int(bubbleDateTime.daysTo(currentDateTime));
    int minute = int(msec / 1000 / 60);

    if (elapsedDay == 0) {
        if (minute == 0) {
            text =  tr("Just Now");
        } else if (minute > 0 && minute < 60) {
            text = tr("%1 minutes ago").arg(minute);
        } else {
            text = tr("%1 hours ago").arg(minute / 60);
        }
    } else if (elapsedDay >= 1 && elapsedDay < 2) {
        text = tr("Yesterday ") + " " + bubbleDateTime.toString("hh:mm");
    } else if (elapsedDay >= 2 && elapsedDay < 7) {
        text = bubbleDateTime.toString("ddd hh:mm");
    } else {
        text = bubbleDateTime.toString("yyyy/MM/dd");
    }
    m_appTimeLabel->setText(text);
}

void BubbleItem::setOverlapWidget(bool isOverlap)
{
    m_isOverlapWidget = isOverlap;
}

void BubbleItem::mousePressEvent(QMouseEvent *event)
{
    m_pressPoint = event->pos();

    return DWidget::mousePressEvent(event);
}

void BubbleItem::mouseReleaseEvent(QMouseEvent *event)
{
    if (m_isOverlapWidget) {
        return DWidget::mouseReleaseEvent(event);
    }

    if (m_pressPoint == event->pos()) {
        if(!m_defaultAction.isEmpty())
        {
            BubbleTool::actionInvoke(m_defaultAction, m_entity);
            m_defaultAction.clear();

            if (m_model != nullptr)
                onCloseBubble();
        }
    }

    return DWidget::mouseReleaseEvent(event);
}

void BubbleItem::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return) {
        if(m_defaultAction.isNull()) {
            return DWidget::keyPressEvent(event);
        } else {
            BubbleTool::actionInvoke(m_defaultAction, m_entity);
            m_defaultAction.clear();
            if (m_model != nullptr)
                onCloseBubble();
        }
    }
    return DWidget::keyPressEvent(event);
}

void BubbleItem::enterEvent(QEvent *event)
{
    if (m_view == nullptr)
        return;
    if (!QScroller::hasScroller(m_view)) {
        setFocus();
        m_view->setCurrentRow(m_entity->currentIndex());
        Q_EMIT havorStateChanged(true);
    }

    return DWidget::enterEvent(event);
}

void BubbleItem::leaveEvent(QEvent *event)
{
    // QScroller::hasScroller用于判断listview是否处于滑动状态，滑动状态不触发paint相关操作，否则滑动动画异常
    bool hasScroller = QScroller::hasScroller(m_view);
    if (!hasScroller) {
        Q_EMIT havorStateChanged(false);
    } else {
        // 滚动结束,处理hover变化
        connect(QScroller::scroller(m_view), &QScroller::stateChanged, this, [this](const QScroller::State state){
            if (state == QScroller::Inactive) {
                Q_EMIT havorStateChanged(false);
            }
        });
    }

    return DWidget::leaveEvent(event);
}

void BubbleItem::focusInEvent(QFocusEvent *event)
{
    m_bgWidget->setHasFocus(true);
    Q_EMIT havorStateChanged(true);
    return DWidget::focusInEvent(event);
}

void BubbleItem::focusOutEvent(QFocusEvent *event)
{
    m_bgWidget->setHasFocus(false);
    Q_EMIT havorStateChanged(false);
    return DWidget::focusOutEvent(event);
}

void BubbleItem::onHavorStateChanged(bool hover)
{
    if (m_showContent) {
        m_closeButton->setVisible(hover);
        if (hover) {
            QFocusEvent event(QEvent::Leave, Qt::MouseFocusReason);
            QApplication::sendEvent (m_closeButton, &event);
        }
        m_appTimeLabel->setVisible(!hover);
    }
}

void BubbleItem::onCloseBubble()
{
    m_view->createRemoveAnimation(this);
    Q_EMIT bubbleRemove();
}

void BubbleItem::setParentModel(NotifyModel *model)
{
    Q_ASSERT(model);
    m_model = model;

    connect(model, &NotifyModel::removedNotif, this, [this] {
        if (!m_actionId.isEmpty()) {
            BubbleTool::actionInvoke(m_actionId, m_entity);
            Q_EMIT SignalBridge::ref().actionInvoked(m_entity->id(), m_actionId);
        }
    });
}

void BubbleItem::setParentView(NotifyListView *view)
{
    m_view = view;
    connect(m_view, &NotifyListView::refreshItemTime, this, &BubbleItem::onRefreshTime);
}

void BubbleItem::refreshTheme()
{
    m_appNameLabel->setForegroundRole(QPalette::BrightText);
}

QList<QPointer<QWidget>> BubbleItem::bubbleElements()
{
    QList<QPointer<QWidget>> bubble_elements;
    bubble_elements.append(m_closeButton); m_actionButton->buttonList();
    foreach (auto btn, m_actionButton->buttonList()) {
        bubble_elements.append(btn);
    }
    return bubble_elements;
}

int BubbleItem::indexRow()
{
    return m_entity->currentIndex();
}

void BubbleItem::setHasFocus(bool focus)
{
    m_bgWidget->setHasFocus(focus);
    Q_EMIT havorStateChanged(focus);
}

int BubbleItem::bubbleItemHeight()
{
    int appBodyHeight = qMax(AppBody::bubbleWidgetAppBodyHeight(), BubbleItemBodyHeight);
    int bubbleTitleHeight = qMax(QFontMetrics(DFontSizeManager::instance()->t8()).height(), BubbleItemTitleHeight);

    return appBodyHeight + bubbleTitleHeight;
}
