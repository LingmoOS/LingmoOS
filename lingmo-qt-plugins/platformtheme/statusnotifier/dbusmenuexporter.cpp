#include "dbusmenuexporter.h"

#include <QAction>
#include <QActionGroup>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QIcon>
#include <QKeySequence>
#include <QMenu>
#include <QMetaObject>

namespace
{
void registerDBusMenuTypes()
{
    static const bool registered = [] {
        qDBusRegisterMetaType<DBusMenuItem>();
        qDBusRegisterMetaType<DBusMenuItemList>();
        qDBusRegisterMetaType<DBusMenuItemKeys>();
        qDBusRegisterMetaType<DBusMenuItemKeysList>();
        qDBusRegisterMetaType<DBusMenuLayoutItem>();
        qDBusRegisterMetaType<DBusMenuLayoutItemList>();
        qDBusRegisterMetaType<DBusMenuShortcut>();
        return true;
    }();
    Q_UNUSED(registered)
}

QString dbusMenuLabel(QString text)
{
    // DBusMenu labels do not use Qt's ampersand mnemonic markers.
    text.replace(QStringLiteral("&&"), QStringLiteral("\x1f"));
    text.remove(QLatin1Char('&'));
    text.replace(QStringLiteral("\x1f"), QStringLiteral("&"));
    return text;
}
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItem &item)
{
    argument.beginStructure();
    argument << item.id << item.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItem &item)
{
    argument.beginStructure();
    argument >> item.id >> item.properties;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuItemKeys &item)
{
    argument.beginStructure();
    argument << item.id << item.properties;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuItemKeys &item)
{
    argument.beginStructure();
    argument >> item.id >> item.properties;
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuLayoutItem &item)
{
    argument.beginStructure();
    argument << item.id << item.properties;
    argument.beginArray(qMetaTypeId<QDBusVariant>());
    for (const DBusMenuLayoutItem &child : item.children)
        argument << QDBusVariant(QVariant::fromValue(child));
    argument.endArray();
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuLayoutItem &item)
{
    argument.beginStructure();
    argument >> item.id >> item.properties;
    argument.beginArray();
    while (!argument.atEnd()) {
        QDBusVariant childVariant;
        argument >> childVariant;
        const QDBusArgument childArgument = childVariant.variant().value<QDBusArgument>();
        DBusMenuLayoutItem child;
        childArgument >> child;
        item.children.append(child);
    }
    argument.endArray();
    argument.endStructure();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const DBusMenuShortcut &shortcut)
{
    argument.beginArray(qMetaTypeId<QStringList>());
    for (const QStringList &sequence : shortcut)
        argument << sequence;
    argument.endArray();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, DBusMenuShortcut &shortcut)
{
    shortcut.clear();
    argument.beginArray();
    while (!argument.atEnd()) {
        QStringList sequence;
        argument >> sequence;
        shortcut.append(sequence);
    }
    argument.endArray();
    return argument;
}

DBusMenuExporter::DBusMenuExporter(const QString &path, QMenu *menu,
                                   const QDBusConnection &connection)
    : QObject(menu)
    , mPath(path)
    , mMenu(menu)
    , mConnection(connection)
{
    registerDBusMenuTypes();
    synchronizeActions();
    mConnection.registerObject(mPath, this,
                               QDBusConnection::ExportAllSlots
                                   | QDBusConnection::ExportAllSignals
                                   | QDBusConnection::ExportAllProperties);
}

DBusMenuExporter::~DBusMenuExporter()
{
    if (mConnection.isConnected())
        mConnection.unregisterObject(mPath);
}

uint DBusMenuExporter::version() const
{
    return 4;
}

QString DBusMenuExporter::status() const
{
    return QStringLiteral("normal");
}

void DBusMenuExporter::Event(int id, const QString &eventId, const QDBusVariant &data,
                             uint timestamp)
{
    Q_UNUSED(data)
    Q_UNUSED(timestamp)

    QAction *action = actionForId(id);
    if (!action)
        return;

    if (eventId == QLatin1String("clicked"))
        action->trigger();
    else if (eventId == QLatin1String("hovered"))
        action->hover();
}

QDBusVariant DBusMenuExporter::GetProperty(int id, const QString &property)
{
    QAction *action = actionForId(id);
    const QVariantMap properties = propertiesForAction(action, {property});
    return QDBusVariant(properties.value(property));
}

void DBusMenuExporter::GetLayout(int parentId, int recursionDepth,
                                 const QStringList &propertyNames, uint &revision,
                                 DBusMenuLayoutItem &item)
{
    synchronizeActions();
    revision = mRevision;
    item = layoutFor(parentId, recursionDepth, propertyNames);
}

DBusMenuItemList DBusMenuExporter::GetGroupProperties(const QList<int> &ids,
                                                       const QStringList &propertyNames)
{
    synchronizeActions();

    DBusMenuItemList result;
    for (int id : ids) {
        QAction *action = actionForId(id);
        if (!action)
            continue;
        result.append({id, propertiesForAction(action, propertyNames)});
    }
    return result;
}

bool DBusMenuExporter::AboutToShow(int id)
{
    QMenu *menu = menuForId(id);
    if (!menu)
        return false;

    // Applications commonly rebuild a menu from QMenu::aboutToShow. The
    // StatusNotifier host cannot show the application's local QMenu, so invoke
    // that notification explicitly before it requests the refreshed layout.
    QMetaObject::invokeMethod(menu, "aboutToShow", Qt::DirectConnection);
    synchronizeActions();
    emitLayoutUpdated(id);
    return true;
}

int DBusMenuExporter::idForAction(QAction *action)
{
    if (!action)
        return 0;

    const auto it = mActionIds.constFind(action);
    if (it != mActionIds.cend())
        return it.value();

    const int id = mNextId++;
    mActionIds.insert(action, id);
    mActions.insert(id, action);

    if (!mObservedActions.contains(action)) {
        mObservedActions.insert(action, true);
        connect(action, &QAction::changed, this, [this, action] {
            emitPropertiesUpdated(action);
        });
        connect(action, &QObject::destroyed, this, [this, action] {
            const int actionId = mActionIds.take(action);
            if (actionId)
                mActions.remove(actionId);
            mObservedActions.remove(action);
            emitLayoutUpdated();
        });
    }

    return id;
}

QAction *DBusMenuExporter::actionForId(int id)
{
    return id == 0 ? nullptr : mActions.value(id, nullptr);
}

QMenu *DBusMenuExporter::menuForId(int id)
{
    if (id == 0)
        return mMenu;

    QAction *action = actionForId(id);
    return action ? action->menu() : nullptr;
}

void DBusMenuExporter::observeMenu(QMenu *menu)
{
    if (!menu || mObservedMenus.contains(menu))
        return;

    mObservedMenus.insert(menu, true);
    connect(menu, &QMenu::aboutToShow, this, [this, menu] {
        synchronizeActions();
        Q_UNUSED(menu)
        emitLayoutUpdated();
    });
    connect(menu, &QObject::destroyed, this, [this, menu] {
        mObservedMenus.remove(menu);
        emitLayoutUpdated();
    });
}

void DBusMenuExporter::synchronizeActions()
{
    synchronizeMenu(mMenu);
}

void DBusMenuExporter::synchronizeMenu(QMenu *menu)
{
    if (!menu)
        return;

    observeMenu(menu);
    const QList<QAction *> actions = menu->actions();
    for (QAction *action : actions) {
        idForAction(action);
        synchronizeMenu(action->menu());
    }
}

QVariantMap DBusMenuExporter::propertiesForAction(const QAction *action,
                                                  const QStringList &propertyNames)
{
    QVariantMap properties;
    if (!action)
        return properties;

    const auto add = [&properties, &propertyNames](const QString &name, const QVariant &value) {
        if (propertyNames.isEmpty() || propertyNames.contains(name))
            properties.insert(name, value);
    };

    if (action->isSeparator()) {
        add(QStringLiteral("type"), QStringLiteral("separator"));
        add(QStringLiteral("visible"), action->isVisible());
        return properties;
    }

    add(QStringLiteral("label"), dbusMenuLabel(action->text()));
    add(QStringLiteral("enabled"), action->isEnabled());
    add(QStringLiteral("visible"), action->isVisible());
    add(QStringLiteral("type"), QStringLiteral("standard"));

    if (action->isCheckable()) {
        const bool isRadio = action->actionGroup() && action->actionGroup()->isExclusive();
        add(QStringLiteral("toggle-type"), isRadio ? QStringLiteral("radio")
                                                     : QStringLiteral("checkmark"));
        add(QStringLiteral("toggle-state"), action->isChecked() ? 1 : 0);
    }

    if (!action->icon().name().isEmpty())
        add(QStringLiteral("icon-name"), action->icon().name());

    if (action->menu())
        add(QStringLiteral("children-display"), QStringLiteral("submenu"));

    if (!action->shortcut().isEmpty()) {
        DBusMenuShortcut shortcut;
        shortcut.append(action->shortcut().toString(QKeySequence::PortableText).split(QLatin1Char('+')));
        add(QStringLiteral("shortcut"), QVariant::fromValue(shortcut));
    }

    return properties;
}

DBusMenuLayoutItem DBusMenuExporter::layoutFor(int id, int recursionDepth,
                                               const QStringList &propertyNames)
{
    DBusMenuLayoutItem item;
    item.id = id;

    QAction *action = actionForId(id);
    item.properties = propertiesForAction(action, propertyNames);

    if (recursionDepth == 0)
        return item;

    QMenu *menu = menuForId(id);
    if (!menu)
        return item;

    const int nextDepth = recursionDepth < 0 ? recursionDepth : recursionDepth - 1;
    const QList<QAction *> actions = menu->actions();
    for (QAction *child : actions)
        item.children.append(layoutFor(idForAction(child), nextDepth, propertyNames));

    return item;
}

void DBusMenuExporter::emitLayoutUpdated(int parentId)
{
    ++mRevision;
    Q_EMIT LayoutUpdated(mRevision, parentId);
}

void DBusMenuExporter::emitPropertiesUpdated(QAction *action)
{
    if (!action)
        return;

    const int id = idForAction(action);
    Q_EMIT ItemsPropertiesUpdated({{id, propertiesForAction(action, {})}}, {});
    emitLayoutUpdated();
}
