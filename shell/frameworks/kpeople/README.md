# KPeople

Provides access to all contacts and aggregates them by person.

## Introduction

KPeople offers unified access to our contacts from different sources, grouping
them by person while still exposing all the data.

Furthermore, KPeople will also provide facilities to integrate the data provided
in user interfaces by providing QML and Qt Widgets components.

The sources are plugin-based, allowing to easily extend the contacts collection.

## Usage

### Components

To use KPeople, you'll want to look for it using

```cmake
find_package(KF6People)
```

Or its qmake counterpart. Then you'll have available 3 different libraries to
depend on:

* KPeople which is the most basic one and will mainly provide PersonsModel
and PersonData. You will use them respectively in case you need information
about the full set of contacts on your system or just the one.

* KPeopleWidgets will be required in case we want to integrate it into your
QtWidgets-based application. We will mainly find the PersonDetailsView and
the MergeDialog.

* KPeopleBackend is the library used to make it possible to create backends
for KPeople. You should _only_ use it in case you are providing support for
a new backend type.

\warning KPeopleBackend ABI interfaces are not yet stable.

* Also KPeople also bundles a qml import module that can be pulled using:

```qml
import org.kde.people 1.0
```

This will allow access to the KPeople data from QML code-bases.

### Models

KPeople provides Qt models for accessing different person related data.
KPeople::PersonsModel lists all available contacts. Actions for each contact can be
accessed by providing KPeople::PersonActions with a KPeople::PersonsModel::personUri.

### Person Data

KPeople::PersonsModel and KPeople::PersonData also provide raw access to the VCard of a contact.

VCard is a standard that is supported on many platforms and supports storing all kind
of information about a person, including instant messenger contacts, phone number, names etc.
It can be processed using a KContacts::VCardConverter.

### Editing contacts

To make changes to a contact, its vcard can be edited by setting the custom property vcard of
KPeople::PersonData to the new raw vcard data. New vcard data can be created using the
KPeople::Addressee class, from which the KPeople::VCardConverter can create a vcard.

### Adding / Removing

KPeople::PersonPluginManager provides static functions for adding and removing contacts.

KPeople::PersonPluginManager::addContact() can be fed with a QVariantMap, which has to contain
the key “vcard” with vcard data.

KPeople::PersonPluginManager::deleteContact() deletes the contact with the specified uri.
