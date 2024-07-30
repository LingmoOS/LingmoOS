/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "partmanager.h"

#include "guiactivateevent.h"
#include "kparts_logging.h"
#include "part.h"
#include "partactivateevent.h"

#include <QApplication>
#include <QMouseEvent>
#include <QScrollBar>

using namespace KParts;

namespace KParts
{
class PartManagerPrivate
{
public:
    PartManagerPrivate()
    {
        m_activeWidget = nullptr;
        m_activePart = nullptr;
        m_bAllowNestedParts = false;
        m_bIgnoreScrollBars = false;
        m_activationButtonMask = Qt::LeftButton | Qt::MiddleButton | Qt::RightButton;
        m_reason = PartManager::NoReason;
        m_bIgnoreExplicitFocusRequest = false;
    }
    ~PartManagerPrivate()
    {
    }
    void setReason(QEvent *ev)
    {
        switch (ev->type()) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonDblClick: {
            // clang-format off
            QMouseEvent *mev = static_cast<QMouseEvent *>(ev);
            m_reason = mev->button() == Qt::LeftButton
                       ? PartManager::ReasonLeftClick
                       : (mev->button() == Qt::MiddleButton
                          ? PartManager::ReasonMidClick
                          : PartManager::ReasonRightClick);
            // clang-format on
            break;
        }
        case QEvent::FocusIn:
            m_reason = static_cast<QFocusEvent *>(ev)->reason();
            break;
        default:
            qCWarning(KPARTSLOG) << "PartManagerPrivate::setReason got unexpected event type" << ev->type();
            break;
        }
    }

    bool allowExplicitFocusEvent(QEvent *ev) const
    {
        if (ev->type() == QEvent::FocusIn) {
            QFocusEvent *fev = static_cast<QFocusEvent *>(ev);
            return (!m_bIgnoreExplicitFocusRequest || fev->reason() != Qt::OtherFocusReason);
        }
        return true;
    }

    Part *m_activePart;
    QWidget *m_activeWidget;

    QList<Part *> m_parts;

    PartManager::SelectionPolicy m_policy;

    QList<const QWidget *> m_managedTopLevelWidgets;
    short int m_activationButtonMask;
    bool m_bIgnoreScrollBars;
    bool m_bAllowNestedParts;
    int m_reason;
    bool m_bIgnoreExplicitFocusRequest;
};

}

PartManager::PartManager(QWidget *parent)
    : QObject(parent)
    , d(new PartManagerPrivate)
{
    qApp->installEventFilter(this);

    d->m_policy = Direct;

    addManagedTopLevelWidget(parent);
}

PartManager::PartManager(QWidget *topLevel, QObject *parent)
    : QObject(parent)
    , d(new PartManagerPrivate)
{
    qApp->installEventFilter(this);

    d->m_policy = Direct;

    addManagedTopLevelWidget(topLevel);
}

PartManager::~PartManager()
{
    for (const QWidget *w : std::as_const(d->m_managedTopLevelWidgets)) {
        disconnect(w, &QWidget::destroyed, this, &PartManager::slotManagedTopLevelWidgetDestroyed);
    }

    for (Part *it : std::as_const(d->m_parts)) {
        it->setManager(nullptr);
    }

    // core dumps ... setActivePart( 0 );
    qApp->removeEventFilter(this);
}

void PartManager::setSelectionPolicy(SelectionPolicy policy)
{
    d->m_policy = policy;
}

PartManager::SelectionPolicy PartManager::selectionPolicy() const
{
    return d->m_policy;
}

void PartManager::setAllowNestedParts(bool allow)
{
    d->m_bAllowNestedParts = allow;
}

bool PartManager::allowNestedParts() const
{
    return d->m_bAllowNestedParts;
}

void PartManager::setIgnoreScrollBars(bool ignore)
{
    d->m_bIgnoreScrollBars = ignore;
}

bool PartManager::ignoreScrollBars() const
{
    return d->m_bIgnoreScrollBars;
}

void PartManager::setActivationButtonMask(short int buttonMask)
{
    d->m_activationButtonMask = buttonMask;
}

short int PartManager::activationButtonMask() const
{
    return d->m_activationButtonMask;
}

bool PartManager::eventFilter(QObject *obj, QEvent *ev)
{
    if (ev->type() != QEvent::MouseButtonPress && ev->type() != QEvent::MouseButtonDblClick && ev->type() != QEvent::FocusIn) {
        return false;
    }

    if (!obj->isWidgetType()) {
        return false;
    }

    QWidget *w = static_cast<QWidget *>(obj);

    if (((w->windowFlags().testFlag(Qt::Dialog)) && w->isModal()) || (w->windowFlags().testFlag(Qt::Popup)) || (w->windowFlags().testFlag(Qt::Tool))) {
        return false;
    }

    QMouseEvent *mev = nullptr;
    if (ev->type() == QEvent::MouseButtonPress || ev->type() == QEvent::MouseButtonDblClick) {
        mev = static_cast<QMouseEvent *>(ev);

        qCDebug(KPARTSLOG) << "PartManager::eventFilter button:" << mev->button() << "d->m_activationButtonMask=" << d->m_activationButtonMask;

        if ((mev->button() & d->m_activationButtonMask) == 0) {
            return false; // ignore this button
        }
    }

    Part *part;
    while (w) {
        QPoint pos;

        if (!d->m_managedTopLevelWidgets.contains(w->topLevelWidget())) {
            return false;
        }

        if (d->m_bIgnoreScrollBars && ::qobject_cast<QScrollBar *>(w)) {
            return false;
        }

        if (mev) { // mouse press or mouse double-click event
            pos = mev->globalPosition().toPoint();
            part = findPartFromWidget(w, pos);
        } else {
            part = findPartFromWidget(w);
        }

        // clang-format off
        const char *evType = (ev->type() == QEvent::MouseButtonPress) ? "MouseButtonPress"
                             : (ev->type() == QEvent::MouseButtonDblClick) ? "MouseButtonDblClick"
                             : (ev->type() == QEvent::FocusIn) ? "FocusIn" : "OTHER! ERROR!";
        // clang-format on
        if (part) { // We found a part whose widget is w
            if (d->m_policy == PartManager::TriState) {
                if (ev->type() == QEvent::MouseButtonDblClick) {
                    if (part == d->m_activePart && w == d->m_activeWidget) {
                        return false;
                    }

                    qCDebug(KPARTSLOG) << "PartManager::eventFilter dblclick -> setActivePart" << part;

                    d->setReason(ev);
                    setActivePart(part, w);
                    d->m_reason = NoReason;
                    return true;
                }

                if ((d->m_activeWidget != w || d->m_activePart != part)) {
                    qCDebug(KPARTSLOG) << "Part" << part << "(non-selectable) made active because" << w->metaObject()->className() << "got event" << evType;

                    d->setReason(ev);
                    setActivePart(part, w);
                    d->m_reason = NoReason;
                    return true;
                } else if (d->m_activeWidget == w && d->m_activePart == part) {
                    return false;
                }

                return false;
            } else if (part != d->m_activePart && d->allowExplicitFocusEvent(ev)) {
                qCDebug(KPARTSLOG) << "Part" << part << "made active because" << w->metaObject()->className() << "got event" << evType;

                d->setReason(ev);
                setActivePart(part, w);
                d->m_reason = NoReason;
            }

            return false;
        }

        w = w->parentWidget();

        if (w && (((w->windowFlags() & Qt::Dialog) && w->isModal()) || (w->windowFlags() & Qt::Popup) || (w->windowFlags() & Qt::Tool))) {
            qCDebug(KPARTSLOG) << "No part made active although" << obj->objectName() << "/" << obj->metaObject()->className() << "got event - loop aborted";

            return false;
        }
    }

    qCDebug(KPARTSLOG) << "No part made active although" << obj->objectName() << "/" << obj->metaObject()->className() << "got event - loop aborted";

    return false;
}

Part *PartManager::findPartFromWidget(QWidget *widget, const QPoint &pos)
{
    for (auto *p : std::as_const(d->m_parts)) {
        Part *part = p->hitTest(widget, pos);
        if (part && d->m_parts.contains(part)) {
            return part;
        }
    }
    return nullptr;
}

Part *PartManager::findPartFromWidget(QWidget *widget)
{
    for (auto *part : std::as_const(d->m_parts)) {
        if (widget == part->widget()) {
            return part;
        }
    }
    return nullptr;
}

void PartManager::addPart(Part *part, bool setActive)
{
    Q_ASSERT(part);

    // don't add parts more than once :)
    if (d->m_parts.contains(part)) {
        qCWarning(KPARTSLOG) << part << " already added";
        return;
    }

    d->m_parts.append(part);

    part->setManager(this);

    if (setActive) {
        setActivePart(part);

        if (QWidget *w = part->widget()) {
            // Prevent focus problems
            if (w->focusPolicy() == Qt::NoFocus) {
                qCWarning(KPARTSLOG) << "Part '" << part->objectName() << "' has a widget " << w->objectName()
                                     << "with a focus policy of NoFocus. It should have at least a"
                                     << "ClickFocus policy, for part activation to work well.";
            }
            if (part->widget() && part->widget()->focusPolicy() == Qt::TabFocus) {
                qCWarning(KPARTSLOG) << "Part '" << part->objectName() << "' has a widget " << w->objectName()
                                     << "with a focus policy of TabFocus. It should have at least a"
                                     << "ClickFocus policy, for part activation to work well.";
            }
            w->setFocus();
            w->show();
        }
    }
    Q_EMIT partAdded(part);
}

void PartManager::removePart(Part *part)
{
    if (!d->m_parts.contains(part)) {
        return;
    }

    const int nb = d->m_parts.removeAll(part);
    Q_ASSERT(nb == 1);
    Q_UNUSED(nb); // no warning in release mode
    part->setManager(nullptr);

    Q_EMIT partRemoved(part);

    if (part == d->m_activePart) {
        setActivePart(nullptr);
    }
}

void PartManager::replacePart(Part *oldPart, Part *newPart, bool setActive)
{
    // qCDebug(KPARTSLOG) << "replacePart" << oldPart->name() << "->" << newPart->name() << "setActive=" << setActive;
    // This methods does exactly removePart + addPart but without calling setActivePart(0) in between
    if (!d->m_parts.contains(oldPart)) {
        qFatal("Can't remove part %s, not in KPartManager's list.", oldPart->objectName().toLocal8Bit().constData());
        return;
    }

    d->m_parts.removeAll(oldPart);
    oldPart->setManager(nullptr);

    Q_EMIT partRemoved(oldPart);

    addPart(newPart, setActive);
}

void PartManager::setActivePart(Part *part, QWidget *widget)
{
    if (part && !d->m_parts.contains(part)) {
        qCWarning(KPARTSLOG) << "trying to activate a non-registered part!" << part->objectName();
        return; // don't allow someone call setActivePart with a part we don't know about
    }

    // check whether nested parts are disallowed and activate the top parent part then, by traversing the
    // tree recursively (Simon)
    if (part && !d->m_bAllowNestedParts) {
        QObject *parentPart = part->parent(); // ### this relies on people using KParts::Factory!
        KParts::Part *parPart = ::qobject_cast<KParts::Part *>(parentPart);
        if (parPart) {
            setActivePart(parPart, parPart->widget());
            return;
        }
    }

    qCDebug(KPARTSLOG) << "PartManager::setActivePart d->m_activePart=" << d->m_activePart << "<->part=" << part << "d->m_activeWidget=" << d->m_activeWidget
                       << "<->widget=" << widget;

    // don't activate twice
    if (d->m_activePart && part && d->m_activePart == part && (!widget || d->m_activeWidget == widget)) {
        return;
    }

    KParts::Part *oldActivePart = d->m_activePart;
    QWidget *oldActiveWidget = d->m_activeWidget;

    d->m_activePart = part;
    d->m_activeWidget = widget;

    if (oldActivePart) {
        KParts::Part *savedActivePart = part;
        QWidget *savedActiveWidget = widget;

        PartActivateEvent ev(false, oldActivePart, oldActiveWidget);
        QApplication::sendEvent(oldActivePart, &ev);
        if (oldActiveWidget) {
            disconnect(oldActiveWidget, &QWidget::destroyed, this, &PartManager::slotWidgetDestroyed);
            QApplication::sendEvent(oldActiveWidget, &ev);
        }

        d->m_activePart = savedActivePart;
        d->m_activeWidget = savedActiveWidget;
    }

    if (d->m_activePart) {
        if (!widget) {
            d->m_activeWidget = part->widget();
        }

        PartActivateEvent ev(true, d->m_activePart, d->m_activeWidget);
        QApplication::sendEvent(d->m_activePart, &ev);
        if (d->m_activeWidget) {
            connect(d->m_activeWidget, &QWidget::destroyed, this, &PartManager::slotWidgetDestroyed);
            QApplication::sendEvent(d->m_activeWidget, &ev);
        }
    }
    // Set the new active instance
    // setActiveComponent(d->m_activePart ? d->m_activePart->componentData() : KComponentData::mainComponent());

    qCDebug(KPARTSLOG) << this << "emitting activePartChanged" << d->m_activePart;

    Q_EMIT activePartChanged(d->m_activePart);
}

Part *PartManager::activePart() const
{
    return d->m_activePart;
}

QWidget *PartManager::activeWidget() const
{
    return d->m_activeWidget;
}

void PartManager::slotObjectDestroyed()
{
    // qDebug();
    removePart(const_cast<Part *>(static_cast<const Part *>(sender())));
}

void PartManager::slotWidgetDestroyed()
{
    // qDebug();
    if (static_cast<const QWidget *>(sender()) == d->m_activeWidget) {
        setActivePart(nullptr); // do not remove the part because if the part's widget dies, then the
    }
    // part will delete itself anyway, invoking removePart() in its destructor
}

const QList<Part *> PartManager::parts() const
{
    return d->m_parts;
}

void PartManager::addManagedTopLevelWidget(const QWidget *topLevel)
{
    if (!topLevel->isWindow()) {
        return;
    }

    if (d->m_managedTopLevelWidgets.contains(topLevel)) {
        return;
    }

    d->m_managedTopLevelWidgets.append(topLevel);
    connect(topLevel, &QWidget::destroyed, this, &PartManager::slotManagedTopLevelWidgetDestroyed);
}

void PartManager::removeManagedTopLevelWidget(const QWidget *topLevel)
{
    d->m_managedTopLevelWidgets.removeAll(topLevel);
}

void PartManager::slotManagedTopLevelWidgetDestroyed()
{
    const QWidget *widget = static_cast<const QWidget *>(sender());
    removeManagedTopLevelWidget(widget);
}

int PartManager::reason() const
{
    return d->m_reason;
}

void PartManager::setIgnoreExplictFocusRequests(bool ignore)
{
    d->m_bIgnoreExplicitFocusRequest = ignore;
}

#include "moc_partmanager.cpp"
