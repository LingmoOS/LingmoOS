// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstracttrayprotocol.h"

#include <QMenu>
#include <QPixmap>
#include <QObject>
#include <QDBusMessage>

#include <QFileSystemWatcher>

class QTimer;
class DBusMenuImporter;
class StatusNotifierItem;

namespace tray {
class DDEindicatorProtocolHandler;
class DDEindicatorProtocol : public AbstractTrayProtocol
{
    Q_OBJECT

public:
    explicit DDEindicatorProtocol(QObject *parent = nullptr);
    virtual ~DDEindicatorProtocol();

private Q_SLOTS:
    void registedItemChanged();

private:
    QFileSystemWatcher* m_watcher;
    QHash<QString, QSharedPointer<DDEindicatorProtocolHandler>> m_registedItem;
};

class DDEindicatorProtocolHandlerPrivate;
class DDEindicatorProtocolHandler : public AbstractTrayProtocolHandler
{
    Q_OBJECT

public:
    DDEindicatorProtocolHandler(QString indicatorFilePath, QObject *parent= nullptr);
    ~DDEindicatorProtocolHandler();

    virtual uint32_t windowId() const override;

    virtual QString id() const override;
    
    virtual QString title() const override;
    virtual QString status() const override;

    virtual QString category() const override;

    virtual QIcon overlayIcon() const override;
    virtual QIcon attentionIcon() const override;
    virtual QIcon icon() const override;

    virtual bool enabled() const override;

private:
    inline void setEnabled(bool enabled);

private Q_SLOTS:
    void textPropertyChanged(const QDBusMessage &message);
    void iconPropertyChanged(const QDBusMessage &message);

Q_SIGNALS:
    void clicked(uint8_t button_index, int x, int y);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    QScopedPointer<DDEindicatorProtocolHandlerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DDEindicatorProtocolHandler)
};
}
