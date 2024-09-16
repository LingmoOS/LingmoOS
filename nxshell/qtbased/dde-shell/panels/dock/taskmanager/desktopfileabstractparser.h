// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "appitem.h"
#include "desktopfileparserfactory.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <utility>


namespace dock {
class AbstractItem;
class AbstractWindow;

class DesktopfileAbstractParser : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString desktopIcon READ desktopIcon NOTIFY desktopIcon)
    Q_PROPERTY(bool isDocked READ isDocked WRITE setDocked NOTIFY dockedChanged)
    Q_PROPERTY(QString genericName READ genericName NOTIFY genericNameChanged)
    Q_PROPERTY(QList<QPair<QString, QString>> actions READ actions NOTIFY actionsChanged)
    Q_PROPERTY(QString xDeepinVendor READ xDeepinVendor NOTIFY xDeepinVendorChanged)

public:
    DesktopfileAbstractParser(QString desktopid, QObject* parent = nullptr);
    ~DesktopfileAbstractParser();

    virtual QString id();
    virtual QString name();
    virtual QList<QPair<QString, QString>> actions();
    virtual QString genericName();
    virtual QString desktopIcon();
    virtual QString xDeepinVendor();
    
    virtual std::pair<bool, QString> isValied();

    virtual void launch();
    virtual void launchWithAction(const QString& action);
    virtual void requestQuit();
    virtual QString type();

    // State in dock
    virtual bool isDocked() final;
    virtual void setDocked(bool docked) final;

    virtual void addAppItem(QPointer<AppItem> item) final;
    virtual QPointer<AppItem> getAppItem() final;
    static QString identifyType();

private:
    friend class DesktopfileParserFactory<DesktopfileAbstractParser>;
    static QString identifyWindow(QPointer<AbstractWindow> window);

Q_SIGNALS:
    void nameChanged();
    void iconChanged();
    void actionsChanged();
    void genericNameChanged();
    void dockedChanged();
    void xDeepinVendorChanged();
    void valiedChanged();

protected:
    QList<QPointer<AppItem>> m_appitems;
    QString m_id;
};
}
