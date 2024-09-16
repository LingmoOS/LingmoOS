// SPDX-FileCopyrightText: 2017 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dexportedinterface.h"
#include "base/private/dobject_p.h"

#include <QHash>
#include <QPair>
#include <QVector>
#include <QVariant>
#include <QDBusConnection>
#include <QDBusVariant>
#include <QDBusContext>
#include <QDBusMessage>

DCORE_BEGIN_NAMESPACE
namespace DUtil {

class DExportedInterfacePrivate;
class DExportedInterfaceDBusInterface : public QObject, protected QDBusContext
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.ExportedInterface")

public:
    DExportedInterfaceDBusInterface(DExportedInterfacePrivate *priv);

public Q_SLOTS:
    QStringList list();
    QString help(const QString &action);
    QDBusVariant invoke(QString action, QString parameters);

private:
    DExportedInterfacePrivate *p;
};

class DExportedInterfacePrivate : public DObjectPrivate
{
public:
    DExportedInterfacePrivate(DExportedInterface *q);

private:
    QStringList actionHelp(QString action, int indent);

    QHash<QString, QPair<std::function<QVariant(QString)>, QString>> actions;
    QScopedPointer<DExportedInterfaceDBusInterface> dbusif;
    D_DECLARE_PUBLIC(DExportedInterface)

    friend class DExportedInterfaceDBusInterface;
};

DExportedInterface::DExportedInterface(QObject *parent)
    : QObject(parent),
      DObject(*new DExportedInterfacePrivate(this))
{
    D_D(DExportedInterface);
    QDBusConnection::sessionBus().registerObject("/", d->dbusif.data(), QDBusConnection::RegisterOption::ExportAllSlots);
}

DExportedInterface::~DExportedInterface()
{
    QDBusConnection::sessionBus().unregisterObject("/");
}

void DExportedInterface::registerAction(const QString &action, const QString &description, const std::function<QVariant (QString)> handler)
{
    D_D(DExportedInterface);
    d->actions[action] = {handler, description};
}

QVariant DExportedInterface::invoke(const QString &action, const QString &parameters) const
{
    D_DC(DExportedInterface);
    if (auto func = d->actions.value(action).first) {
        return func(parameters);
    }
    return QVariant();
}

DExportedInterfacePrivate::DExportedInterfacePrivate(DExportedInterface *q)
    : DObjectPrivate(q)
    , dbusif(new DExportedInterfaceDBusInterface(this))
{}

QStringList DExportedInterfacePrivate::actionHelp(QString action, int indent)
{
    QStringList ret;
    if (actions.contains(action)) {
        ret << QString(indent * 2, ' ') + QString("%1: %2").arg(action).arg(actions[action].second);
    }
    return ret;
}

DExportedInterfaceDBusInterface::DExportedInterfaceDBusInterface(DExportedInterfacePrivate *priv)
    : QObject(nullptr)
    , p(priv)
{}

QStringList DExportedInterfaceDBusInterface::list()
{
    return p->actions.keys();
}

QString DExportedInterfaceDBusInterface::help(const QString &action)
{
    if (action.length()) {
        return p->actionHelp(action, 0).join('\n');
    } else {
        QString ret = "Available actions:";
        QStringList actions = p->actions.keys();
        actions.sort();
        for (auto action : actions) {
            ret += QString("\n\n") + p->actionHelp(action, 1).join('\n');
        }
        return ret;
    }
}

QDBusVariant DExportedInterfaceDBusInterface::invoke(QString action, QString parameters)
{
    QDBusVariant ret;
    if (!p->actions.contains(action)) {
        sendErrorReply(QDBusError::ErrorType::InvalidArgs, QString("Action \"%1\" is not registered").arg(action));
    } else {
        ret.setVariant(p->q_func()->invoke(action, parameters));
    }
    return ret;
}

}
DCORE_END_NAMESPACE

#include "dexportedinterface.moc"
