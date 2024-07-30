/*
    SPDX-FileCopyrightText: 2013 David Edmundson (davidedmundson@kde.org)

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "persondata.h"

#include "backends/abstractcontact.h"
#include "backends/abstracteditablecontact.h"
#include "backends/basepersonsdatasource.h"
#include "backends/contactmonitor.h"
#include "metacontact_p.h"
#include "personmanager_p.h"
#include "personpluginmanager.h"

#include "kpeople_debug.h"

#include <QStandardPaths>
#include <QUrl>

namespace KPeople
{
class PersonDataPrivate
{
public:
    QStringList contactUris;
    MetaContact metaContact;
    QList<ContactMonitorPtr> watchers;
};
}

using namespace KPeople;

KPeople::PersonData::PersonData(const QString &id, QObject *parent)
    : QObject(parent)
    , d_ptr(new PersonDataPrivate)
{
    Q_D(PersonData);

    if (id.isEmpty()) {
        return;
    }

    QString personUri;
    // query DB
    if (id.startsWith(QLatin1String("kpeople://"))) {
        personUri = id;
    } else {
        personUri = PersonManager::instance()->personUriForContact(id);
    }

    if (personUri.isEmpty()) {
        d->contactUris = QStringList() << id;
    } else {
        d->contactUris = PersonManager::instance()->contactsForPersonUri(personUri);
    }

    QMap<QString, AbstractContact::Ptr> contacts;
    for (const QString &contactUri : std::as_const(d->contactUris)) {
        // load the correct data source for this contact ID
        const QString sourceId = QUrl(contactUri).scheme();
        Q_ASSERT(!sourceId.isEmpty());
        BasePersonsDataSource *dataSource = PersonPluginManager::dataSource(sourceId);
        if (dataSource) {
            ContactMonitorPtr cw = dataSource->contactMonitor(contactUri);
            d->watchers << cw;

            // if the data source already has the contact set it already
            // if not it will be loaded when the contactChanged signal is emitted
            if (cw->contact()) {
                contacts[contactUri] = cw->contact();
            }
            connect(cw.data(), SIGNAL(contactChanged()), SLOT(onContactChanged()));
        } else {
            qCWarning(KPEOPLE_LOG) << "error: creating PersonData for unknown contact" << contactUri << id;
        }
    }

    if (personUri.isEmpty() && contacts.size() == 1) {
        d->metaContact = MetaContact(id, contacts.first());
    } else {
        d->metaContact = MetaContact(personUri, contacts);
    }
}

PersonData::~PersonData()
{
    delete d_ptr;
}

bool PersonData::isValid() const
{
    Q_D(const PersonData);
    return !d->metaContact.id().isEmpty();
}

QString PersonData::personUri() const
{
    Q_D(const PersonData);
    return d->metaContact.id();
}

QStringList PersonData::contactUris() const
{
    Q_D(const PersonData);
    return d->metaContact.contactUris();
}

void PersonData::onContactChanged()
{
    Q_D(PersonData);

    ContactMonitor *watcher = qobject_cast<ContactMonitor *>(sender());
    if (!watcher->contact()) {
        d->metaContact.removeContact(watcher->contactUri());
    } else if (d->metaContact.contactUris().contains(watcher->contactUri())) {
#ifdef __GNUC__
#warning probably not needed anymore
#endif
        d->metaContact.updateContact(watcher->contactUri(), watcher->contact());
    } else {
        d->metaContact.insertContact(watcher->contactUri(), watcher->contact());
    }
    Q_EMIT dataChanged();
}

QPixmap PersonData::photo() const
{
    QPixmap avatar;

    QVariant pic = contactCustomProperty(AbstractContact::PictureProperty);
    if (pic.canConvert<QImage>()) {
        avatar = QPixmap::fromImage(pic.value<QImage>());
    } else if (pic.canConvert<QUrl>()) {
        avatar = QPixmap(pic.toUrl().toLocalFile());
    }

    if (avatar.isNull()) {
        static QString defaultAvatar = QStringLiteral(":/org.kde.kpeople/pixmaps/dummy_avatar.png");
        avatar = QPixmap(defaultAvatar);
    }
    return avatar;
}
QVariant PersonData::contactCustomProperty(const QString &key) const
{
    Q_D(const PersonData);
    return d->metaContact.personAddressee()->customProperty(key);
}

bool KPeople::PersonData::setContactCustomProperty(const QString &key, const QVariant &value)
{
    Q_D(PersonData);
    auto contact = dynamic_cast<AbstractEditableContact *>(d->metaContact.personAddressee().data());

    return contact && contact->setCustomProperty(key, value);
}

QString PersonData::presenceIconName() const
{
    QString contactPresence = contactCustomProperty(QStringLiteral("telepathy-presence")).toString();
    return KPeople::iconNameForPresenceString(contactPresence);
}

QString PersonData::name() const
{
    return contactCustomProperty(AbstractContact::NameProperty).toString();
}

QString PersonData::presence() const
{
    return contactCustomProperty(AbstractContact::PresenceProperty).toString();
}

QUrl PersonData::pictureUrl() const
{
    return contactCustomProperty(AbstractContact::PictureProperty).toUrl();
}

QString PersonData::email() const
{
    return contactCustomProperty(AbstractContact::EmailProperty).toString();
}

QStringList PersonData::groups() const
{
    //     We might want to cache it eventually?

    const QVariantList groups = contactCustomProperty(AbstractContact::GroupsProperty).toList();
    QStringList ret;
    for (const QVariant &g : groups) {
        Q_ASSERT(g.canConvert<QString>());
        ret += g.toString();
    }
    ret.removeDuplicates();
    return ret;
}

QStringList PersonData::allEmails() const
{
    const QVariantList emails = contactCustomProperty(AbstractContact::AllEmailsProperty).toList();
    QStringList ret;
    for (const QVariant &e : emails) {
        Q_ASSERT(e.canConvert<QString>());
        ret += e.toString();
    }
    ret.removeDuplicates();
    return ret;
}

bool KPeople::PersonData::isEditable() const
{
    Q_D(const PersonData);
    return dynamic_cast<const AbstractEditableContact *>(d->metaContact.personAddressee().constData());
}

#include "moc_persondata.cpp"
