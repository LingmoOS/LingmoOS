/*
    Persons Model
    SPDX-FileCopyrightText: 2012 Martin Klapetek <martin.klapetek@gmail.com>
    SPDX-FileCopyrightText: 2012 Aleix Pol Gonzalez <aleixpol@blue-systems.com>
    SPDX-FileCopyrightText: 2013 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "personsmodel.h"

#include "backends/abstractcontact.h"
#include "backends/basepersonsdatasource.h"
#include "imageprovideruri_p.h"
#include "metacontact_p.h"
#include "personmanager_p.h"
#include "personpluginmanager.h"

#include "kpeople_debug.h"

#include <QPixmap>
#include <QStandardPaths>
#include <QUrl>

namespace KPeople
{
class PersonsModelPrivate : public QObject
{
    Q_OBJECT
public:
    PersonsModelPrivate(PersonsModel *qq)
        : q(qq)
    {
    }
    PersonsModel *const q;

    // NOTE This is the opposite way round to the return value from contactMapping() for easier lookups
    QHash<QString /*contactUri*/, QString /*PersonUri*/> contactToPersons;

    // hash of person objects indexed by ID
    QHash<QString /*Person ID*/, QPersistentModelIndex /*Row*/> personIndex;

    // a list so we have an order in the model
    QList<MetaContact> metacontacts;

    QList<AllContactsMonitorPtr> m_sourceMonitors;

    int initialFetchesDoneCount = 0;

    bool isInitialized = false;
    bool hasError = false;

    // methods that manipulate the model
    void addPerson(const MetaContact &mc);
    void removePerson(const QString &id);
    void personChanged(const QString &personUri);
    QString personUriForContact(const QString &contactUri) const;
    QVariant dataForContact(const QString &personUri, const AbstractContact::Ptr &contact, int role) const;

    //     SLOTS
    void onContactsFetched();
    // update when a resource signals a contact has changed
    void onContactAdded(const QString &contactUri, const AbstractContact::Ptr &contact);
    void onContactChanged(const QString &contactUri, const AbstractContact::Ptr &contact);
    void onContactRemoved(const QString &contactUri);

    // update on metadata changes
    void onAddContactToPerson(const QString &contactUri, const QString &newPersonUri);
    void onRemoveContactsFromPerson(const QString &contactUri);

public Q_SLOTS:
    void onMonitorInitialFetchComplete(bool success = true);
};
}

using namespace KPeople;

PersonsModel::PersonsModel(QObject *parent)
    : QAbstractItemModel(parent)
    , d_ptr(new PersonsModelPrivate(this))
{
    Q_D(PersonsModel);
    const auto listPlugins = PersonPluginManager::dataSourcePlugins();
    for (BasePersonsDataSource *dataSource : listPlugins) {
        const AllContactsMonitorPtr monitor = dataSource->allContactsMonitor();
        if (monitor->isInitialFetchComplete()) {
            QMetaObject::invokeMethod(d, "onMonitorInitialFetchComplete", Qt::QueuedConnection, Q_ARG(bool, monitor->initialFetchSuccess()));
        } else {
            connect(monitor.data(), &AllContactsMonitor::initialFetchComplete, d, &PersonsModelPrivate::onMonitorInitialFetchComplete);
        }
        d->m_sourceMonitors << monitor;
    }
    d->onContactsFetched();

    connect(PersonManager::instance(), &PersonManager::contactAddedToPerson, d, &PersonsModelPrivate::onAddContactToPerson);
    connect(PersonManager::instance(), &PersonManager::contactRemovedFromPerson, d, &PersonsModelPrivate::onRemoveContactsFromPerson);

    initResources();
}

PersonsModel::~PersonsModel()
{
}

QHash<int, QByteArray> PersonsModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    roles.insert(PersonUriRole, "personUri");
    roles.insert(PersonVCardRole, "personVCard");
    roles.insert(ContactsVCardRole, "contactsVCard");
    roles.insert(PhoneNumberRole, "phoneNumber");
    roles.insert(PhotoImageProviderUri, "photoImageProviderUri");
    return roles;
}

QVariant PersonsModel::data(const QModelIndex &index, int role) const
{
    Q_D(const PersonsModel);

    // optimization - if we don't cover this role, ignore it
    if (role < Qt::UserRole && role != Qt::DisplayRole && role != Qt::DecorationRole) {
        return QVariant();
    }

    if (index.row() < 0 || index.row() >= rowCount(index.parent())) {
        return QVariant();
    }

    if (index.parent().isValid()) {
        if (role == ContactsVCardRole) {
            return QVariant::fromValue<AbstractContact::List>(AbstractContact::List());
        }
        const MetaContact &mc = d->metacontacts.at(index.parent().row());

        return d->dataForContact(mc.id(), mc.contacts().at(index.row()), role);
    } else {
        const MetaContact &mc = d->metacontacts.at(index.row());
        return d->dataForContact(mc.id(), mc.personAddressee(), role);
    }
}

QVariant PersonsModelPrivate::dataForContact(const QString &personUri, const AbstractContact::Ptr &person, int role) const
{
    switch (role) {
    case PersonsModel::FormattedNameRole:
        return person->customProperty(AbstractContact::NameProperty);
    case PersonsModel::PhotoRole: {
        QVariant pic = person->customProperty(AbstractContact::PictureProperty);
        if (pic.canConvert<QImage>()) {
            QImage avatar = pic.value<QImage>();
            if (!avatar.isNull()) {
                return avatar;
            }
        } else if (pic.canConvert<QPixmap>()) {
            QPixmap avatar = pic.value<QPixmap>();
            if (!avatar.isNull()) {
                return avatar;
            }
        } else if (pic.canConvert<QUrl>() && pic.toUrl().isLocalFile()) {
            QPixmap avatar = QPixmap(pic.toUrl().toLocalFile());
            if (!avatar.isNull()) {
                return avatar;
            }
        }

        // If none of the above were valid images,
        // return the generic one
        return QPixmap(QStringLiteral(":/org.kde.kpeople/pixmaps/dummy_avatar.png"));
    }
    case PersonsModel::PersonUriRole:
        return personUri;
    case PersonsModel::PersonVCardRole:
        return QVariant::fromValue<AbstractContact::Ptr>(person);
    case PersonsModel::ContactsVCardRole:
        return QVariant::fromValue<AbstractContact::List>(metacontacts[personIndex[personUri].row()].contacts());
    case PersonsModel::GroupsRole:
        return person->customProperty(QStringLiteral("all-groups"));
    case PersonsModel::PhoneNumberRole:
        return person->customProperty(AbstractContact::PhoneNumberProperty);
    case PersonsModel::PhotoImageProviderUri:
        return ::photoImageProviderUri(personUri);
    }
    return QVariant();
}

int PersonsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return 1;
}

int PersonsModel::rowCount(const QModelIndex &parent) const
{
    Q_D(const PersonsModel);

    if (!parent.isValid()) {
        return d->metacontacts.size();
    }

    if (parent.isValid() && !parent.parent().isValid()) {
        return d->metacontacts.at(parent.row()).contacts().count();
    }

    return 0;
}

bool PersonsModel::isInitialized() const
{
    Q_D(const PersonsModel);

    return d->isInitialized;
}

QModelIndex PersonsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || row >= rowCount(parent)) {
        return QModelIndex();
    }
    // top level items have internalId -1. Anything >=0 is the row of the top level item
    if (!parent.isValid()) {
        return createIndex(row, column, -1);
    }

    return createIndex(row, column, parent.row());
}

QModelIndex PersonsModel::parent(const QModelIndex &childIndex) const
{
    if (childIndex.internalId() == -1 || !childIndex.isValid()) {
        return QModelIndex();
    }

    return index(childIndex.internalId(), 0, QModelIndex());
}

void PersonsModelPrivate::onMonitorInitialFetchComplete(bool success)
{
    initialFetchesDoneCount++;
    if (!success) {
        hasError = true;
    }
    Q_ASSERT(initialFetchesDoneCount <= m_sourceMonitors.count());
    if (initialFetchesDoneCount == m_sourceMonitors.count()) {
        isInitialized = true;
        Q_EMIT q->modelInitialized(!hasError);
    }
}

void PersonsModelPrivate::onContactsFetched()
{
    QMap<QString, AbstractContact::Ptr> addresseeMap;

    // fetch all already loaded contacts from plugins
    for (const AllContactsMonitorPtr &contactWatcher : std::as_const(m_sourceMonitors)) {
        addresseeMap.insert(contactWatcher->contacts());
    }

    // add metacontacts
    const QMultiHash<QString, QString> contactMapping = PersonManager::instance()->allPersons();

    for (const QString &key : contactMapping.uniqueKeys()) {
        QMap<QString, AbstractContact::Ptr> contacts;
        for (const QString &contact : contactMapping.values(key)) {
            contactToPersons[contact] = key;
            AbstractContact::Ptr ptr = addresseeMap.take(contact);
            if (ptr) {
                contacts[contact] = ptr;
            }
        }
        if (!contacts.isEmpty()) {
            addPerson(MetaContact(key, contacts));
        }
    }

    // add remaining contacts
    QMap<QString, AbstractContact::Ptr>::const_iterator i;
    for (i = addresseeMap.constBegin(); i != addresseeMap.constEnd(); ++i) {
        addPerson(MetaContact(i.key(), i.value()));
    }

    for (const AllContactsMonitorPtr &monitor : std::as_const(m_sourceMonitors)) {
        connect(monitor.data(), &AllContactsMonitor::contactAdded, this, &PersonsModelPrivate::onContactAdded);
        connect(monitor.data(), &AllContactsMonitor::contactChanged, this, &PersonsModelPrivate::onContactChanged);
        connect(monitor.data(), &AllContactsMonitor::contactRemoved, this, &PersonsModelPrivate::onContactRemoved);
    }
}

void PersonsModelPrivate::onContactAdded(const QString &contactUri, const AbstractContact::Ptr &contact)
{
    const QString &personUri = personUriForContact(contactUri);

    QHash<QString, QPersistentModelIndex>::const_iterator pidx = personIndex.constFind(personUri);
    if (pidx != personIndex.constEnd()) {
        int personRow = pidx->row();
        MetaContact &mc = metacontacts[personRow];

        // if the MC object already contains this object, we want to update the row, not do an insert
        if (mc.contactUris().contains(contactUri)) {
            qCWarning(KPEOPLE_LOG) << "Source emitted contactAdded for a contact we already know about " << contactUri;
            onContactChanged(contactUri, contact);
        } else {
            int newContactPos = mc.contacts().size();
            q->beginInsertRows(q->index(personRow), newContactPos, newContactPos);
            mc.insertContact(contactUri, contact);
            q->endInsertRows();
            personChanged(personUri);
        }
    } else { // new contact -> new person
        QMap<QString, AbstractContact::Ptr> map;
        map[contactUri] = contact;
        addPerson(MetaContact(personUri, map));
    }
}

void PersonsModelPrivate::onContactChanged(const QString &contactUri, const AbstractContact::Ptr &contact)
{
    const QString &personUri = personUriForContact(contactUri);
    int personRow = personIndex[personUri].row();
    int contactRow = metacontacts[personRow].updateContact(contactUri, contact);

    const QModelIndex contactIndex = q->index(contactRow, 0, q->index(personRow));

    Q_EMIT q->dataChanged(contactIndex, contactIndex);

    personChanged(personUri);
}

void PersonsModelPrivate::onContactRemoved(const QString &contactUri)
{
    const QString &personUri = personUriForContact(contactUri);

    int personRow = personIndex[personUri].row();

    MetaContact &mc = metacontacts[personRow];
    int contactPosition = mc.contactUris().indexOf(contactUri);
    q->beginRemoveRows(q->index(personRow, 0), contactPosition, contactPosition);
    mc.removeContact(contactUri);
    q->endRemoveRows();

    // if MC object is now invalid remove the person from the list
    if (!mc.isValid()) {
        removePerson(personUri);
    } else {
        personChanged(personUri);
    }
}

void PersonsModelPrivate::onAddContactToPerson(const QString &contactUri, const QString &newPersonUri)
{
    const QString oldPersonUri = personUriForContact(contactUri);

    contactToPersons.insert(contactUri, newPersonUri);

    int oldPersonRow = personIndex[oldPersonUri].row();

    if (oldPersonRow < 0) {
        return;
    }

    MetaContact &oldMc = metacontacts[oldPersonRow];

    // get contact already in the model, remove it from the previous contact
    int contactPosition = oldMc.contactUris().indexOf(contactUri);
    const AbstractContact::Ptr contact = oldMc.contacts().at(contactPosition);

    q->beginRemoveRows(q->index(oldPersonRow), contactPosition, contactPosition);
    oldMc.removeContact(contactUri);
    q->endRemoveRows();

    if (!oldMc.isValid()) {
        removePerson(oldPersonUri);
    } else {
        personChanged(oldPersonUri);
    }

    // if the new person is already in the model, add the contact to it
    QHash<QString, QPersistentModelIndex>::const_iterator pidx = personIndex.constFind(newPersonUri);
    if (pidx != personIndex.constEnd()) {
        int newPersonRow = pidx->row();
        MetaContact &newMc = metacontacts[newPersonRow];
        int newContactPos = newMc.contacts().size();
        q->beginInsertRows(q->index(newPersonRow), newContactPos, newContactPos);
        newMc.insertContact(contactUri, contact);
        q->endInsertRows();
        personChanged(newPersonUri);
    } else { // if the person is not in the model, create a new person and insert it
        QMap<QString, AbstractContact::Ptr> contacts;
        contacts[contactUri] = contact;
        addPerson(MetaContact(newPersonUri, contacts));
    }
}

void PersonsModelPrivate::onRemoveContactsFromPerson(const QString &contactUri)
{
    const QString personUri = personUriForContact(contactUri);
    int personRow = personIndex[personUri].row();
    MetaContact &mc = metacontacts[personRow];

    const AbstractContact::Ptr &contact = mc.contact(contactUri);
    const int index = mc.contactUris().indexOf(contactUri);

    q->beginRemoveRows(personIndex[personUri], index, index);
    mc.removeContact(contactUri);
    q->endRemoveRows();
    contactToPersons.remove(contactUri);

    // if we don't want the person object anymore
    if (!mc.isValid()) {
        removePerson(personUri);
    } else {
        personChanged(personUri);
    }

    // now re-insert as a new contact
    // we know it's not part of a metacontact anymore so reinsert as a contact
    addPerson(MetaContact(contactUri, contact));
}

void PersonsModelPrivate::addPerson(const KPeople::MetaContact &mc)
{
    const QString &id = mc.id();

    int row = metacontacts.size();
    q->beginInsertRows(QModelIndex(), row, row);
    metacontacts.append(mc);
    personIndex[id] = q->index(row);
    q->endInsertRows();
}

void PersonsModelPrivate::removePerson(const QString &id)
{
    QPersistentModelIndex index = personIndex.value(id);
    if (!index.isValid()) { // item not found
        return;
    }

    q->beginRemoveRows(QModelIndex(), index.row(), index.row());
    personIndex.remove(id);
    metacontacts.removeAt(index.row());
    q->endRemoveRows();
}

void PersonsModelPrivate::personChanged(const QString &personUri)
{
    int row = personIndex[personUri].row();
    if (row >= 0) {
        const QModelIndex personIndex = q->index(row);
        Q_EMIT q->dataChanged(personIndex, personIndex);
    }
}

QString PersonsModelPrivate::personUriForContact(const QString &contactUri) const
{
    QHash<QString, QString>::const_iterator it = contactToPersons.constFind(contactUri);
    if (it != contactToPersons.constEnd()) {
        return *it;
    } else {
        return contactUri;
    }
}

QModelIndex PersonsModel::indexForPersonUri(const QString &personUri) const
{
    Q_D(const PersonsModel);
    return d->personIndex.value(personUri);
}

QVariant PersonsModel::get(int row, int role)
{
    return index(row, 0).data(role);
}

QVariant PersonsModel::contactCustomProperty(const QModelIndex &index, const QString &key) const
{
    Q_D(const PersonsModel);
    if (index.parent().isValid()) {
        const MetaContact &mc = d->metacontacts.at(index.parent().row());

        return mc.contacts().at(index.row())->customProperty(key);
    } else {
        const MetaContact &mc = d->metacontacts.at(index.row());
        return mc.personAddressee()->customProperty(key);
    }
}

#include "moc_personsmodel.cpp"
#include "personsmodel.moc"
