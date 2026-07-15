#pragma once

#include <QDBusConnection>
#include <QDBusVariant>
#include <QHash>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QVariantMap>

class QAction;
class QDBusArgument;
class QMenu;

struct DBusMenuItem {
    int id = 0;
    QVariantMap properties;
};
Q_DECLARE_METATYPE(DBusMenuItem)
using DBusMenuItemList = QList<DBusMenuItem>;
Q_DECLARE_METATYPE(DBusMenuItemList)

struct DBusMenuItemKeys {
    int id = 0;
    QStringList properties;
};
Q_DECLARE_METATYPE(DBusMenuItemKeys)
using DBusMenuItemKeysList = QList<DBusMenuItemKeys>;
Q_DECLARE_METATYPE(DBusMenuItemKeysList)

struct DBusMenuLayoutItem {
    int id = 0;
    QVariantMap properties;
    QList<DBusMenuLayoutItem> children;
};
Q_DECLARE_METATYPE(DBusMenuLayoutItem)
using DBusMenuLayoutItemList = QList<DBusMenuLayoutItem>;
Q_DECLARE_METATYPE(DBusMenuLayoutItemList)

using DBusMenuShortcut = QList<QStringList>;
Q_DECLARE_METATYPE(DBusMenuShortcut)

QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItem &item);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItem &item);
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItemKeys &item);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItemKeys &item);
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuLayoutItem &item);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuLayoutItem &item);
QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuShortcut &shortcut);
const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuShortcut &shortcut);

/**
 * Exposes a QMenu over the com.canonical.dbusmenu protocol.
 *
 * Qt 6 no longer ships the Qt 5 DBusMenu exporter that the old platform theme
 * used. Keeping the small exporter here avoids a Qt 5 ABI dependency while
 * preserving StatusNotifierItem context menus for Plasma and other hosts.
 */
class DBusMenuExporter : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.canonical.dbusmenu")
    Q_PROPERTY(uint Version READ version CONSTANT)
    Q_PROPERTY(QString Status READ status CONSTANT)

public:
    DBusMenuExporter(const QString &path, QMenu *menu, const QDBusConnection &connection);
    ~DBusMenuExporter() override;

    uint version() const;
    QString status() const;

public Q_SLOTS:
    void Event(int id, const QString &eventId, const QDBusVariant &data, uint timestamp);
    QDBusVariant GetProperty(int id, const QString &property);
    void GetLayout(int parentId, int recursionDepth, const QStringList &propertyNames,
                   uint &revision, DBusMenuLayoutItem &item);
    DBusMenuItemList GetGroupProperties(const QList<int> &ids,
                                        const QStringList &propertyNames);
    bool AboutToShow(int id);

Q_SIGNALS:
    void ItemsPropertiesUpdated(const DBusMenuItemList &updated,
                                const DBusMenuItemKeysList &removed);
    void LayoutUpdated(uint revision, int parentId);
    void ItemActivationRequested(int id, uint timestamp);

private:
    int idForAction(QAction *action);
    QAction *actionForId(int id);
    QMenu *menuForId(int id);
    void observeMenu(QMenu *menu);
    void synchronizeActions();
    void synchronizeMenu(QMenu *menu);
    QVariantMap propertiesForAction(const QAction *action,
                                    const QStringList &propertyNames);
    DBusMenuLayoutItem layoutFor(int id, int recursionDepth,
                                 const QStringList &propertyNames);
    void emitLayoutUpdated(int parentId = 0);
    void emitPropertiesUpdated(QAction *action);

    QString mPath;
    QMenu *mMenu = nullptr;
    QDBusConnection mConnection;
    QHash<QAction *, int> mActionIds;
    QHash<int, QAction *> mActions;
    QHash<QMenu *, bool> mObservedMenus;
    QHash<QAction *, bool> mObservedActions;
    int mNextId = 1;
    uint mRevision = 1;
};
