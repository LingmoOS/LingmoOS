/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2011 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Dominik Haumann <dhaumann@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/
#include "kmessagewidget.h"

#include <QAction>
#include <QApplication>
#include <QEvent>
#include <QGridLayout>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QShowEvent>
#include <QStyle>
#include <QStyleOption>
#include <QTimeLine>
#include <QToolButton>
//---------------------------------------------------------------------
// KMessageWidgetPrivate
//---------------------------------------------------------------------

constexpr int borderSize = 2;

class KMessageWidgetPrivate
{
public:
    void init(KMessageWidget *);

    KMessageWidget *q;
    QLabel *iconLabel = nullptr;
    QLabel *textLabel = nullptr;
    QToolButton *closeButton = nullptr;
    QTimeLine *timeLine = nullptr;
    QIcon icon;
    bool ignoreShowAndResizeEventDoingAnimatedShow = false;
    KMessageWidget::MessageType messageType;
    KMessageWidget::Position position = KMessageWidget::Inline;
    bool wordWrap;
    QList<QToolButton *> buttons;
    bool ignorePaletteChange = false;

    void createLayout();
    void setPalette();
    void updateLayout();
    void slotTimeLineChanged(qreal);
    void slotTimeLineFinished();
    int bestContentHeight() const;
};

void KMessageWidgetPrivate::init(KMessageWidget *q_ptr)
{
    q = q_ptr;
    // Note: when changing the value 500, also update KMessageWidgetTest
    timeLine = new QTimeLine(500, q);
    QObject::connect(timeLine, &QTimeLine::valueChanged, q, [this](qreal value) {
        slotTimeLineChanged(value);
    });
    QObject::connect(timeLine, &QTimeLine::finished, q, [this]() {
        slotTimeLineFinished();
    });

    wordWrap = false;

    iconLabel = new QLabel(q);
    iconLabel->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    iconLabel->hide();

    textLabel = new QLabel(q);
    textLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    textLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
    QObject::connect(textLabel, &QLabel::linkActivated, q, &KMessageWidget::linkActivated);
    QObject::connect(textLabel, &QLabel::linkHovered, q, &KMessageWidget::linkHovered);

    QAction *closeAction = new QAction(q);
    closeAction->setText(KMessageWidget::tr("&Close", "@action:button"));
    closeAction->setToolTip(KMessageWidget::tr("Close message", "@info:tooltip"));
    QStyleOptionFrame opt;
    opt.initFrom(q);
    closeAction->setIcon(q->style()->standardIcon(QStyle::SP_DialogCloseButton, &opt, q));

    QObject::connect(closeAction, &QAction::triggered, q, &KMessageWidget::animatedHide);

    closeButton = new QToolButton(q);
    closeButton->setAutoRaise(true);
    closeButton->setDefaultAction(closeAction);

    q->setMessageType(KMessageWidget::Information);
}

void KMessageWidgetPrivate::createLayout()
{
    delete q->layout();

    qDeleteAll(buttons);
    buttons.clear();

    const auto actions = q->actions();
    buttons.reserve(actions.size());
    for (QAction *action : actions) {
        QToolButton *button = new QToolButton(q);
        button->setDefaultAction(action);
        button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        auto previous = buttons.isEmpty() ? static_cast<QWidget *>(textLabel) : buttons.back();
        QWidget::setTabOrder(previous, button);
        buttons.append(button);
    }

    // AutoRaise reduces visual clutter, but we don't want to turn it on if
    // there are other buttons, otherwise the close button will look different
    // from the others.
    closeButton->setAutoRaise(buttons.isEmpty());
    if (wordWrap) {
        QGridLayout *layout = new QGridLayout(q);
        // Set alignment to make sure icon does not move down if text wraps
        layout->addWidget(iconLabel, 0, 0, 1, 1, Qt::AlignCenter);
        layout->addWidget(textLabel, 0, 1);

        if (buttons.isEmpty()) {
            // Use top-vertical alignment like the icon does.
            layout->addWidget(closeButton, 0, 2, 1, 1, Qt::AlignHCenter | Qt::AlignTop);
        } else {
            // Use an additional layout in row 1 for the buttons.
            QHBoxLayout *buttonLayout = new QHBoxLayout;
            buttonLayout->addStretch();
            for (QToolButton *button : std::as_const(buttons)) {
                // For some reason, calling show() is necessary if wordwrap is true,
                // otherwise the buttons do not show up. It is not needed if
                // wordwrap is false.
                button->show();
                buttonLayout->addWidget(button);
            }
            buttonLayout->addWidget(closeButton);
            layout->addItem(buttonLayout, 1, 0, 1, 2);
        }
    } else {
        QHBoxLayout *layout = new QHBoxLayout(q);
        layout->addWidget(iconLabel, 0, Qt::AlignVCenter);
        layout->addWidget(textLabel);

        for (QToolButton *button : std::as_const(buttons)) {
            layout->addWidget(button, 0, Qt::AlignTop);
        }
        layout->addWidget(closeButton, 0, Qt::AlignTop);
    };
    // Add bordersize to the margin so it starts from the inner border and doesn't look too cramped
    q->layout()->setContentsMargins(q->layout()->contentsMargins() + borderSize);
    if (q->isVisible()) {
        q->setFixedHeight(q->sizeHint().height());
    }
    q->updateGeometry();
}

void KMessageWidgetPrivate::setPalette()
{
    QColor bgBaseColor;

    // We have to hardcode colors here because KWidgetsAddons is a tier 1 framework
    // and therefore can't depend on any other KDE Frameworks
    // The following RGB color values come from the "default" scheme in kcolorscheme.cpp
    switch (messageType) {
    case KMessageWidget::Positive:
        bgBaseColor.setRgb(39, 174, 96); // Window: ForegroundPositive
        break;
    case KMessageWidget::Information:
        bgBaseColor.setRgb(61, 174, 233); // Window: ForegroundActive
        break;
    case KMessageWidget::Warning:
        bgBaseColor.setRgb(246, 116, 0); // Window: ForegroundNeutral
        break;
    case KMessageWidget::Error:
        bgBaseColor.setRgb(218, 68, 83); // Window: ForegroundNegative
        break;
    }
    QPalette palette = q->palette();
    palette.setColor(QPalette::Window, bgBaseColor);
    const QColor parentTextColor = (q->parentWidget() ? q->parentWidget()->palette() : qApp->palette()).color(QPalette::WindowText);
    palette.setColor(QPalette::WindowText, parentTextColor);
    // Explicitly set the palettes of the labels because some apps use stylesheets which break the
    // palette propagation
    ignorePaletteChange = true;
    q->setPalette(palette);
    ignorePaletteChange = false;
    iconLabel->setPalette(palette);
    textLabel->setPalette(palette);

    // update the Icon in case it is recolorable
    q->setIcon(icon);
    q->update();
}

void KMessageWidgetPrivate::updateLayout()
{
    createLayout();
}

void KMessageWidgetPrivate::slotTimeLineChanged(qreal value)
{
    q->setFixedHeight(qMin(value * 2, qreal(1.0)) * bestContentHeight());
    q->update();
}

void KMessageWidgetPrivate::slotTimeLineFinished()
{
    if (timeLine->direction() == QTimeLine::Forward) {
        q->resize(q->width(), bestContentHeight());

        // notify about finished animation
        Q_EMIT q->showAnimationFinished();
    } else {
        // hide and notify about finished animation
        q->hide();
        Q_EMIT q->hideAnimationFinished();
    }
}

int KMessageWidgetPrivate::bestContentHeight() const
{
    int height = q->heightForWidth(q->width());
    if (height == -1) {
        height = q->sizeHint().height();
    }
    return height;
}

//---------------------------------------------------------------------
// KMessageWidget
//---------------------------------------------------------------------
KMessageWidget::KMessageWidget(QWidget *parent)
    : QFrame(parent)
    , d(new KMessageWidgetPrivate)
{
    d->init(this);
}

KMessageWidget::KMessageWidget(const QString &text, QWidget *parent)
    : QFrame(parent)
    , d(new KMessageWidgetPrivate)
{
    d->init(this);
    setText(text);
}

KMessageWidget::~KMessageWidget() = default;

QString KMessageWidget::text() const
{
    return d->textLabel->text();
}

void KMessageWidget::setText(const QString &text)
{
    d->textLabel->setText(text);
    updateGeometry();
}

Qt::TextFormat KMessageWidget::textFormat() const
{
    return d->textLabel->textFormat();
}

void KMessageWidget::setTextFormat(Qt::TextFormat textFormat)
{
    d->textLabel->setTextFormat(textFormat);
}

KMessageWidget::MessageType KMessageWidget::messageType() const
{
    return d->messageType;
}

void KMessageWidget::setMessageType(KMessageWidget::MessageType type)
{
    d->messageType = type;
    d->setPalette();
}

QSize KMessageWidget::sizeHint() const
{
    ensurePolished();
    return QFrame::sizeHint();
}

QSize KMessageWidget::minimumSizeHint() const
{
    ensurePolished();
    return QFrame::minimumSizeHint();
}

bool KMessageWidget::event(QEvent *event)
{
    if (event->type() == QEvent::Polish && !layout()) {
        d->createLayout();
    } else if ((event->type() == QEvent::Show && !d->ignoreShowAndResizeEventDoingAnimatedShow)
               || (event->type() == QEvent::LayoutRequest && d->timeLine->state() == QTimeLine::NotRunning)) {
        setFixedHeight(d->bestContentHeight());

        // if we are displaying this when application first starts, there's
        // a possibility that the layout is not properly updated with the
        // rest of the application because the setFixedHeight call above has
        // the same height that was set beforehand, when we lacked a parent
        // and thus, the layout() geometry is bogus. so we pass a bogus
        // value to it, just to trigger a recalculation, and revert to the
        // best content height.
        if (geometry().height() < layout()->geometry().height()) {
            setFixedHeight(d->bestContentHeight() + 2); // this triggers a recalculation.
            setFixedHeight(d->bestContentHeight()); // this actually sets the correct values.
        }

    } else if (event->type() == QEvent::ParentChange) {
        d->setPalette();
    } else if (event->type() == QEvent::PaletteChange) {
        if (!d->ignorePaletteChange) {
            d->setPalette();
        }
    }
    return QFrame::event(event);
}

void KMessageWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    if (d->timeLine->state() == QTimeLine::NotRunning && d->ignoreShowAndResizeEventDoingAnimatedShow) {
        setFixedHeight(d->bestContentHeight());
    }
}

int KMessageWidget::heightForWidth(int width) const
{
    ensurePolished();
    return QFrame::heightForWidth(width);
}

void KMessageWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    QPainter painter(this);
    if (d->timeLine->state() == QTimeLine::Running) {
        painter.setOpacity(d->timeLine->currentValue() * d->timeLine->currentValue());
    }
    constexpr float radius = 4 * 0.6;
    const QRect innerRect = rect().marginsRemoved(QMargins() + borderSize / 2);
    const QColor color = palette().color(QPalette::Window);
    constexpr float alpha = 0.2;
    const QColor parentWindowColor = (parentWidget() ? parentWidget()->palette() : qApp->palette()).color(QPalette::Window);
    const int newRed = (color.red() * alpha) + (parentWindowColor.red() * (1 - alpha));
    const int newGreen = (color.green() * alpha) + (parentWindowColor.green() * (1 - alpha));
    const int newBlue = (color.blue() * alpha) + (parentWindowColor.blue() * (1 - alpha));

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setBrush(QColor(newRed, newGreen, newBlue));
    if (d->position == Position::Inline) {
        painter.setPen(QPen(color, borderSize));
        painter.drawRoundedRect(innerRect, radius, radius);
        return;
    }

    painter.setPen(QPen(Qt::NoPen));
    painter.drawRect(rect());

    if (d->position == Position::Header) {
        painter.setPen(QPen(color, 1));
        painter.drawLine(0, rect().height(), rect().width(), rect().height());
    } else {
        painter.setPen(QPen(color, 1));
        painter.drawLine(0, 0, rect().width(), 0);
    }
}

bool KMessageWidget::wordWrap() const
{
    return d->wordWrap;
}

void KMessageWidget::setWordWrap(bool wordWrap)
{
    d->wordWrap = wordWrap;
    d->textLabel->setWordWrap(wordWrap);
    QSizePolicy policy = sizePolicy();
    policy.setHeightForWidth(wordWrap);
    setSizePolicy(policy);
    d->updateLayout();
    // Without this, when user does wordWrap -> !wordWrap -> wordWrap, a minimum
    // height is set, causing the widget to be too high.
    // Mostly visible in test programs.
    if (wordWrap) {
        setMinimumHeight(0);
    }
}

KMessageWidget::Position KMessageWidget::position() const
{
    return d->position;
}

void KMessageWidget::setPosition(KMessageWidget::Position position)
{
    d->position = position;
    updateGeometry();
}

bool KMessageWidget::isCloseButtonVisible() const
{
    return d->closeButton->isVisible();
}

void KMessageWidget::setCloseButtonVisible(bool show)
{
    d->closeButton->setVisible(show);
    updateGeometry();
}

void KMessageWidget::addAction(QAction *action)
{
    QFrame::addAction(action);
    d->updateLayout();
}

void KMessageWidget::removeAction(QAction *action)
{
    QFrame::removeAction(action);
    d->updateLayout();
}

void KMessageWidget::clearActions()
{
    const auto ourActions = actions();
    for (auto *action : ourActions) {
        removeAction(action);
    }
    d->updateLayout();
}

void KMessageWidget::animatedShow()
{
    // Test before styleHint, as there might have been a style change while animation was running
    if (isHideAnimationRunning()) {
        d->timeLine->stop();
        Q_EMIT hideAnimationFinished();
    }

    if (!style()->styleHint(QStyle::SH_Widget_Animate, nullptr, this) || (parentWidget() && !parentWidget()->isVisible())) {
        show();
        Q_EMIT showAnimationFinished();
        return;
    }

    if (isVisible() && (d->timeLine->state() == QTimeLine::NotRunning) && (height() == d->bestContentHeight())) {
        Q_EMIT showAnimationFinished();
        return;
    }

    d->ignoreShowAndResizeEventDoingAnimatedShow = true;
    show();
    d->ignoreShowAndResizeEventDoingAnimatedShow = false;
    setFixedHeight(0);

    d->timeLine->setDirection(QTimeLine::Forward);
    if (d->timeLine->state() == QTimeLine::NotRunning) {
        d->timeLine->start();
    }
}

void KMessageWidget::animatedHide()
{
    // test this before isVisible, as animatedShow might have been called directly before,
    // so the first timeline event is not yet done and the widget is still hidden
    // And before styleHint, as there might have been a style change while animation was running
    if (isShowAnimationRunning()) {
        d->timeLine->stop();
        Q_EMIT showAnimationFinished();
    }

    if (!style()->styleHint(QStyle::SH_Widget_Animate, nullptr, this)) {
        hide();
        Q_EMIT hideAnimationFinished();
        return;
    }

    if (!isVisible()) {
        // explicitly hide it, so it stays hidden in case it is only not visible due to the parents
        hide();
        Q_EMIT hideAnimationFinished();
        return;
    }

    d->timeLine->setDirection(QTimeLine::Backward);
    if (d->timeLine->state() == QTimeLine::NotRunning) {
        d->timeLine->start();
    }
}

bool KMessageWidget::isHideAnimationRunning() const
{
    return (d->timeLine->direction() == QTimeLine::Backward) && (d->timeLine->state() == QTimeLine::Running);
}

bool KMessageWidget::isShowAnimationRunning() const
{
    return (d->timeLine->direction() == QTimeLine::Forward) && (d->timeLine->state() == QTimeLine::Running);
}

QIcon KMessageWidget::icon() const
{
    return d->icon;
}

void KMessageWidget::setIcon(const QIcon &icon)
{
    d->icon = icon;
    if (d->icon.isNull()) {
        d->iconLabel->hide();
    } else {
        const int size = style()->pixelMetric(QStyle::PM_ToolBarIconSize);
        d->iconLabel->setPixmap(d->icon.pixmap(size));
        d->iconLabel->show();
    }
}

#include "moc_kmessagewidget.cpp"
