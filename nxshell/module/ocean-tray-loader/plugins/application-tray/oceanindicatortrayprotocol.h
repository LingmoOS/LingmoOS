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
class OCEANindicatorProtocolHandler;
class OCEANindicatorProtocol : public AbstractTrayProtocol
{
    Q_OBJECT

public:
    explicit OCEANindicatorProtocol(QObject *parent = nullptr);
    virtual ~OCEANindicatorProtocol();

private Q_SLOTS:
    void registedItemChanged();

private:
    QFileSystemWatcher* m_watcher;
    QHash<QString, QSharedPointer<OCEANindicatorProtocolHandler>> m_registedItem;
};

class OCEANindicatorProtocolHandlerPrivate;
class OCEANindicatorProtocolHandler : public AbstractTrayProtocolHandler
{
    Q_OBJECT

public:
    OCEANindicatorProtocolHandler(QString indicatorFilePath, QObject *parent= nullptr);
    ~OCEANindicatorProtocolHandler();

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
    QScopedPointer<OCEANindicatorProtocolHandlerPrivate> d_ptr;
    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), OCEANindicatorProtocolHandler)
};
}
