/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Felix Ernst <fe.a.ernst@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later OR BSD-2-Clause
*/

#include "ktooltiphelper.h"
#include "ktooltiphelper_p.h"

#include <KColorScheme>
#include <KLocalizedString>

#include <QAction>
#include <QApplication>
#include <QCursor>
#include <QDesktopServices>
#include <QHelpEvent>
#include <QMenu>
#include <QStyle>
#include <QToolButton>
#include <QToolTip>
#include <QWhatsThis>
#include <QWhatsThisClickedEvent>
#include <QWindow>
#include <QtGlobal>

KToolTipHelper *KToolTipHelper::instance()
{
    return KToolTipHelperPrivate::instance();
}

KToolTipHelper *KToolTipHelperPrivate::instance()
{
    if (!s_instance) {
        s_instance = new KToolTipHelper(qApp);
    }
    return s_instance;
}

KToolTipHelper::KToolTipHelper(QObject *parent)
    : QObject{parent}
    , d{new KToolTipHelperPrivate(this)}
{
}

KToolTipHelperPrivate::KToolTipHelperPrivate(KToolTipHelper *qq)
    : q{qq}
{
    m_toolTipTimeout.setSingleShot(true);
    connect(&m_toolTipTimeout, &QTimer::timeout, this, &KToolTipHelperPrivate::postToolTipEventIfCursorDidntMove);
}

KToolTipHelper::~KToolTipHelper() = default;

KToolTipHelperPrivate::~KToolTipHelperPrivate() = default;

bool KToolTipHelper::eventFilter(QObject *watched, QEvent *event)
{
    return d->eventFilter(watched, event);
}

bool KToolTipHelperPrivate::eventFilter(QObject *watched, QEvent *event)
{
    switch (event->type()) {
    case QEvent::Hide:
        return handleHideEvent(watched, event);
    case QEvent::KeyPress:
        return handleKeyPressEvent(event);
    case QEvent::ToolTip:
        return handleToolTipEvent(watched, static_cast<QHelpEvent *>(event));
    case QEvent::WhatsThisClicked:
        return handleWhatsThisClickedEvent(event);
    default:
        return false;
    }
}

const QString KToolTipHelper::whatsThisHintOnly()
{
    return KToolTipHelperPrivate::whatsThisHintOnly();
}

const QString KToolTipHelperPrivate::whatsThisHintOnly()
{
    return QStringLiteral("tooltip bug"); // if a user ever sees this, there is a bug somewhere.
}

bool KToolTipHelperPrivate::handleHideEvent(QObject *watched, QEvent *event)
{
    if (event->spontaneous()) {
        return false;
    }
    const QMenu *menu = qobject_cast<QMenu *>(watched);
    if (!menu) {
        return false;
    }

    m_cursorGlobalPosWhenLastMenuHid = QCursor::pos();
    m_toolTipTimeout.start(menu->style()->styleHint(QStyle::SH_ToolTip_WakeUpDelay, nullptr, menu));
    return false;
}

bool KToolTipHelperPrivate::handleKeyPressEvent(QEvent *event)
{
    if (!QToolTip::isVisible() || static_cast<QKeyEvent *>(event)->key() != Qt::Key_Shift || !m_widget) {
        return false;
    }

    if (!m_lastToolTipWasExpandable) {
        return false;
    }

    QToolTip::hideText();
    // We need to explicitly hide the tooltip window before showing the whatsthis because hideText()
    // runs a timer before hiding. On Wayland when hiding a popup Qt will close all popups opened after
    // it, including the whatsthis popup here. Unfortunately we can't access the tooltip window/widget
    // directly so we search for it below.
    Q_ASSERT(QApplication::focusWindow());
    const auto windows = QGuiApplication::allWindows();
    auto it = std::find_if(windows.begin(), windows.end(), [](const QWindow *window) {
        return window->type() == Qt::ToolTip && QGuiApplication::focusWindow()->isAncestorOf(window);
    });
    if (it != windows.end()) {
        (*it)->setVisible(false);
    }

    if (QMenu *menu = qobject_cast<QMenu *>(m_widget)) {
        if (m_action) {
            // The widget displaying the whatsThis() text tries to avoid covering the QWidget
            // given as the third parameter of QWhatsThis::showText(). Normally we would have
            // menu as the third parameter but because QMenus are quite big the text panel
            // oftentimes fails to find a nice position around it and will instead cover
            // the hovered action itself! To avoid this we give a smaller positioningHelper-widget
            // as the third parameter which only has the size of the hovered menu action entry.
            QWidget *positioningHelper = new QWidget(menu); // Needs to be alive as long as the help is shown or hyperlinks can't be opened.
            positioningHelper->setGeometry(menu->actionGeometry(m_action));
            QWhatsThis::showText(m_lastExpandableToolTipGlobalPos, m_action->whatsThis(), positioningHelper);
            connect(menu, &QMenu::aboutToHide, positioningHelper, &QObject::deleteLater);
        }
        return true;
    }
    QWhatsThis::showText(m_lastExpandableToolTipGlobalPos, m_widget->whatsThis(), m_widget);
    return true;
}

bool KToolTipHelperPrivate::handleMenuToolTipEvent(QMenu *menu, QHelpEvent *helpEvent)
{
    Q_CHECK_PTR(helpEvent);
    Q_CHECK_PTR(menu);

    m_action = menu->actionAt(helpEvent->pos());
    if (!m_action || (m_action->menu() && !m_action->menu()->isEmpty())) {
        // Do not show a tooltip when there is a menu since they will compete space-wise.
        QToolTip::hideText();
        return false;
    }

    // All actions have their text as a tooltip by default.
    // We only want to display the tooltip text if it isn't identical
    // to the already visible text in the menu.
    const bool explicitTooltip = !isTextSimilar(m_action->iconText(), m_action->toolTip());
    // We only want to show the whatsThisHint in a tooltip if the whatsThis isn't empty.
    const bool emptyWhatsThis = m_action->whatsThis().isEmpty();
    if (!explicitTooltip && emptyWhatsThis) {
        QToolTip::hideText();
        return false;
    }

    // Calculate a nice location for the tooltip so it doesn't unnecessarily cover
    // a part of the menu.
    const QRect actionGeometry = menu->actionGeometry(m_action);
    const int xOffset = menu->layoutDirection() == Qt::RightToLeft ? 0 : actionGeometry.width();
    const QPoint toolTipPosition(helpEvent->globalX() - helpEvent->x() + xOffset,
                                 helpEvent->globalY() - helpEvent->y() + actionGeometry.y() - actionGeometry.height() / 2);

    if (explicitTooltip) {
        if (emptyWhatsThis || isTextSimilar(m_action->whatsThis(), m_action->toolTip())) {
            if (m_action->toolTip() != whatsThisHintOnly()) {
                QToolTip::showText(toolTipPosition, m_action->toolTip(), m_widget, actionGeometry);
            }
        } else {
            showExpandableToolTip(toolTipPosition, m_action->toolTip(), actionGeometry);
        }
        return true;
    }
    Q_ASSERT(!m_action->whatsThis().isEmpty());
    showExpandableToolTip(toolTipPosition, QStringLiteral(), actionGeometry);
    return true;
}

bool KToolTipHelperPrivate::handleToolTipEvent(QObject *watched, QHelpEvent *helpEvent)
{
    if (auto watchedWidget = qobject_cast<QWidget *>(watched)) {
        m_widget = watchedWidget;
    } else {
        // There are fringe cases in which QHelpEvents are sent to QObjects that are not QWidgets
        // e.g. objects inheriting from QSystemTrayIcon.
        // We do not know how to handle those so we return false.
        return false;
    }

    m_lastToolTipWasExpandable = false;

    bool areToolTipAndWhatsThisSimilar = isTextSimilar(m_widget->whatsThis(), m_widget->toolTip());

    if (QToolButton *toolButton = qobject_cast<QToolButton *>(m_widget)) {
        if (const QAction *action = toolButton->defaultAction()) {
            if (!action->shortcut().isEmpty() && action->toolTip() != whatsThisHintOnly()) {
                // Because we set the tool button's tooltip below, we must re-check the whats this, because the shortcut
                // would technically make it unique.
                areToolTipAndWhatsThisSimilar = isTextSimilar(action->whatsThis(), action->toolTip());

                toolButton->setToolTip(i18nc("@info:tooltip %1 is the tooltip of an action, %2 is its keyboard shorcut",
                                             "%1 (%2)",
                                             action->toolTip(),
                                             action->shortcut().toString(QKeySequence::NativeText)));
                // Do not replace the brackets in the above i18n-call with <shortcut> tags from
                // KUIT because mixing KUIT with HTML is not allowed and %1 could be anything.

                // We don't show the tooltip here because aside from adding the keyboard shortcut
                // the QToolButton can now be handled like the tooltip event for any other widget.
            }
        }
    } else if (QMenu *menu = qobject_cast<QMenu *>(m_widget)) {
        return handleMenuToolTipEvent(menu, helpEvent);
    }

    while (m_widget->toolTip().isEmpty()) {
        m_widget = m_widget->parentWidget();
        if (!m_widget) {
            return false;
        }
    }

    if (m_widget->whatsThis().isEmpty() || areToolTipAndWhatsThisSimilar) {
        if (m_widget->toolTip() == whatsThisHintOnly()) {
            return true;
        }
        return false;
    }
    showExpandableToolTip(helpEvent->globalPos(), m_widget->toolTip());
    return true;
}

bool KToolTipHelperPrivate::handleWhatsThisClickedEvent(QEvent *event)
{
    event->accept();
    const auto whatsThisClickedEvent = static_cast<QWhatsThisClickedEvent *>(event);
    QDesktopServices::openUrl(QUrl(whatsThisClickedEvent->href()));
    return true;
}

void KToolTipHelperPrivate::postToolTipEventIfCursorDidntMove() const
{
    const QPoint globalCursorPos = QCursor::pos();
    if (globalCursorPos != m_cursorGlobalPosWhenLastMenuHid) {
        return;
    }

    const auto widgetUnderCursor = qApp->widgetAt(globalCursorPos);
    // We only want a behaviour change for QMenus.
    if (qobject_cast<QMenu *>(widgetUnderCursor)) {
        qGuiApp->postEvent(widgetUnderCursor, new QHelpEvent(QEvent::ToolTip, widgetUnderCursor->mapFromGlobal(globalCursorPos), globalCursorPos));
    }
}

void KToolTipHelperPrivate::showExpandableToolTip(const QPoint &globalPos, const QString &toolTip, const QRect &rect)
{
    m_lastExpandableToolTipGlobalPos = QPoint(globalPos);
    m_lastToolTipWasExpandable = true;
    const KColorScheme colorScheme = KColorScheme(QPalette::Normal, KColorScheme::Tooltip);
    const QColor hintTextColor = colorScheme.foreground(KColorScheme::InactiveText).color();

    if (toolTip.isEmpty() || toolTip == whatsThisHintOnly()) {
        const QString whatsThisHint =
            // i18n: Pressing Shift will show a longer message with contextual info
            // about the thing the tooltip was invoked for. If there is no good way to translate
            // the message, translating "Press Shift to learn more." would also mostly fit what
            // is supposed to be expressed here.
            i18nc("@info:tooltip", "<small><font color=\"%1\">Press <b>Shift</b> for more Info.</font></small>", hintTextColor.name());
        QToolTip::showText(m_lastExpandableToolTipGlobalPos, whatsThisHint, m_widget, rect);
    } else {
        const QString toolTipWithHint = QStringLiteral("<qt>") +
            // i18n: The 'Press Shift for more' message is added to tooltips that have an
            // available whatsthis help message. Pressing Shift will show this more exhaustive message.
            // It is particularly important to keep this translation short because:
            // 1. A longer translation will increase the size of *every* tooltip that gets this hint
            //      added e.g. a two word tooltip followed by a four word hint.
            // 2. The purpose of this hint is so we can keep the tooltip shorter than it would have to
            //      be if we couldn't refer to the message that appears when pressing Shift.
            //
            // %1 can be any tooltip. <br/> produces a linebreak. The other things between < and > are
            // styling information. The word "more" refers to "information".
            i18nc("@info:tooltip keep short", "%1<br/><small><font color=\"%2\">Press <b>Shift</b> for more.</font></small>", toolTip, hintTextColor.name())
            + QStringLiteral("</qt>");
        // Do not replace above HTML tags with KUIT because mixing HTML and KUIT is not allowed and
        // we can not know what kind of markup the tooltip in %1 contains.
        QToolTip::showText(m_lastExpandableToolTipGlobalPos, toolTipWithHint, m_widget, rect);
    }
}

KToolTipHelper *KToolTipHelperPrivate::s_instance = nullptr;

bool isTextSimilar(const QString &a, const QString &b)
{
    int i = -1;
    int j = -1;
    do {
        i++;
        j++;
        // Both of these QStrings are considered equal if their only differences are '&' and '.' chars.
        // Now move both of their indices to the next char that is neither '&' nor '.'.
        while (i < a.size() && (a.at(i) == QLatin1Char('&') || a.at(i) == QLatin1Char('.'))) {
            i++;
        }
        while (j < b.size() && (b.at(j) == QLatin1Char('&') || b.at(j) == QLatin1Char('.'))) {
            j++;
        }

        if (i >= a.size()) {
            return j >= b.size();
        }
        if (j >= b.size()) {
            return i >= a.size();
        }
    } while (a.at(i) == b.at(j));
    return false; // We have found a difference.
}