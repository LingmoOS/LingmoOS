/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#pragma once
#include <ibus.h>
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>

#include "panel.h"
#include <QAbstractNativeEventFilter>
#include <QByteArray>
#include <QGuiApplication>
#include <QMap>
#include <QPair>
#include <memory>
class QDBusServiceWatcher;

class XcbEventFilter : public QAbstractNativeEventFilter
{
public:
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;
#else
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;
#endif
};

class App : public QGuiApplication
{
    Q_OBJECT
public:
    typedef QPair<uint, uint> TriggerKey;
    App(int &argc, char *argv[]);
    ~App() override;
    void setTriggerKeys(QList<TriggerKey> triggersList);
    void setDoGrab(bool doGrab);
    bool keyboardGrabbed()
    {
        return m_keyboardGrabbed;
    }
    bool nativeEvent(xcb_generic_event_t *event);
    void nameAcquired();
    void nameLost();
    QByteArray normalizeIconName(const QByteArray &icon) const;
public Q_SLOTS:
    void init();
    void finalize();
    void clean();
    void grabKey();
    void ungrabKey();
    uint getPrimaryModifier(uint state);
    void keyRelease(const xcb_key_release_event_t *event);
    void accept();
    void ungrabXKeyboard();
    bool grabXKeyboard();

private:
    std::unique_ptr<XcbEventFilter> m_eventFilter;
    bool m_init;
    IBusBus *m_bus;
    IBusPanelImpanel *m_impanel;
    QList<QPair<uint, uint>> m_triggersList;
    bool m_keyboardGrabbed;
    bool m_doGrab;
    xcb_key_symbols_t *m_syms;
    QMap<QByteArray, QByteArray> m_iconMap;
    QDBusServiceWatcher *m_watcher;
};
