/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Reginald Stadlbauer <reggie@kde.org>
    SPDX-FileCopyrightText: 1999 Simon Hausmann <hausmann@kde.org>
    SPDX-FileCopyrightText: 2000 Nicolas Hadacek <haadcek@kde.org>
    SPDX-FileCopyrightText: 2000 Kurt Granroth <granroth@kde.org>
    SPDX-FileCopyrightText: 2000 Michael Koch <koch@kde.org>
    SPDX-FileCopyrightText: 2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2002 Joseph Wenninger <jowenn@kde.org>
    SPDX-FileCopyrightText: 2005-2007 Hamish Rodda <rodda@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "config-xmlgui.h"

#include "kactioncollection.h"

#include "debug.h"
#include "kactioncategory.h"
#include "kxmlguiclient.h"
#include "kxmlguifactory.h"

#include <KAuthorized>
#include <KConfigGroup>
#if HAVE_GLOBALACCEL
#include <KGlobalAccel>
#endif
#include <KSharedConfig>

#include <QDomDocument>
#include <QGuiApplication>
#include <QList>
#include <QMap>
#include <QMetaMethod>
#include <QSet>

#include <cstdio>

static bool actionHasGlobalShortcut(const QAction *action)
{
#if HAVE_GLOBALACCEL
    return KGlobalAccel::self()->hasShortcut(action);
#else
    return false;
#endif
}

class KActionCollectionPrivate
{
public:
    KActionCollectionPrivate(KActionCollection *qq)
        : q(qq)
        , configIsGlobal(false)
        , connectTriggered(false)
        , connectHovered(false)

    {
    }

    void setComponentForAction(QAction *action)
    {
        const bool hasGlobalShortcut = actionHasGlobalShortcut(action);
        if (!hasGlobalShortcut) {
            action->setProperty("componentName", m_componentName);
            action->setProperty("componentDisplayName", m_componentDisplayName);
        }
    }

    static QList<KActionCollection *> s_allCollections;

    void _k_associatedWidgetDestroyed(QObject *obj);
    void _k_actionDestroyed(QObject *obj);

    bool writeKXMLGUIConfigFile();

    QString m_componentName;
    QString m_componentDisplayName;

    //! Remove a action from our internal bookkeeping. Returns a nullptr if the
    //! action doesn't belong to us.
    QAction *unlistAction(QAction *);

    QMap<QString, QAction *> actionByName;
    QList<QAction *> actions;

    KActionCollection *q = nullptr;

    const KXMLGUIClient *m_parentGUIClient = nullptr;

    QString configGroup{QStringLiteral("Shortcuts")};
    bool configIsGlobal : 1;

    bool connectTriggered : 1;
    bool connectHovered : 1;

    QList<QWidget *> associatedWidgets;
};

QList<KActionCollection *> KActionCollectionPrivate::s_allCollections;

KActionCollection::KActionCollection(QObject *parent, const QString &cName)
    : QObject(parent)
    , d(new KActionCollectionPrivate(this))
{
    KActionCollectionPrivate::s_allCollections.append(this);

    setComponentName(cName);
}

KActionCollection::KActionCollection(const KXMLGUIClient *parent)
    : QObject(nullptr)
    , d(new KActionCollectionPrivate(this))
{
    KActionCollectionPrivate::s_allCollections.append(this);

    d->m_parentGUIClient = parent;
    d->m_componentName = parent->componentName();
}

KActionCollection::~KActionCollection()
{
    KActionCollectionPrivate::s_allCollections.removeAll(this);
}

void KActionCollection::clear()
{
    d->actionByName.clear();
    qDeleteAll(d->actions);
    d->actions.clear();
}

QAction *KActionCollection::action(const QString &name) const
{
    QAction *action = nullptr;

    if (!name.isEmpty()) {
        action = d->actionByName.value(name);
    }

    return action;
}

QAction *KActionCollection::action(int index) const
{
    // ### investigate if any apps use this at all
    return actions().value(index);
}

int KActionCollection::count() const
{
    return d->actions.count();
}

bool KActionCollection::isEmpty() const
{
    return count() == 0;
}

void KActionCollection::setComponentName(const QString &cName)
{
    for (QAction *a : std::as_const(d->actions)) {
        if (actionHasGlobalShortcut(a)) {
            // Its component name is part of an action's signature in the context of
            // global shortcuts and the semantics of changing an existing action's
            // signature are, as it seems, impossible to get right.
            qCWarning(DEBUG_KXMLGUI) << "KActionCollection::setComponentName does not work on a KActionCollection containing actions with global shortcuts!"
                                     << cName;
            break;
        }
    }

    if (!cName.isEmpty()) {
        d->m_componentName = cName;
    } else {
        d->m_componentName = QCoreApplication::applicationName();
    }
}

QString KActionCollection::componentName() const
{
    return d->m_componentName;
}

void KActionCollection::setComponentDisplayName(const QString &displayName)
{
    d->m_componentDisplayName = displayName;
}

QString KActionCollection::componentDisplayName() const
{
    if (!d->m_componentDisplayName.isEmpty()) {
        return d->m_componentDisplayName;
    }
    if (!QGuiApplication::applicationDisplayName().isEmpty()) {
        return QGuiApplication::applicationDisplayName();
    }
    return QCoreApplication::applicationName();
}

const KXMLGUIClient *KActionCollection::parentGUIClient() const
{
    return d->m_parentGUIClient;
}

QList<QAction *> KActionCollection::actions() const
{
    return d->actions;
}

const QList<QAction *> KActionCollection::actionsWithoutGroup() const
{
    QList<QAction *> ret;
    for (QAction *action : std::as_const(d->actions)) {
        if (!action->actionGroup()) {
            ret.append(action);
        }
    }
    return ret;
}

const QList<QActionGroup *> KActionCollection::actionGroups() const
{
    QSet<QActionGroup *> set;
    for (QAction *action : std::as_const(d->actions)) {
        if (action->actionGroup()) {
            set.insert(action->actionGroup());
        }
    }
    return set.values();
}

QAction *KActionCollection::addAction(const QString &name, QAction *action)
{
    if (!action) {
        return action;
    }

    const QString objectName = action->objectName();
    QString indexName = name;

    if (indexName.isEmpty()) {
        // No name provided. Use the objectName.
        indexName = objectName;

    } else {
        // A name was provided. Check against objectName.
        if ((!objectName.isEmpty()) && (objectName != indexName)) {
            // The user specified a new name and the action already has a
            // different one. The objectName is used for saving shortcut
            // settings to disk. Both for local and global shortcuts.
            qCDebug(DEBUG_KXMLGUI) << "Registering action " << objectName << " under new name " << indexName;
            // If there is a global shortcuts it's a very bad idea.
#if HAVE_GLOBALACCEL
            if (KGlobalAccel::self()->hasShortcut(action)) {
                // In debug mode assert
                Q_ASSERT(!KGlobalAccel::self()->hasShortcut(action));
                // In release mode keep the old name
                qCCritical(DEBUG_KXMLGUI) << "Changing action name from " << objectName << " to " << indexName
                                          << "\nignored because of active global shortcut.";
                indexName = objectName;
            }
#endif
        }

        // Set the new name
        action->setObjectName(indexName);
    }

    // No name provided and the action had no name. Make one up. This will not
    // work when trying to save shortcuts. Both local and global shortcuts.
    if (indexName.isEmpty()) {
        indexName = QString::asprintf("unnamed-%p", (void *)action);
        action->setObjectName(indexName);
    }

    // From now on the objectName has to have a value. Else we cannot safely
    // remove actions.
    Q_ASSERT(!action->objectName().isEmpty());

    // look if we already have THIS action under THIS name ;)
    if (d->actionByName.value(indexName, nullptr) == action) {
        // This is not a multi map!
        Q_ASSERT(d->actionByName.count(indexName) == 1);
        return action;
    }

    if (!KAuthorized::authorizeAction(indexName)) {
        // Disable this action
        action->setEnabled(false);
        action->setVisible(false);
        action->blockSignals(true);
    }

    // Check if we have another action under this name
    if (QAction *oldAction = d->actionByName.value(indexName)) {
        takeAction(oldAction);
    }

    // Check if we have this action under a different name.
    // Not using takeAction because we don't want to remove it from categories,
    // and because it has the new name already.
    const int oldIndex = d->actions.indexOf(action);
    if (oldIndex != -1) {
        d->actionByName.remove(d->actionByName.key(action));
        d->actions.removeAt(oldIndex);
    }

    // Add action to our lists.
    d->actionByName.insert(indexName, action);
    d->actions.append(action);

    for (QWidget *widget : std::as_const(d->associatedWidgets)) {
        widget->addAction(action);
    }

    connect(action, &QObject::destroyed, this, [this](QObject *obj) {
        d->_k_actionDestroyed(obj);
    });

    d->setComponentForAction(action);

    if (d->connectHovered) {
        connect(action, &QAction::hovered, this, &KActionCollection::slotActionHovered);
    }

    if (d->connectTriggered) {
        connect(action, &QAction::triggered, this, &KActionCollection::slotActionTriggered);
    }

    Q_EMIT inserted(action);
    Q_EMIT changed();
    return action;
}

void KActionCollection::addActions(const QList<QAction *> &actions)
{
    for (QAction *action : actions) {
        addAction(action->objectName(), action);
    }
}

void KActionCollection::removeAction(QAction *action)
{
    delete takeAction(action);
}

QAction *KActionCollection::takeAction(QAction *action)
{
    if (!d->unlistAction(action)) {
        return nullptr;
    }

    // Remove the action from all widgets
    for (QWidget *widget : std::as_const(d->associatedWidgets)) {
        widget->removeAction(action);
    }

    action->disconnect(this);

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
    Q_EMIT removed(action); // deprecated
#endif
    Q_EMIT changed();
    return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QObject *receiver, const char *member)
{
    QAction *action = KStandardAction::create(actionType, receiver, member, this);
    return action;
}

QAction *KActionCollection::addAction(KStandardAction::StandardAction actionType, const QString &name, const QObject *receiver, const char *member)
{
    // pass 0 as parent, because if the parent is a KActionCollection KStandardAction::create automatically
    // adds the action to it under the default name. We would trigger the
    // warning about renaming the action then.
    QAction *action = KStandardAction::create(actionType, receiver, member, nullptr);
    // Give it a parent for gc.
    action->setParent(this);
    // Remove the name to get rid of the "rename action" warning above
    action->setObjectName(name);
    // And now add it with the desired name.
    return addAction(name, action);
}

QAction *KActionCollection::addAction(const QString &name, const QObject *receiver, const char *member)
{
    QAction *a = new QAction(this);
    if (receiver && member) {
        connect(a, SIGNAL(triggered(bool)), receiver, member);
    }
    return addAction(name, a);
}

QKeySequence KActionCollection::defaultShortcut(QAction *action) const
{
    const QList<QKeySequence> shortcuts = defaultShortcuts(action);
    return shortcuts.isEmpty() ? QKeySequence() : shortcuts.first();
}

QList<QKeySequence> KActionCollection::defaultShortcuts(QAction *action) const
{
    return action->property("defaultShortcuts").value<QList<QKeySequence>>();
}

void KActionCollection::setDefaultShortcut(QAction *action, const QKeySequence &shortcut)
{
    setDefaultShortcuts(action, QList<QKeySequence>() << shortcut);
}

void KActionCollection::setDefaultShortcuts(QAction *action, const QList<QKeySequence> &shortcuts)
{
    action->setShortcuts(shortcuts);
    action->setProperty("defaultShortcuts", QVariant::fromValue(shortcuts));
}

bool KActionCollection::isShortcutsConfigurable(QAction *action) const
{
    // Considered as true by default
    const QVariant value = action->property("isShortcutConfigurable");
    return value.isValid() ? value.toBool() : true;
}

void KActionCollection::setShortcutsConfigurable(QAction *action, bool configurable)
{
    action->setProperty("isShortcutConfigurable", configurable);
}

QString KActionCollection::configGroup() const
{
    return d->configGroup;
}

void KActionCollection::setConfigGroup(const QString &group)
{
    d->configGroup = group;
}

bool KActionCollection::configIsGlobal() const
{
    return d->configIsGlobal;
}

void KActionCollection::setConfigGlobal(bool global)
{
    d->configIsGlobal = global;
}

void KActionCollection::importGlobalShortcuts(KConfigGroup *config)
{
#if HAVE_GLOBALACCEL
    Q_ASSERT(config);
    if (!config || !config->exists()) {
        return;
    }

    for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin(); it != d->actionByName.constEnd(); ++it) {
        QAction *action = it.value();
        if (!action) {
            continue;
        }

        const QString &actionName = it.key();

        if (isShortcutsConfigurable(action)) {
            QString entry = config->readEntry(actionName, QString());
            if (!entry.isEmpty()) {
                KGlobalAccel::self()->setShortcut(action, QKeySequence::listFromString(entry), KGlobalAccel::NoAutoloading);
            } else {
                QList<QKeySequence> defaultShortcut = KGlobalAccel::self()->defaultShortcut(action);
                KGlobalAccel::self()->setShortcut(action, defaultShortcut, KGlobalAccel::NoAutoloading);
            }
        }
    }
#else
    Q_UNUSED(config);
#endif
}

void KActionCollection::readSettings(KConfigGroup *config)
{
    KConfigGroup cg(KSharedConfig::openConfig(), configGroup());
    if (!config) {
        config = &cg;
    }

    if (!config->exists()) {
        return;
    }

    for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin(); it != d->actionByName.constEnd(); ++it) {
        QAction *action = it.value();
        if (!action) {
            continue;
        }

        if (isShortcutsConfigurable(action)) {
            const QString &actionName = it.key();
            QString entry = config->readEntry(actionName, QString());
            if (!entry.isEmpty()) {
                action->setShortcuts(QKeySequence::listFromString(entry));
            } else {
                action->setShortcuts(defaultShortcuts(action));
            }
        }
    }

    // qCDebug(DEBUG_KXMLGUI) << " done";
}

void KActionCollection::exportGlobalShortcuts(KConfigGroup *config, bool writeAll) const
{
#if HAVE_GLOBALACCEL
    Q_ASSERT(config);
    if (!config) {
        return;
    }

    for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin(); it != d->actionByName.constEnd(); ++it) {
        QAction *action = it.value();
        if (!action) {
            continue;
        }
        const QString &actionName = it.key();

        // If the action name starts with unnamed- spit out a warning. That name
        // will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            qCCritical(DEBUG_KXMLGUI) << "Skipped exporting Shortcut for action without name " << action->text() << "!";
            continue;
        }

        if (isShortcutsConfigurable(action) && KGlobalAccel::self()->hasShortcut(action)) {
            bool bConfigHasAction = !config->readEntry(actionName, QString()).isEmpty();
            bool bSameAsDefault = (KGlobalAccel::self()->shortcut(action) == KGlobalAccel::self()->defaultShortcut(action));
            // If we're using a global config or this setting
            //  differs from the default, then we want to write.
            KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;
            if (configIsGlobal()) {
                flags |= KConfigGroup::Global;
            }
            if (writeAll || !bSameAsDefault) {
                QString s = QKeySequence::listToString(KGlobalAccel::self()->shortcut(action));
                if (s.isEmpty()) {
                    s = QStringLiteral("none");
                }
                qCDebug(DEBUG_KXMLGUI) << "\twriting " << actionName << " = " << s;
                config->writeEntry(actionName, s, flags);
            }
            // Otherwise, this key is the same as default
            //  but exists in config file.  Remove it.
            else if (bConfigHasAction) {
                qCDebug(DEBUG_KXMLGUI) << "\tremoving " << actionName << " because == default";
                config->deleteEntry(actionName, flags);
            }
        }
    }

    config->sync();
#else
    Q_UNUSED(config);
    Q_UNUSED(writeAll);
#endif
}

bool KActionCollectionPrivate::writeKXMLGUIConfigFile()
{
    const KXMLGUIClient *kxmlguiClient = q->parentGUIClient();
    // return false if there is no KXMLGUIClient
    if (!kxmlguiClient || kxmlguiClient->xmlFile().isEmpty()) {
        return false;
    }

    qCDebug(DEBUG_KXMLGUI) << "xmlFile=" << kxmlguiClient->xmlFile();

    QString attrShortcut = QStringLiteral("shortcut");

    // Read XML file
    QString sXml(KXMLGUIFactory::readConfigFile(kxmlguiClient->xmlFile(), q->componentName()));
    QDomDocument doc;
    doc.setContent(sXml);

    // Process XML data

    // Get hold of ActionProperties tag
    QDomElement elem = KXMLGUIFactory::actionPropertiesElement(doc);

    // now, iterate through our actions
    for (QMap<QString, QAction *>::ConstIterator it = actionByName.constBegin(); it != actionByName.constEnd(); ++it) {
        QAction *action = it.value();
        if (!action) {
            continue;
        }

        const QString &actionName = it.key();

        // If the action name starts with unnamed- spit out a warning and ignore
        // it. That name will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            qCCritical(DEBUG_KXMLGUI) << "Skipped writing shortcut for action " << actionName << "(" << action->text() << ")!";
            continue;
        }

        bool bSameAsDefault = (action->shortcuts() == q->defaultShortcuts(action));
        qCDebug(DEBUG_KXMLGUI) << "name = " << actionName << " shortcut = " << QKeySequence::listToString(action->shortcuts())
#if HAVE_GLOBALACCEL
                               << " globalshortcut = " << QKeySequence::listToString(KGlobalAccel::self()->shortcut(action))
#endif
                               << " def = " << QKeySequence::listToString(q->defaultShortcuts(action));

        // now see if this element already exists
        // and create it if necessary (unless bSameAsDefault)
        QDomElement act_elem = KXMLGUIFactory::findActionByName(elem, actionName, !bSameAsDefault);
        if (act_elem.isNull()) {
            continue;
        }

        if (bSameAsDefault) {
            act_elem.removeAttribute(attrShortcut);
            // qCDebug(DEBUG_KXMLGUI) << "act_elem.attributes().count() = " << act_elem.attributes().count();
            if (act_elem.attributes().count() == 1) {
                elem.removeChild(act_elem);
            }
        } else {
            act_elem.setAttribute(attrShortcut, QKeySequence::listToString(action->shortcuts()));
        }
    }

    // Write back to XML file
    KXMLGUIFactory::saveConfigFile(doc, kxmlguiClient->localXMLFile(), q->componentName());
    // and since we just changed the xml file clear the dom we have in memory
    // it'll be rebuilt if needed
    const_cast<KXMLGUIClient *>(kxmlguiClient)->setXMLGUIBuildDocument({});
    return true;
}

void KActionCollection::writeSettings(KConfigGroup *config, bool writeAll, QAction *oneAction) const
{
    // If the caller didn't provide a config group we try to save the KXMLGUI
    // Configuration file. If that succeeds we are finished.
    if (config == nullptr && d->writeKXMLGUIConfigFile()) {
        return;
    }

    KConfigGroup cg(KSharedConfig::openConfig(), configGroup());
    if (!config) {
        config = &cg;
    }

    QList<QAction *> writeActions;
    if (oneAction) {
        writeActions.append(oneAction);
    } else {
        writeActions = actions();
    }

    for (QMap<QString, QAction *>::ConstIterator it = d->actionByName.constBegin(); it != d->actionByName.constEnd(); ++it) {
        QAction *action = it.value();
        if (!action) {
            continue;
        }

        const QString &actionName = it.key();

        // If the action name starts with unnamed- spit out a warning and ignore
        // it. That name will change at will and will break loading writing
        if (actionName.startsWith(QLatin1String("unnamed-"))) {
            qCCritical(DEBUG_KXMLGUI) << "Skipped saving Shortcut for action without name " << action->text() << "!";
            continue;
        }

        // Write the shortcut
        if (isShortcutsConfigurable(action)) {
            bool bConfigHasAction = !config->readEntry(actionName, QString()).isEmpty();
            bool bSameAsDefault = (action->shortcuts() == defaultShortcuts(action));
            // If we're using a global config or this setting
            //  differs from the default, then we want to write.
            KConfigGroup::WriteConfigFlags flags = KConfigGroup::Persistent;

            // Honor the configIsGlobal() setting
            if (configIsGlobal()) {
                flags |= KConfigGroup::Global;
            }

            if (writeAll || !bSameAsDefault) {
                // We are instructed to write all shortcuts or the shortcut is
                // not set to its default value. Write it
                QString s = QKeySequence::listToString(action->shortcuts());
                if (s.isEmpty()) {
                    s = QStringLiteral("none");
                }
                qCDebug(DEBUG_KXMLGUI) << "\twriting " << actionName << " = " << s;
                config->writeEntry(actionName, s, flags);

            } else if (bConfigHasAction) {
                // Otherwise, this key is the same as default but exists in
                // config file. Remove it.
                qCDebug(DEBUG_KXMLGUI) << "\tremoving " << actionName << " because == default";
                config->deleteEntry(actionName, flags);
            }
        }
    }

    config->sync();
}

void KActionCollection::slotActionTriggered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
        Q_EMIT actionTriggered(action);
    }
}

#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
void KActionCollection::slotActionHighlighted()
{
    slotActionHovered();
}
#endif

void KActionCollection::slotActionHovered()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action) {
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
        Q_EMIT actionHighlighted(action);
#endif
        Q_EMIT actionHovered(action);
    }
}

// The downcast from a QObject to a QAction triggers UBSan
// but we're only comparing pointers, so UBSan shouldn't check vptrs
// Similar to https://github.com/itsBelinda/plog/pull/1/files
#if defined(__clang__) || __GNUC__ >= 8
__attribute__((no_sanitize("vptr")))
#endif
void KActionCollectionPrivate::_k_actionDestroyed(QObject *obj)
{
    // obj isn't really a QAction anymore. So make sure we don't do fancy stuff
    // with it.
    QAction *action = static_cast<QAction *>(obj);

    if (!unlistAction(action)) {
        return;
    }

    // HACK the object we emit is partly destroyed
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
    Q_EMIT q->removed(action);
#endif
    Q_EMIT q->changed();
}

void KActionCollection::connectNotify(const QMetaMethod &signal)
{
    if (d->connectHovered && d->connectTriggered) {
        return;
    }

    if (
#if KXMLGUI_BUILD_DEPRECATED_SINCE(5, 0)
        signal.methodSignature() == "actionHighlighted(QAction*)" ||
#endif
        signal.methodSignature() == "actionHovered(QAction*)") {
        if (!d->connectHovered) {
            d->connectHovered = true;
            for (QAction *action : std::as_const(d->actions)) {
                connect(action, &QAction::hovered, this, &KActionCollection::slotActionHovered);
            }
        }

    } else if (signal.methodSignature() == "actionTriggered(QAction*)") {
        if (!d->connectTriggered) {
            d->connectTriggered = true;
            for (QAction *action : std::as_const(d->actions)) {
                connect(action, &QAction::triggered, this, &KActionCollection::slotActionTriggered);
            }
        }
    }

    QObject::connectNotify(signal);
}

const QList<KActionCollection *> &KActionCollection::allCollections()
{
    return KActionCollectionPrivate::s_allCollections;
}

void KActionCollection::associateWidget(QWidget *widget) const
{
    for (QAction *action : std::as_const(d->actions)) {
        if (!widget->actions().contains(action)) {
            widget->addAction(action);
        }
    }
}

void KActionCollection::addAssociatedWidget(QWidget *widget)
{
    if (!d->associatedWidgets.contains(widget)) {
        widget->addActions(actions());

        d->associatedWidgets.append(widget);
        connect(widget, &QObject::destroyed, this, [this](QObject *obj) {
            d->_k_associatedWidgetDestroyed(obj);
        });
    }
}

void KActionCollection::removeAssociatedWidget(QWidget *widget)
{
    for (QAction *action : std::as_const(d->actions)) {
        widget->removeAction(action);
    }

    d->associatedWidgets.removeAll(widget);
    disconnect(widget, &QObject::destroyed, this, nullptr);
}

QAction *KActionCollectionPrivate::unlistAction(QAction *action)
{
    // ATTENTION:
    //   This method is called with an QObject formerly known as a QAction
    //   during _k_actionDestroyed(). So don't do fancy stuff here that needs a
    //   real QAction!

    // Get the index for the action
    int index = actions.indexOf(action);

    // Action not found.
    if (index == -1) {
        return nullptr;
    }

    // An action collection can't have the same action twice.
    Q_ASSERT(actions.indexOf(action, index + 1) == -1);

    // Get the actions name
    const QString name = action->objectName();

    // Remove the action
    actionByName.remove(name);
    actions.removeAt(index);

    // Remove the action from the categories. Should be only one
    const QList<KActionCategory *> categories = q->findChildren<KActionCategory *>();
    for (KActionCategory *category : categories) {
        category->unlistAction(action);
    }

    return action;
}

QList<QWidget *> KActionCollection::associatedWidgets() const
{
    return d->associatedWidgets;
}

void KActionCollection::clearAssociatedWidgets()
{
    for (QWidget *widget : std::as_const(d->associatedWidgets)) {
        for (QAction *action : std::as_const(d->actions)) {
            widget->removeAction(action);
        }
    }

    d->associatedWidgets.clear();
}

void KActionCollectionPrivate::_k_associatedWidgetDestroyed(QObject *obj)
{
    associatedWidgets.removeAll(static_cast<QWidget *>(obj));
}

#include "moc_kactioncollection.cpp"
