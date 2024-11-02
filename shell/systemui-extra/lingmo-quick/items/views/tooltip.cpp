/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Artur Duque de Souza <asouza@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2024 iaom <zhangpengfei@kylinos.cn>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "tooltip.h"

#include <QTextDocument>

namespace LingmoUIQuick {

// ====== Tooltip ======
TooltipDialog *Tooltip::s_dialog = nullptr;
int Tooltip::s_dialogUsers = 0;

Tooltip::Tooltip(QQuickItem *parent)
        : QQuickItem(parent)
        , m_tooltipsEnabledGlobally(true)
        , m_containsMouse(false)
        , m_location(LingmoUIQuick::Dialog::PopupLocation::Floating)
        , m_active(true)
        , m_timeout(10000)
        , m_usingDialog(false)
        , m_interval(500)
        , m_posFollowCursor(true)
        , m_margin(0)
        , m_interactive(false)
        , m_deactivatedByClick(false)
{
    setAcceptHoverEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
    setFiltersChildMouseEvents(true);

    m_showTimer = new QTimer(this);
    m_showTimer->setSingleShot(true);
    connect(m_showTimer, &QTimer::timeout, this, &Tooltip::showTooltip);
}

Tooltip::~Tooltip()
{
    if (s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }

    if (m_usingDialog) {
        --s_dialogUsers;
    }

    if (s_dialogUsers == 0) {
        delete s_dialog;
        s_dialog = nullptr;
    }
}

QQuickItem *Tooltip::mainItem() const
{
    return m_mainItem.data();;
}

void Tooltip::setMainItem(QQuickItem *mainItem)
{
    if (m_mainItem.data() != mainItem) {
        m_mainItem = mainItem;

        Q_EMIT mainItemChanged();

        if (!isValid() && s_dialog && s_dialog->owner() == this) {
            s_dialog->setVisible(false);
        }
    }
}

QString Tooltip::mainText() const
{
    return m_mainText;
}

void Tooltip::setMainText(const QString &mainText)
{
    if (mainText == m_mainText) {
        return;
    }

    m_mainText = mainText;
    Q_EMIT mainTextChanged();

    if (m_isRichText != Qt::mightBeRichText(m_mainText)) {
        m_isRichText = !m_isRichText;
        Q_EMIT isRichTextChanged();
    }

    if (!isValid() && s_dialog && s_dialog->owner() == this) {
        s_dialog->setVisible(false);
    }
}

LingmoUIQuick::Dialog::PopupLocation Tooltip::location() const
{
    return m_location;
}

void Tooltip::setLocation(LingmoUIQuick::Dialog::PopupLocation location)
{
    if (m_location == location) {
        return;
    }
    m_location = location;
    Q_EMIT locationChanged();
}

bool Tooltip::containsMouse() const
{
    return m_containsMouse;
}

void Tooltip::setContainsMouse(bool contains)
{
    if (m_containsMouse != contains) {
        m_containsMouse = contains;
        Q_EMIT containsMouseChanged();
    }
    if (!contains) {
        tooltipDialogInstance()->dismiss();
    }
}

void Tooltip::setActive(bool active)
{
    if (m_active == active) {
        return;
    }

    m_active = active;
    if (!active) {
        tooltipDialogInstance()->dismiss();
    }
    Q_EMIT activeChanged();
}

void Tooltip::setTimeout(int timeout)
{
    m_timeout = timeout;
}

void Tooltip::showTooltip()
{
    if (!m_active || m_deactivatedByClick) {
        return;
    }

    Q_EMIT aboutToShow();

    TooltipDialog *dlg = tooltipDialogInstance();

    if (!mainItem()) {
        setMainItem(dlg->loadDefaultItem());
    }

    // Unset the dialog's old contents before reparenting the dialog.
    dlg->setMainItem(nullptr);

    LingmoUIQuick::Dialog::PopupLocation location = m_location;
    if (m_location == LingmoUIQuick::Dialog::PopupLocation::Floating) {
        QQuickItem *p = parentItem();
        while (p) {
            if (p->property("location").isValid()) {
                location = (LingmoUIQuick::Dialog::PopupLocation)p->property("location").toInt();
                break;
            }
            p = p->parentItem();
        }
    }

    if (mainItem()) {
        mainItem()->setProperty("tooltip", QVariant::fromValue(this));
        mainItem()->setVisible(true);
    }

    connect(dlg, &TooltipDialog::visibleChanged, this, &Tooltip::toolTipVisibleChanged, Qt::UniqueConnection);

    dlg->setHideTimeout(m_timeout);
    dlg->setOwner(this);
    dlg->setLocation(location);
    dlg->setPosFollowCursor(m_posFollowCursor);
    dlg->setMargin(m_margin);
    dlg->setVisualParent(this);
    dlg->setMainItem(mainItem());
    dlg->setInteractive(m_interactive);
    dlg->setVisible(true);
}

void Tooltip::hideTooltip()
{
    m_showTimer->stop();
    tooltipDialogInstance()->dismiss();
}

void Tooltip::hideImmediately()
{
    m_showTimer->stop();
    tooltipDialogInstance()->setVisible(false);
}

bool Tooltip::childMouseEventFilter(QQuickItem *item, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        hideTooltip();
    }
    return QQuickItem::childMouseEventFilter(item, event);
}

void Tooltip::hoverEnterEvent(QHoverEvent *event)
{
    setContainsMouse(true);
    m_deactivatedByClick = false;
    if (!m_tooltipsEnabledGlobally) {
        return QQuickItem::hoverEnterEvent(event);
    }

    if (!isValid()) {
        return QQuickItem::hoverEnterEvent(event);
    }

    if (tooltipDialogInstance()->isVisible()) {
        // We signal the tooltipmanager that we're "potentially interested,
        // and ask to keep it open for a bit, so other items get the chance
        // to update the content before the tooltip hides -- this avoids
        // flickering
        // It need to be considered only when other items can deal with tooltip area
        if (m_active && !m_deactivatedByClick) {
            tooltipDialogInstance()->keepalive();
            // FIXME: showToolTip needs to be renamed in sync or something like that
            showTooltip();
        }
    } else {
        m_showTimer->start(m_interval);
    }
    QQuickItem::hoverEnterEvent(event);
}

void Tooltip::hoverLeaveEvent(QHoverEvent *event)
{
    Q_UNUSED(event)
    setContainsMouse(false);
    m_showTimer->stop();
}

void Tooltip::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    m_deactivatedByClick = true;
    hideImmediately();
    event->ignore();
    QQuickItem::mousePressEvent(event);
}
TooltipDialog *Tooltip::tooltipDialogInstance()
{
    if (!s_dialog) {
        s_dialog = new TooltipDialog;
        s_dialogUsers = 1;
    }

    if (!m_usingDialog) {
        s_dialogUsers++;
        m_usingDialog = true;
    }

    return s_dialog;
}

bool Tooltip::isValid() const
{
    return m_mainItem || !mainText().isEmpty();
}

bool Tooltip::posFollowCursor()
{
    return m_posFollowCursor;
}

void Tooltip::setPosFollowCursor(bool follow)
{
    if(m_posFollowCursor == follow) {
        return;
    }
    m_posFollowCursor = follow;
    Q_EMIT posFollowCursorChanged();
}

int Tooltip::margin() const
{
    return m_margin;
}

void Tooltip::setMargin(int margin)
{
    if(m_margin != margin) {
        m_margin = margin;
        Q_EMIT marginChanged();
    }
}

void Tooltip::setInteractive(bool interactive)
{
    if (m_interactive == interactive) {
        return;
    }
    m_interactive = interactive;
    Q_EMIT interactiveChanged();
}
} // LingmoUIQuick
