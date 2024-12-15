// SPDX-FileCopyrightText: 2017 - 2022 Uniontech Software Technology Co.,Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef TESTWINDOW_H
#define TESTWINDOW_H

#include <QtGlobal>

#ifdef Q_OS_LINUX
#define private public
#include "qxcbwindow.h"
#include "qxcbclipboard.h"
typedef QXcbWindow QNativeWindow;
#undef private
#elif defined(Q_OS_WIN)
#include "qwindowswindow.h"
typedef QWindowsWindow QNativeWindow;
#endif

#include "global.h"

DPP_BEGIN_NAMESPACE

class DPlatformWindowHook : public QObject
{
    Q_OBJECT

public:
    DPlatformWindowHook(QNativeWindow *window);

    ~DPlatformWindowHook();

    QNativeWindow *window() const
    { return static_cast<QNativeWindow*>(reinterpret_cast<QPlatformWindow*>(const_cast<DPlatformWindowHook*>(this)));}

    DPlatformWindowHook *me() const;

    void setGeometry(const QRect &rect);
    QRect geometry() const;

    QMargins frameMargins() const;

    void setParent(const QPlatformWindow *window);

    void setWindowTitle(const QString &title);
    void setWindowIcon(const QIcon &icon);

    QPoint mapToGlobal(const QPoint &pos) const;
    QPoint mapFromGlobal(const QPoint &pos) const;

    void setMask(const QRegion &region);
#ifdef Q_OS_LINUX
    void setWindowState(Qt::WindowState state);
    void setVisible(bool visible);
    void requestActivateWindow();
#endif
//    bool startSystemResize(const QPoint &pos, Qt::Corner corner);

    void propagateSizeHints();

    static DPlatformWindowHook *getHookByWindow(const QPlatformWindow *window);

signals:
    void windowGeometryAboutChanged(const QRect &rect);

private:
    void setWindowMargins(const QMargins &margins, bool propagateSizeHints = false);

    QMargins windowMargins;
    static QHash<const QPlatformWindow*, DPlatformWindowHook*> mapped;

    QNativeWindow *nativeWindow;

    friend class DPlatformBackingStore;
};

DPP_END_NAMESPACE

Q_DECLARE_METATYPE(QPainterPath)

#endif // TESTWINDOW_H
