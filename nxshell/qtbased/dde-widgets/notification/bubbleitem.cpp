// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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
#include "style.h"
#include "helper.hpp"

#include <QTimer>
#include <QDateTime>
#include <QPropertyAnimation>
#include <QDebug>
#include <QProcess>
#include <QMouseEvent>
#include <QScroller>
#include <QPainterPath>
#include <QMenu>

#include <DIconButton>
#include <DStyleHelper>
#include <DGuiApplicationHelper>
#include <DFontSizeManager>
#include <DIconTheme>

AlphaWidget::AlphaWidget(QWidget *parent)
    : DWidget(parent)
{
}

void AlphaWidget::setHasFocus(bool focus)
{
    if (m_hasFocus == focus)
        return;

    m_hasFocus = focus;
    update();
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
    : BubbleBase(parent, entity)
    , m_entity(entity)
    , m_bgWidget(new AlphaWidget(this))
    , m_titleWidget(new AlphaWidget(this))
    , m_bodyWidget(new AlphaWidget(this))
    , m_appNameLabel(new DLabel(this))
    , m_appTimeLabel(new AppBodyLabel(this))
    , m_icon(new AppIcon(this))
    , m_body(new AppBody(this))
    , m_actionButton(new ActionButton(this, OSD::BUBBLEWIDGET))
{
    initUI();
    initContent();

    updateTabOrder();
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

    setWindowFlags(Qt::Widget);
    setFocusPolicy(Qt::StrongFocus);
    resize(OSD::BubbleSize(OSD::BUBBLEWIDGET));
    m_icon->setFixedSize(OSD::IconSize(OSD::BUBBLEWIDGET));
    m_closeButton->setFixedSize(UI::Bubble::buttonSize);
    m_closeButton->setIconSize(UI::Panel::clearIconSize);
    m_closeButton->setVisible(false);

    m_settingBtn->setFixedSize(UI::Bubble::buttonSize);
    m_settingBtn->setIconSize(UI::Panel::settingsIconSize);
    m_settingBtn->setVisible(false);

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
    m_actionButton->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    DFontSizeManager::instance()->bind(m_appNameLabel, DFontSizeManager::T8);
    DFontSizeManager::instance()->bind(m_appTimeLabel, DFontSizeManager::T8);

    setAlpha(Notify::BubbleDefaultAlpha);
    titleLayout->addWidget(m_settingBtn);
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

    installEventFilter(this);
    m_settingBtn->installEventFilter(this);
    m_closeButton->installEventFilter(this);
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
    connect(this, &BubbleItem::focusStateChanged, this, &BubbleItem::onFocusStateChanged);
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
            text =  tr("Just now");
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

    return BubbleBase::mousePressEvent(event);
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

    return BubbleBase::mouseReleaseEvent(event);
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
    return BubbleBase::keyPressEvent(event);
}

void BubbleItem::onFocusStateChanged(bool focus)
{
    if (m_showContent) {
        m_closeButton->setVisible(focus);
        m_settingBtn->setVisible(focus);
        m_appTimeLabel->setVisible(!focus);
        m_bgWidget->setHasFocus(focus);
    }
}

void BubbleItem::onCloseBubble()
{
    m_view->createRemoveAnimation(this);
    Q_EMIT bubbleRemove();
}

void BubbleItem::setParentModel(NotifyModel *model)
{
    BubbleBase::setParentModel(model);

    connect(model, &NotifyModel::removedNotif, this, [this] {
        if (!m_actionId.isEmpty()) {
            BubbleTool::actionInvoke(m_actionId, m_entity);
            Q_EMIT SignalBridge::ref().actionInvoked(m_entity->id(), m_actionId);
        }
    });
}

void BubbleItem::setParentView(NotifyListView *view)
{
    BubbleBase::setParentView(view);
    connect(m_view, &NotifyListView::refreshItemTime, this, &BubbleItem::onRefreshTime);
}

void BubbleItem::refreshTheme()
{
    m_appNameLabel->setForegroundRole(QPalette::BrightText);
}

QList<QPointer<QWidget>> BubbleItem::bubbleElements() const
{
    QList<QPointer<QWidget>> bubble_elements{BubbleBase::bubbleElements()};
    foreach (auto btn, m_actionButton->buttonList()) {
        bubble_elements.append(btn);
    }
    return bubble_elements;
}

int BubbleItem::indexRow()
{
    return m_entity->currentIndex();
}

int BubbleItem::bubbleItemHeight()
{
    int appBodyHeight = qMax(AppBody::bubbleWidgetAppBodyHeight(), BubbleItemBodyHeight);
    int bubbleTitleHeight = qMax(QFontMetrics(DFontSizeManager::instance()->t8()).height(), BubbleItemTitleHeight);

    return appBodyHeight + bubbleTitleHeight;
}

bool BubbleItem::realHasFocus() const
{
    return hasFocus() || m_settingBtn->hasFocus() || m_closeButton->hasFocus();
}

bool BubbleItem::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::FocusIn:
    case QEvent::FocusOut:
        // it maybe a bug for Qt, QAbstractItemView::edit() would execute `w->setFocus()`
        // when Mouse Moveing while ItemDelegate's flags has ItemIsEditable,
        // so we remove flags of Model's ItemIsEditable.
//            if (e->reason() == Qt::OtherFocusReason)
//                break;
        if (watched == this || watched == m_settingBtn || watched == m_closeButton)
            focusStateChanged(realHasFocus());

        break;
    case QEvent::Enter:
    case QEvent::Leave:
        if (watched == this)
            focusStateChanged(event->type() == QEvent::Enter);

        break;
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

BubbleBase::BubbleBase(QWidget *parent, EntityPtr entity)
    : QWidget(parent)
    , m_closeButton(new CicleIconButton(this))
    , m_settingBtn(new CicleIconButton(this))
    , m_appName(entity->appName())
{
    m_settingBtn->setAccessibleName("SettingButton");
    m_settingBtn->setObjectName(m_appName + "-Settings");
    m_settingBtn->setIcon(DDciIcon::fromTheme("notify_more"));
    connect(m_settingBtn, &CicleIconButton::clicked, this, &BubbleBase::showSettingsMenu);

    m_closeButton->setAccessibleName("CloseButton");
    m_closeButton->setObjectName(m_appName + "-CloseButton");
    m_closeButton->setIcon(DIconTheme::findQIcon("notify_clear"));
}

void BubbleBase::setParentModel(NotifyModel *model)
{
    Q_ASSERT(model);
    m_model = model;
}

void BubbleBase::setParentView(NotifyListView *view)
{
    m_view = view;
}

QList<QPointer<QWidget> > BubbleBase::bubbleElements() const
{
    QList<QPointer<QWidget>> bubble_elements;
    bubble_elements.append(m_settingBtn);
    bubble_elements.append(m_closeButton);
    return bubble_elements;
}

void BubbleBase::updateTabOrder()
{
    auto focusElements = bubbleElements();
    focusElements.prepend(this);
    for (int i = 1; i < focusElements.count(); i++) {
        focusElements[i]->setFocusPolicy(Qt::TabFocus);
        QWidget::setTabOrder(focusElements[i - 1], focusElements[i]);
    }
}

void BubbleBase::showSettingsMenu()
{
    QMenu *menu = new QMenu(this);

    do {
        const bool isTopping = m_model->isAppTopping(m_appName);
        QAction *action = menu->addAction(isTopping ? tr("Unpin") : tr("Pin"));
        action->setCheckable(true);

        connect(action, &QAction::triggered, this, &BubbleBase::toggleAppTopping);
    } while (false);

     do {
         QAction *action = menu->addAction(tr("Notification settings"));
         action->setCheckable(true);

         connect(action, &QAction::triggered, this, &BubbleBase::showNotificationModuleOfControlCenter);
     } while (false);

    // using relation position instead of QCursor's position
    // avoid to error in triggering by Key_Tab scene.
    m_settingBtn->setDown(true);
    menu->exec(m_settingBtn->mapToGlobal(QPoint(0, m_settingBtn->geometry().bottom() + UI::Panel::settingSpacingBetweenMenu)));
    menu->deleteLater();
    m_settingBtn->setDown(false);
}

void BubbleBase::toggleAppTopping()
{
    const bool isTopping = m_model->isAppTopping(m_appName);
    m_model->setAppTopping(m_appName, !isTopping);

    m_model->refreshAppTopping();
}

void BubbleBase::showNotificationModuleOfControlCenter()
{
    Helper::instance()->showNotificationModuleOfControlCenter();
}
