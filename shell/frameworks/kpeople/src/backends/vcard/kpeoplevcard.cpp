/*
    SPDX-FileCopyrightText: 2015 Aleix Pol Gonzalez <aleixpol@kde.org>
    SPDX-FileCopyrightText: 2015 Martin Klapetek <mklapetek@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "kpeoplevcard.h"
#include <KContacts/Picture>
#include <KContacts/VCardConverter>
#include <KLocalizedString>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QStandardPaths>

#include <KFileUtils>
#include <KPluginFactory>

using namespace KPeople;

#ifdef Q_OS_WIN
Q_GLOBAL_STATIC_WITH_ARGS(QString, vcardsLocation, (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString::fromLatin1("/Contacts")))
#else
Q_GLOBAL_STATIC_WITH_ARGS(QString,
                          vcardsLocation,
                          (QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QString::fromLatin1("/kpeoplevcard")))
#endif

#ifdef Q_OS_WIN
Q_GLOBAL_STATIC_WITH_ARGS(QString, vcardsWriteLocation, (QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + QString::fromLatin1("/Contacts/own")))
#else
Q_GLOBAL_STATIC_WITH_ARGS(QString,
                          vcardsWriteLocation,
                          (QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QString::fromLatin1("/kpeoplevcard/own/")))
#endif

class VCardContact : public AbstractEditableContact
{
public:
    VCardContact()
    {
    }
    VCardContact(const KContacts::Addressee &addr, const QUrl &location)
        : m_addressee(addr)
        , m_location(location)
    {
    }
    void setAddressee(const KContacts::Addressee &addr)
    {
        m_addressee = addr;
    }
    void setUrl(const QUrl &url)
    {
        m_location = url;
    }

    QVariant customProperty(const QString &key) const override
    {
        QVariant ret;
        if (key == NameProperty) {
            const QString name = m_addressee.realName();
            if (!name.isEmpty()) {
                return name;
            }

            // If both first and last name are set combine them to a full name
            if (!m_addressee.givenName().isEmpty() && !m_addressee.familyName().isEmpty())
                return i18ndc("kpeoplevcard", "given-name family-name", "%1 %2", m_addressee.givenName(), m_addressee.familyName());

            // If only one of them is set just return what we know
            if (!m_addressee.givenName().isEmpty())
                return m_addressee.givenName();
            if (!m_addressee.familyName().isEmpty())
                return m_addressee.familyName();

            // Fall back to other identifiers
            if (!m_addressee.preferredEmail().isEmpty()) {
                return m_addressee.preferredEmail();
            }
            if (!m_addressee.phoneNumbers().isEmpty()) {
                return m_addressee.phoneNumbers().at(0).number();
            }
            return QVariant();
        } else if (key == EmailProperty)
            return m_addressee.preferredEmail();
        else if (key == AllEmailsProperty)
            return m_addressee.emails();
        else if (key == PictureProperty)
            return m_addressee.photo().data();
        else if (key == AllPhoneNumbersProperty) {
            const auto phoneNumbers = m_addressee.phoneNumbers();
            QVariantList numbers;
            for (const KContacts::PhoneNumber &phoneNumber : phoneNumbers) {
                // convert from KContacts specific format to QString
                numbers << phoneNumber.number();
            }
            return numbers;
        } else if (key == PhoneNumberProperty) {
            return m_addressee.phoneNumbers().isEmpty() ? QVariant() : m_addressee.phoneNumbers().at(0).number();
        } else if (key == VCardProperty) {
            KContacts::VCardConverter converter;
            return converter.createVCard(m_addressee);
        }

        return ret;
    }

    bool setCustomProperty(const QString &key, const QVariant &value) override
    {
        if (key == VCardProperty) {
            QFile f(m_location.toLocalFile());
            if (!f.open(QIODevice::WriteOnly))
                return false;
            f.write(value.toByteArray());
            return true;
        }
        return false;
    }

    static QString createUri(const QString &path)
    {
        return QStringLiteral("vcard:/") + path;
    }

private:
    KContacts::Addressee m_addressee;
    QUrl m_location;
};

bool VCardDataSource::addContact(const QVariantMap &properties)
{
    if (!properties.contains("vcard"))
        return false;

    if (!QDir().mkpath(*vcardsWriteLocation))
        return false;

    QFile f(*vcardsWriteLocation + KFileUtils::suggestName(QUrl::fromLocalFile(*vcardsWriteLocation), QStringLiteral("contact.vcard")));
    if (!f.open(QFile::WriteOnly)) {
        qWarning() << "could not open file to write" << f.fileName();
        return false;
    }

    f.write(properties.value("vcard").toByteArray());
    return true;
}

bool VCardDataSource::deleteContact(const QString &uri)
{
    if (!uri.startsWith("vcard:/"))
        return false;

    QString path = uri;
    path.remove("vcard:/");

    if (!path.startsWith(*vcardsLocation))
        return false;

    return QFile::remove(path);
}

KPeopleVCard::KPeopleVCard()
    : KPeople::AllContactsMonitor()
    , m_fs(new KDirWatch(this))
{
    QDir().mkpath(*vcardsLocation);

    processDirectory(QFileInfo(*vcardsLocation));

    emitInitialFetchComplete(true);

    connect(m_fs, &KDirWatch::dirty, this, [this](const QString &path) {
        const QFileInfo fi(path);
        if (fi.isFile())
            processVCard(path);
        else
            processDirectory(fi);
    });
    connect(m_fs, &KDirWatch::created, this, [this](const QString &path) {
        const QFileInfo fi(path);
        if (fi.isFile())
            processVCard(path);
        else
            processDirectory(fi);
    });
    connect(m_fs, &KDirWatch::deleted, this, &KPeopleVCard::deleteVCard);
}

KPeopleVCard::~KPeopleVCard()
{
}

QMap<QString, AbstractContact::Ptr> KPeopleVCard::contacts()
{
    return m_contactForUri;
}

void KPeopleVCard::processDirectory(const QFileInfo &fi)
{
    const QDir dir(fi.absoluteFilePath());
    {
        const auto subdirs = dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot); // includes '.', ie. vcards from no subdir

        for (const auto &subdir : subdirs) {
            processDirectory(subdir);
        }
    }

    {
        const QFileInfoList subdirVcards = dir.entryInfoList({"*.vcard", "*.vcf"});
        for (const QFileInfo &vcardFile : subdirVcards) {
            processVCard(vcardFile.absoluteFilePath());
        }
    }
    m_fs->addDir(dir.absolutePath(), KDirWatch::WatchDirOnly | KDirWatch::WatchSubDirs | KDirWatch::WatchFiles);
}

void KPeopleVCard::processVCard(const QString &path)
{
    m_fs->addFile(path);

    QFile f(path);
    bool b = f.open(QIODevice::ReadOnly);
    if (!b) {
        qWarning() << "error: couldn't open:" << path;
        return;
    }

    KContacts::VCardConverter conv;
    const KContacts::Addressee addressee = conv.parseVCard(f.readAll());

    QString uri = VCardContact::createUri(path);
    auto it = m_contactForUri.find(uri);
    if (it != m_contactForUri.end()) {
        static_cast<VCardContact *>(it->data())->setAddressee(addressee);
        static_cast<VCardContact *>(it->data())->setUrl(QUrl::fromLocalFile(path));
        Q_EMIT contactChanged(uri, *it);
    } else {
        KPeople::AbstractContact::Ptr contact(new VCardContact(addressee, QUrl::fromLocalFile(path)));
        m_contactForUri.insert(uri, contact);
        Q_EMIT contactAdded(uri, contact);
    }
}

void KPeopleVCard::deleteVCard(const QString &path)
{
    if (QFile::exists(path))
        return;
    QString uri = VCardContact::createUri(path);

    const int r = m_contactForUri.remove(uri);
    if (r)
        Q_EMIT contactRemoved(uri);
}

QString KPeopleVCard::contactsVCardPath()
{
    return *vcardsLocation;
}

QString KPeopleVCard::contactsVCardWritePath()
{
    return *vcardsWriteLocation;
}

VCardDataSource::VCardDataSource(QObject *parent, const QVariantList &args)
    : BasePersonsDataSourceV2(parent)
{
    Q_UNUSED(args);
}

VCardDataSource::~VCardDataSource()
{
}

QString VCardDataSource::sourcePluginId() const
{
    return QStringLiteral("vcard");
}

AllContactsMonitor *VCardDataSource::createAllContactsMonitor()
{
    return new KPeopleVCard();
}

K_PLUGIN_CLASS_WITH_JSON(VCardDataSource, "kpeoplevcard.json")

#include "kpeoplevcard.moc"

#include "moc_kpeoplevcard.cpp"
