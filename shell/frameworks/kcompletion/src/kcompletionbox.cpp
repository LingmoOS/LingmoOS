/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2000, 2001, 2002 Carsten Pfeiffer <pfeiffer@kde.org>
    SPDX-FileCopyrightText: 2000 Stefan Schimanski <1Stein@gmx.de>
    SPDX-FileCopyrightText: 2000, 2001, 2002, 2003, 2004 Dawit Alemayehu <adawit@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcompletionbox.h"
#include "klineedit.h"

#include <QApplication>
#include <QKeyEvent>
#include <QScreen>
#include <QScrollBar>

class KCompletionBoxPrivate
{
public:
    QWidget *m_parent = nullptr; // necessary to set the focus back
    QString cancelText;
    bool tabHandling = true;
    bool upwardBox = false;
    bool emitSelected = true;
};

KCompletionBox::KCompletionBox(QWidget *parent)
    : QListWidget(parent)
    , d(new KCompletionBoxPrivate)
{
    d->m_parent = parent;

    // we can't link to QXcbWindowFunctions::Combo
    // also, q->setAttribute(Qt::WA_X11NetWmWindowTypeCombo); is broken in Qt xcb
    setProperty("_q_xcb_wm_window_type", 0x001000);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // on wayland, we need an xdg-popup but we don't want it to grab
    // calls setVisible, so must be done after initializations
    if (qGuiApp->platformName() == QLatin1String("wayland")) {
        setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    } else {
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::BypassWindowManagerHint);
    }
    setUniformItemSizes(true);

    setLineWidth(1);
    setFrameStyle(QFrame::Box | QFrame::Plain);

    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(this, &QListWidget::itemDoubleClicked, this, &KCompletionBox::slotActivated);
    connect(this, &KCompletionBox::itemClicked, this, [this](QListWidgetItem *item) {
        if (item) {
            hide();
            Q_EMIT currentTextChanged(item->text());
            Q_EMIT textActivated(item->text());
        }
    });
}

KCompletionBox::~KCompletionBox()
{
    d->m_parent = nullptr;
}

QStringList KCompletionBox::items() const
{
    QStringList list;
    list.reserve(count());
    for (int i = 0; i < count(); i++) {
        const QListWidgetItem *currItem = item(i);

        list.append(currItem->text());
    }

    return list;
}

void KCompletionBox::slotActivated(QListWidgetItem *item)
{
    if (item) {
        hide();
        Q_EMIT textActivated(item->text());
    }
}

bool KCompletionBox::eventFilter(QObject *o, QEvent *e)
{
    int type = e->type();
    QWidget *wid = qobject_cast<QWidget *>(o);

    if (o == this) {
        return false;
    }

    if (wid && wid == d->m_parent //
        && (type == QEvent::Move || type == QEvent::Resize)) {
        resizeAndReposition();
        return false;
    }

    if (wid && (wid->windowFlags() & Qt::Window) //
        && type == QEvent::Move && wid == d->m_parent->window()) {
        hide();
        return false;
    }

    if (type == QEvent::MouseButtonPress && (wid && !isAncestorOf(wid))) {
        if (!d->emitSelected && currentItem() && !qobject_cast<QScrollBar *>(o)) {
            Q_ASSERT(currentItem());
            Q_EMIT currentTextChanged(currentItem()->text());
        }
        hide();
        e->accept();
        return true;
    }

    if (wid && wid->isAncestorOf(d->m_parent) && isVisible()) {
        if (type == QEvent::KeyPress) {
            QKeyEvent *ev = static_cast<QKeyEvent *>(e);
            switch (ev->key()) {
            case Qt::Key_Backtab:
                if (d->tabHandling && (ev->modifiers() == Qt::NoButton || (ev->modifiers() & Qt::ShiftModifier))) {
                    up();
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Tab:
                if (d->tabHandling && (ev->modifiers() == Qt::NoButton)) {
                    down();
                    // #65877: Key_Tab should complete using the first
                    // (or selected) item, and then offer completions again
                    if (count() == 1) {
                        KLineEdit *parent = qobject_cast<KLineEdit *>(d->m_parent);
                        if (parent) {
                            parent->doCompletion(currentItem()->text());
                        } else {
                            hide();
                        }
                    }
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Down:
                down();
                ev->accept();
                return true;
            case Qt::Key_Up:
                // If there is no selected item and we've popped up above
                // our parent, select the first item when they press up.
                if (!selectedItems().isEmpty() //
                    || mapToGlobal(QPoint(0, 0)).y() > d->m_parent->mapToGlobal(QPoint(0, 0)).y()) {
                    up();
                } else {
                    down();
                }
                ev->accept();
                return true;
            case Qt::Key_PageUp:
                pageUp();
                ev->accept();
                return true;
            case Qt::Key_PageDown:
                pageDown();
                ev->accept();
                return true;
            case Qt::Key_Escape:
                if (!d->cancelText.isNull()) {
                    Q_EMIT userCancelled(d->cancelText);
                }
                if (isVisible()) {
                    hide();
                }
                ev->accept();
                return true;
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (ev->modifiers() & Qt::ShiftModifier) {
                    hide();
                    ev->accept(); // Consume the Enter event
                    return true;
                }
                break;
            case Qt::Key_End:
                if (ev->modifiers() & Qt::ControlModifier) {
                    end();
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Home:
                if (ev->modifiers() & Qt::ControlModifier) {
                    home();
                    ev->accept();
                    return true;
                }
                Q_FALLTHROUGH();
            default:
                break;
            }
        } else if (type == QEvent::ShortcutOverride) {
            // Override any accelerators that match
            // the key sequences we use here...
            QKeyEvent *ev = static_cast<QKeyEvent *>(e);
            switch (ev->key()) {
            case Qt::Key_Down:
            case Qt::Key_Up:
            case Qt::Key_PageUp:
            case Qt::Key_PageDown:
            case Qt::Key_Escape:
            case Qt::Key_Enter:
            case Qt::Key_Return:
                ev->accept();
                return true;
            case Qt::Key_Tab:
            case Qt::Key_Backtab:
                if (ev->modifiers() == Qt::NoButton || (ev->modifiers() & Qt::ShiftModifier)) {
                    ev->accept();
                    return true;
                }
                break;
            case Qt::Key_Home:
            case Qt::Key_End:
                if (ev->modifiers() & Qt::ControlModifier) {
                    ev->accept();
                    return true;
                }
                break;
            default:
                break;
            }
        } else if (type == QEvent::FocusOut) {
            QFocusEvent *event = static_cast<QFocusEvent *>(e);
            if (event->reason() != Qt::PopupFocusReason
#ifdef Q_OS_WIN
                && (event->reason() != Qt::ActiveWindowFocusReason || QApplication::activeWindow() != this)
#endif
            ) {
                hide();
            }
        }
    }

    return QListWidget::eventFilter(o, e);
}

void KCompletionBox::popup()
{
    if (count() == 0) {
        hide();
    } else {
        bool block = signalsBlocked();
        blockSignals(true);
        setCurrentRow(-1);
        blockSignals(block);
        clearSelection();
        if (!isVisible()) {
            show();
        } else if (size().height() != sizeHint().height()) {
            resizeAndReposition();
        }
    }
}

void KCompletionBox::resizeAndReposition()
{
    int currentGeom = height();
    QPoint currentPos = pos();
    QRect geom = calculateGeometry();
    resize(geom.size());

    int x = currentPos.x();
    int y = currentPos.y();
    if (d->m_parent) {
        if (!isVisible()) {
            const QPoint orig = globalPositionHint();
            QScreen *screen = QGuiApplication::screenAt(orig);
            if (screen) {
                const QRect screenSize = screen->geometry();

                x = orig.x() + geom.x();
                y = orig.y() + geom.y();

                if (x + width() > screenSize.right()) {
                    x = screenSize.right() - width();
                }
                if (y + height() > screenSize.bottom()) {
                    y = y - height() - d->m_parent->height();
                    d->upwardBox = true;
                }
            }
        } else {
            // Are we above our parent? If so we must keep bottom edge anchored.
            if (d->upwardBox) {
                y += (currentGeom - height());
            }
        }
        move(x, y);
    }
}

QPoint KCompletionBox::globalPositionHint() const
{
    if (!d->m_parent) {
        return QPoint();
    }
    return d->m_parent->mapToGlobal(QPoint(0, d->m_parent->height()));
}

void KCompletionBox::setVisible(bool visible)
{
    if (visible) {
        d->upwardBox = false;
        if (d->m_parent) {
            resizeAndReposition();
            qApp->installEventFilter(this);
        }

        // FIXME: Is this comment still valid or can it be deleted? Is a patch already sent to Qt?
        // Following lines are a workaround for a bug (not sure whose this is):
        // If this KCompletionBox' parent is in a layout, that layout will detect the
        // insertion of a new child (posting a ChildInserted event). Then it will trigger relayout
        // (posting a LayoutHint event).
        //
        // QWidget::show() then sends also posted ChildInserted events for the parent,
        // and later all LayoutHint events, which cause layout updating.
        // The problem is that KCompletionBox::eventFilter() detects the resizing
        // of the parent, calls hide() and this hide() happens in the middle
        // of show(), causing inconsistent state. I'll try to submit a Qt patch too.
        qApp->sendPostedEvents();
    } else {
        if (d->m_parent) {
            qApp->removeEventFilter(this);
        }
        d->cancelText.clear();
    }

    QListWidget::setVisible(visible);
}

QRect KCompletionBox::calculateGeometry() const
{
    QRect visualRect;
    if (count() == 0 || !(visualRect = visualItemRect(item(0))).isValid()) {
        return QRect();
    }

    int x = 0;
    int y = 0;
    int ih = visualRect.height();
    int h = qMin(15 * ih, count() * ih) + 2 * frameWidth();

    int w = (d->m_parent) ? d->m_parent->width() : QListWidget::minimumSizeHint().width();
    w = qMax(QListWidget::minimumSizeHint().width(), w);
    return QRect(x, y, w, h);
}

QSize KCompletionBox::sizeHint() const
{
    return calculateGeometry().size();
}

void KCompletionBox::down()
{
    const int row = currentRow();
    const int lastRow = count() - 1;
    if (row < lastRow) {
        setCurrentRow(row + 1);
        return;
    }

    if (lastRow > -1) {
        setCurrentRow(0);
    }
}

void KCompletionBox::up()
{
    const int row = currentRow();
    if (row > 0) {
        setCurrentRow(row - 1);
        return;
    }

    const int lastRow = count() - 1;
    if (lastRow > 0) {
        setCurrentRow(lastRow);
    }
}

void KCompletionBox::pageDown()
{
    selectionModel()->setCurrentIndex(moveCursor(QAbstractItemView::MovePageDown, Qt::NoModifier), QItemSelectionModel::SelectCurrent);
}

void KCompletionBox::pageUp()
{
    selectionModel()->setCurrentIndex(moveCursor(QAbstractItemView::MovePageUp, Qt::NoModifier), QItemSelectionModel::SelectCurrent);
}

void KCompletionBox::home()
{
    setCurrentRow(0);
}

void KCompletionBox::end()
{
    setCurrentRow(count() - 1);
}

void KCompletionBox::setTabHandling(bool enable)
{
    d->tabHandling = enable;
}

bool KCompletionBox::isTabHandling() const
{
    return d->tabHandling;
}

void KCompletionBox::setCancelledText(const QString &text)
{
    d->cancelText = text;
}

QString KCompletionBox::cancelledText() const
{
    return d->cancelText;
}

void KCompletionBox::insertItems(const QStringList &items, int index)
{
    bool block = signalsBlocked();
    blockSignals(true);
    QListWidget::insertItems(index, items);
    blockSignals(block);
    setCurrentRow(-1);
}

void KCompletionBox::setItems(const QStringList &items)
{
    bool block = signalsBlocked();
    blockSignals(true);

    int rowIndex = 0;

    if (!count()) {
        addItems(items);
    } else {
        for (const auto &text : items) {
            if (rowIndex < count()) {
                auto item = this->item(rowIndex);
                if (item->text() != text) {
                    item->setText(text);
                }
            } else {
                addItem(text);
            }
            rowIndex++;
        }

        // remove unused items with an index >= rowIndex
        for (; rowIndex < count();) {
            QListWidgetItem *item = takeItem(rowIndex);
            Q_ASSERT(item);
            delete item;
        }
    }

    if (isVisible() && size().height() != sizeHint().height()) {
        resizeAndReposition();
    }

    blockSignals(block);
}

void KCompletionBox::setActivateOnSelect(bool doEmit)
{
    d->emitSelected = doEmit;
}

bool KCompletionBox::activateOnSelect() const
{
    return d->emitSelected;
}

#include "moc_kcompletionbox.cpp"
