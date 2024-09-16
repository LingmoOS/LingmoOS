// SPDX-FileCopyrightText: 0217 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef DNOTITLEBARWINDOWHELPER_WL_H
#define DNOTITLEBARWINDOWHELPER_WL_H

#include "global.h"

#include <QObject>
#include <QPainterPath>

QT_BEGIN_NAMESPACE
class QWindow;
QT_END_NAMESPACE

DPP_BEGIN_NAMESPACE

class DNativeSettings;
class DNoTitlebarWlWindowHelper : public QObject
{
    Q_OBJECT

public:
    explicit DNoTitlebarWlWindowHelper(QWindow *window);
    ~DNoTitlebarWlWindowHelper();

    static void setWindowProperty(QWindow *window, const char *name, const QVariant &value);
    static void requestByWindowProperty(QWindow *window, const char *name);
    static void popupSystemWindowMenu(quintptr wid);

private slots:
    // update properties
    Q_SLOT void updateEnableSystemMoveFromProperty();

private:
    static bool windowEvent(QWindow *w, QEvent *event);
    bool isEnableSystemMove();
    static void startMoveWindow(QWindow *window);

private:
    QWindow *m_window;
    bool m_windowMoving = false;

    // properties
    bool m_enableSystemMove = true;

public:
    static QHash<const QWindow*, DNoTitlebarWlWindowHelper*> mapped;
};

DPP_END_NAMESPACE

Q_DECLARE_METATYPE(QPainterPath)

#endif // DNOTITLEBARWINDOWHELPER_WL_H
