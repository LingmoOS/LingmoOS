// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstracttrayprotocol.h"
#include "traymanager1interface.h"

#include <cstdint>

#include <qobjectdefs.h>
#include <qpoint.h>
#include <qtimer.h>
#include <xcb/xproto.h>
#include <xcb/composite.h>
#include <xcb/xcb_image.h>

namespace tray {
class XembedProtocol : public AbstractTrayProtocol
{
    Q_OBJECT

public:
    XembedProtocol(QObject *parent = nullptr);
    ~XembedProtocol();

private Q_SLOTS:
    void onTrayIconsChanged();

private:
    TrayManager* m_trayManager;
    QHash<uint32_t, QSharedPointer<AbstractTrayProtocolHandler>> m_registedItem;
};

class XembedProtocolHandler : public AbstractTrayProtocolHandler
{
    Q_OBJECT

public:
    XembedProtocolHandler(const uint32_t& id, QObject *parent = nullptr);
    ~XembedProtocolHandler();

    virtual uint32_t windowId() const override;

    virtual QString id() const override;
    
    virtual QString title() const override;
    virtual QString status() const override;

    virtual QString category() const override;

    virtual QIcon overlayIcon() const override;
    virtual QIcon attentionIcon() const override;
    virtual QIcon icon() const override;

    virtual bool enabled() const override;

public Q_SLOTS:
    void xembedTrayIconChanged(uint32_t windowId);

private:
    void generateId();
    QPixmap getPixmapFromWidnow();
    void initX11resources();

    void sendHover();
    void sendClick(uint8_t mouseButton, const int& x, const int& y);

    QPoint calculateClickPoint() const;
    QSize calculateClientWindowSize() const;

    inline QPoint getGlobalPos();
    inline void updateEmbedWindowPosForGetInputEvent();

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    enum InjectMode {
        Direct,
        XTest,
    };

    bool m_enabled;
    uint32_t m_windowId;
    xcb_window_t m_containerWid;
    QPixmap m_icon;
    QPixmap m_attentionIcon;
    InjectMode m_injectMode;
    QString m_id;

    QTimer *m_hoverTimer;
    QTimer *m_attentionTimer;
    QTimer *m_iconUpdateTimer;
};
}

