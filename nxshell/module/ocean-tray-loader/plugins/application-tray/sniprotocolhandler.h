// SPDX-FileCopyrightText: 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "abstracttrayprotocol.h"
#include "statusnotifieriteminterface.h"
#include "statusnotifierwatcherinterface.h"
#include "api/types/dbusimagelist.h"

#include <QMenu>
#include <QPixmap>
#include <DLabel>
#include <qlabel.h>

class QTimer;
class DBusMenuImporter;

namespace tray {
class SniTrayProtocolHandler;
class SniTrayProtocol : public AbstractTrayProtocol
{
    Q_OBJECT

public:
    explicit SniTrayProtocol(QObject *parent = nullptr);
    ~SniTrayProtocol();

private Q_SLOTS:
    void registedItemChanged();

private:
    OrgKdeStatusNotifierWatcherInterface* m_trayManager;

    QHash<QString, QSharedPointer<SniTrayProtocolHandler>> m_registedItem;
};

class SniTrayProtocolHandler : public AbstractTrayProtocolHandler
{
    Q_OBJECT

public:
    SniTrayProtocolHandler(const QString &sniServicePath, QObject *parent= nullptr);
    ~SniTrayProtocolHandler();

    virtual uint32_t windowId() const override;

    virtual QString id() const override;
    
    virtual QString title() const override;
    virtual QString status() const override;

    virtual QWidget* tooltip() const override;
    virtual QString category() const override;

    virtual QIcon icon() const override;
    virtual QIcon overlayIcon() const override;
    virtual QIcon attentionIcon() const override;


    virtual bool enabled() const override;

    static QPair<QString, QString> serviceAndPath(const QString &servicePath);
    static QIcon dbusImageList2QIcon(const DBusImageList &servicePath);

protected:
    virtual bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void generateId();

private:
    QString m_dbusUniqueName;
    QLabel *m_tooltip;
    StatusNotifierItem *m_sniInter;
    DBusMenuImporter *m_dbusMenuImporter;
    QString m_id;
    bool m_ignoreFirstAttention;
};
}
