/*
    This file is part of the KContacts framework.
    SPDX-FileCopyrightText: 2008 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2008 Kevin Krammer <kevin.krammer@gmx.at>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contactgrouptool.h"
#include "contactgroup.h"

#include <QIODevice>

#include <QXmlStreamReader>
#include <QXmlStreamWriter>

using namespace KContacts;

class XmlContactGroupWriter : public QXmlStreamWriter
{
public:
    XmlContactGroupWriter();

    void write(const ContactGroup &group, QIODevice *device);
    void write(const QList<ContactGroup> &groupLis, QIODevice *device);

private:
    void writeGroup(const ContactGroup &group);
    void writeContactReference(const ContactGroup::ContactReference &reference);
    void writeContactGroupReference(const ContactGroup::ContactGroupReference &reference);
    void writeData(const ContactGroup::Data &data);
};

XmlContactGroupWriter::XmlContactGroupWriter()
{
    setAutoFormatting(true);
}

void XmlContactGroupWriter::write(const ContactGroup &group, QIODevice *device)
{
    setDevice(device);

    writeStartDocument();

    writeGroup(group);

    writeEndDocument();
}

void XmlContactGroupWriter::write(const QList<ContactGroup> &groupList, QIODevice *device)
{
    setDevice(device);

    writeStartDocument();

    writeStartElement(QStringLiteral("contactGroupList"));

    for (const ContactGroup &group : groupList) {
        writeGroup(group);
    }

    writeEndElement();

    writeEndDocument();
}

void XmlContactGroupWriter::writeGroup(const ContactGroup &group)
{
    writeStartElement(QStringLiteral("contactGroup"));
    writeAttribute(QStringLiteral("uid"), group.id());
    writeAttribute(QStringLiteral("name"), group.name());

    const uint contactCount(group.contactReferenceCount());
    for (uint i = 0; i < contactCount; ++i) {
        writeContactReference(group.contactReference(i));
    }

    const uint contactGroupReference(group.contactGroupReferenceCount());
    for (uint i = 0; i < contactGroupReference; ++i) {
        writeContactGroupReference(group.contactGroupReference(i));
    }

    const uint dataCount(group.dataCount());
    for (uint i = 0; i < dataCount; ++i) {
        writeData(group.data(i));
    }

    writeEndElement();
}

void XmlContactGroupWriter::writeContactReference(const ContactGroup::ContactReference &reference)
{
    writeStartElement(QStringLiteral("contactReference"));
    writeAttribute(QStringLiteral("uid"), reference.uid());
    writeAttribute(QStringLiteral("gid"), reference.gid());
    if (!reference.preferredEmail().isEmpty()) {
        writeAttribute(QStringLiteral("preferredEmail"), reference.preferredEmail());
    }

    // TODO: customs

    writeEndElement();
}

void XmlContactGroupWriter::writeContactGroupReference(const ContactGroup::ContactGroupReference &reference)
{
    writeStartElement(QStringLiteral("contactGroupReference"));
    writeAttribute(QStringLiteral("uid"), reference.uid());

    // TODO: customs

    writeEndElement();
}

void XmlContactGroupWriter::writeData(const ContactGroup::Data &data)
{
    writeStartElement(QStringLiteral("contactData"));
    writeAttribute(QStringLiteral("name"), data.name());
    writeAttribute(QStringLiteral("email"), data.email());

    // TODO: customs

    writeEndElement();
}

class XmlContactGroupReader : public QXmlStreamReader
{
public:
    XmlContactGroupReader();

    bool read(QIODevice *device, ContactGroup &group);
    bool read(QIODevice *device, QList<ContactGroup> &groupList);

private:
    bool readGroup(ContactGroup &group);
    bool readContactReference(ContactGroup::ContactReference &reference);
    bool readContactGroupReference(ContactGroup::ContactGroupReference &reference);
    bool readData(ContactGroup::Data &data);
};

XmlContactGroupReader::XmlContactGroupReader()
{
}

bool XmlContactGroupReader::read(QIODevice *device, ContactGroup &group)
{
    setDevice(device);

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("contactGroup")) {
                return readGroup(group);
            } else {
                raiseError(QStringLiteral("The document does not describe a ContactGroup"));
            }
        }
    }

    return error() == NoError;
}

bool XmlContactGroupReader::read(QIODevice *device, QList<ContactGroup> &groupList)
{
    setDevice(device);

    int depth = 0;

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            ++depth;
            if (depth == 1) {
                if (name() == QLatin1String("contactGroupList")) {
                    continue;
                } else {
                    raiseError(QStringLiteral("The document does not describe a list of ContactGroup"));
                }
            } else if (depth == 2) {
                if (name() == QLatin1String("contactGroup")) {
                    ContactGroup group;
                    if (!readGroup(group)) {
                        return false;
                    }

                    groupList.append(group);
                } else {
                    raiseError(QStringLiteral("The document does not describe a list of ContactGroup"));
                }
            }
        }

        if (isEndElement()) {
            --depth;
        }
    }

    return error() == NoError;
}

bool XmlContactGroupReader::readGroup(ContactGroup &group)
{
    const QXmlStreamAttributes elementAttributes = attributes();
    const auto uid = elementAttributes.value(QLatin1String("uid"));
    if (uid.isEmpty()) {
        raiseError(QStringLiteral("ContactGroup is missing a uid"));
        return false;
    }

    const auto groupName = elementAttributes.value(QLatin1String("name"));
    if (groupName.isEmpty()) {
        raiseError(QStringLiteral("ContactGroup is missing a name"));
        return false;
    }

    group.setId(uid.toString());
    group.setName(groupName.toString());

    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("contactData")) {
                ContactGroup::Data data;
                if (!readData(data)) {
                    return false;
                }
                group.append(data);
            } else if (name() == QLatin1String("contactReference")) {
                ContactGroup::ContactReference reference;
                if (!readContactReference(reference)) {
                    return false;
                }
                group.append(reference);
            } else if (name() == QLatin1String("contactGroupReference")) {
                ContactGroup::ContactGroupReference reference;
                if (!readContactGroupReference(reference)) {
                    return false;
                }
                group.append(reference);
            } else {
                raiseError(QStringLiteral("The document does not describe a ContactGroup"));
            }
        }

        if (isEndElement()) {
            if (name() == QLatin1String("contactGroup")) {
                return true;
            }
        }
    }

    return false;
}

bool XmlContactGroupReader::readData(ContactGroup::Data &data)
{
    const QXmlStreamAttributes elementAttributes = attributes();
    const auto email = elementAttributes.value(QLatin1String("email"));
    if (email.isEmpty()) {
        raiseError(QStringLiteral("ContactData is missing an email address"));
        return false;
    }

    const auto name = elementAttributes.value(QLatin1String("name"));

    data.setName(name.toString());
    data.setEmail(email.toString());

    return true;
}

bool XmlContactGroupReader::readContactReference(ContactGroup::ContactReference &reference)
{
    const QXmlStreamAttributes elementAttributes = attributes();
    const auto uid = elementAttributes.value(QLatin1String("uid"));
    const auto gid = elementAttributes.value(QLatin1String("gid"));
    if (uid.isEmpty() && gid.isEmpty()) {
        raiseError(QStringLiteral("ContactReference is missing both uid and gid"));
        return false;
    }
    const auto preferredEmail = elementAttributes.value(QLatin1String("preferredEmail"));

    reference.setUid(uid.toString());
    reference.setGid(gid.toString());
    reference.setPreferredEmail(preferredEmail.toString());

    return true;
}

bool XmlContactGroupReader::readContactGroupReference(ContactGroup::ContactGroupReference &reference)
{
    const QXmlStreamAttributes elementAttributes = attributes();
    const auto uid = elementAttributes.value(QLatin1String("uid"));
    if (uid.isEmpty()) {
        raiseError(QStringLiteral("ContactGroupReference is missing a uid"));
        return false;
    }

    reference.setUid(uid.toString());

    return true;
}

bool ContactGroupTool::convertFromXml(QIODevice *device, ContactGroup &group, QString *errorMessage)
{
    Q_UNUSED(errorMessage);

    XmlContactGroupReader reader;

    bool ok = reader.read(device, group);

    if (!ok && errorMessage != nullptr) {
        *errorMessage = reader.errorString();
    }

    return ok;
}

bool ContactGroupTool::convertToXml(const ContactGroup &group, QIODevice *device, QString *errorMessage)
{
    Q_UNUSED(errorMessage);

    XmlContactGroupWriter writer;
    writer.write(group, device);

    return true;
}

bool ContactGroupTool::convertFromXml(QIODevice *device, QList<ContactGroup> &groupList, QString *errorMessage)
{
    Q_UNUSED(errorMessage);

    XmlContactGroupReader reader;

    bool ok = reader.read(device, groupList);

    if (!ok && errorMessage != nullptr) {
        *errorMessage = reader.errorString();
    }

    return ok;
}

bool ContactGroupTool::convertToXml(const QList<ContactGroup> &groupList, QIODevice *device, QString *errorMessage)
{
    Q_UNUSED(errorMessage);

    XmlContactGroupWriter writer;
    writer.write(groupList, device);

    return true;
}
