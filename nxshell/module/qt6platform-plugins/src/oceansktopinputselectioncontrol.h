// SPDX-FileCopyrightText: 2020 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DESKTOPINPUTSELECTIONCONTROL_H
#define DESKTOPINPUTSELECTIONCONTROL_H

#include "global.h"

#include <QMap>
#include <QSize>
#include <QPoint>
#include <QObject>
#include <QVector>
#include <QPointer>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QMouseEvent;
class QInputMethod;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DInputSelectionHandle;
class DApplicationEventMonitor;
class DSelectedTextTooltip;
class DDesktopInputSelectionControl : public QObject
{
    Q_OBJECT

public:
    enum HandleState {
        HandleIsReleased = 0,
        HandleIsHeld = 1,
        HandleIsMoving = 2
    };

    enum HandleType {
        AnchorHandle = 0,
        CursorHandle = 1
    };

    DDesktopInputSelectionControl(QObject *parent, QInputMethod *inputMethod);
    ~DDesktopInputSelectionControl() override;

    void createHandles();
    void setApplicationEventMonitor(DApplicationEventMonitor *pMonitor);

public Q_SLOTS:
    void updateAnchorHandlePosition();
    void updateCursorHandlePosition();
    void updateTooltipPosition();
    void onWindowStateChanged(Qt::WindowState state);
    void updateSelectionControlVisible();
    void onOptAction(int type);
    void onFocusWindowChanged();

signals:
    void anchorPositionChanged();
    void cursorPositionChanged();
    void anchorRectangleChanged();
    void cursorRectangleChanged();
    void selectionControlVisibleChanged();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;

private:
    void setHandleState(HandleState state);
    int anchorPosition() const;
    int cursorPosition() const;
    Qt::InputMethodHints inputMethodHints() const;
    QRectF anchorRectangle() const;
    QRectF cursorRectangle() const;
    QRectF keyboardRectangle() const;
    bool anchorRectIntersectsClipRect() const;
    bool cursorRectIntersectsClipRect() const;
    void updateHandleFlags();

    void setSelectionOnFocusObject(const QPointF &anchorPos, const QPointF &cursorPos);

    QRect anchorHandleRect() const;
    QRect cursorHandleRect() const;
    QRect handleRectForCursorRect(const QRectF &cursorRect) const;
    QRect handleRectForAnchorRect(const QRectF &anchorRect) const;

    bool testHandleVisible() const;

private:
    QInputMethod *m_pInputMethod;
    QScopedPointer<DInputSelectionHandle> m_anchorSelectionHandle;
    QScopedPointer<DInputSelectionHandle> m_cursorSelectionHandle;
    QScopedPointer<DSelectedTextTooltip> m_selectedTextTooltip;
    QPointer<DApplicationEventMonitor> m_pApplicationEventMonitor;
    QSize m_handleImageSize;

    HandleState m_handleState;
    HandleType m_currentDragHandle;

    bool m_eventFilterEnabled;
    bool m_anchorHandleVisible;
    bool m_cursorHandleVisible;
    bool m_handleVisible;
    QPoint m_otherSelectionPoint;
    QVector<QMouseEvent *> m_eventQueue;
    QPoint m_distanceBetweenMouseAndCursor;
    QPoint m_handleDragStartedPosition;
    QSize m_fingerOptSize;

    QMap<QObject*, QPointF> m_focusWindow;
};

DPP_END_NAMESPACE

#endif // DESKTOPINPUTSELECTIONCONTROL_H
