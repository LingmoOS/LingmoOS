// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QIcon>
#include <QWidget>
#include <QObject>
#include <cstddef>
#include <qwidget.h>

class TrayWidget;
namespace tray {
class AbstractTrayProtocolHandler;
class AbstractTrayProtocol : public QObject
{
    Q_OBJECT

public:
    explicit AbstractTrayProtocol(QObject *parent = nullptr) : QObject(parent) {};

Q_SIGNALS:
    void trayCreated(QPointer<AbstractTrayProtocolHandler> handler);
};

class AbstractTrayProtocolHandler : public QObject
{
    Q_OBJECT

    Q_PROPERTY(uint32_t windowId READ windowId CONSTANT FINAL)
    Q_PROPERTY(QString id READ id CONSTANT FINAL)

    Q_PROPERTY(QString title READ title NOTIFY titleChanged FINAL)
    Q_PROPERTY(QString status READ status NOTIFY statusChanged FINAL)

    Q_PROPERTY(QString category READ category NOTIFY categoryChanged FINAL)

    Q_PROPERTY(QIcon icon READ icon NOTIFY iconChanged FINAL)
    Q_PROPERTY(QIcon overlayIcon READ overlayIcon NOTIFY overlayIconChanged FINAL)
    Q_PROPERTY(QIcon attentionIcon READ attentionIcon NOTIFY attentionIconChanged FINAL)

    Q_PROPERTY(bool enabled READ enabled NOTIFY enabledChanged FINAL)

public:
    AbstractTrayProtocolHandler(QObject *parent = nullptr) {};
    ~AbstractTrayProtocolHandler() {};

    virtual uint32_t windowId() const = 0;

    virtual QString id() const = 0;
    
    virtual QString title() const = 0;
    virtual QString status() const = 0;

    virtual QString category() const = 0;

    virtual QIcon overlayIcon() const = 0;
    virtual QIcon attentionIcon() const = 0;
    virtual QIcon icon() const = 0;
    virtual QWidget *tooltip() const {return nullptr;}

    virtual bool enabled() const {return false;}

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) {return false;};

Q_SIGNALS:
    void titleChanged();
    void statusChanged();
    void categoryChanged();
    void tooltiChanged();

    void iconChanged();
    void overlayIconChanged();
    void attentionIconChanged();

    void enabledChanged();
};
}
