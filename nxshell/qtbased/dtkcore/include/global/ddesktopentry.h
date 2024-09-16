// SPDX-FileCopyrightText: 2019 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#pragma once

#include "dtkcore_global.h"

#include <QIODevice>
#include <QObject>
#include <QVariant>

DCORE_BEGIN_NAMESPACE

class DDesktopEntryPrivate;
class LIBDTKCORESHARED_EXPORT DDesktopEntry
{
    Q_GADGET
public:
    enum EntryType {
        Unknown = 0, //!<@~english Unknown desktop file type. Maybe is invalid.
        Application, //!<@~english The file describes application.
        Link,        //!<@~english The file describes URL.
        Directory,   //!<@~english The file describes directory settings.
        ServiceType, //!<@~english KDE specific type. mentioned in the spec, so listed here too.
        Service,     //!<@~english KDE specific type. mentioned in the spec, so listed here too.
        FSDevice     //!<@~english KDE specific type. mentioned in the spec, so listed here too.
    };
    Q_ENUM(EntryType)

    enum ValueType {
        Unparsed = 0, // Maybe useless, consider remove it?
        String,
        Strings,
        Boolean,
        Numeric,
        NotExisted = 99
    };
    Q_ENUM(ValueType)

    enum Status {
        NoError = 0, //!<@~english No error occurred.
        AccessError, //!<@~english An access error occurred (e.g. trying to write to a read-only file).
        FormatError  //!<@~english A format error occurred (e.g. loading a malformed desktop entry file).
    };
    Q_ENUM(Status)

    explicit DDesktopEntry(const QString &filePath) noexcept;
    ~DDesktopEntry();

    bool save() const;

    Status status() const;
    QStringList keys(const QString &section = "Desktop Entry") const;
    QStringList allGroups(bool sorted = false) const;

    bool contains(const QString &key, const QString &section = "Desktop Entry") const;

    QString name() const;
    QString genericName() const;
    QString ddeDisplayName() const;
    QString comment() const;

    QString rawValue(const QString &key, const QString &section = "Desktop Entry",
                     const QString &defaultValue = QString()) const;
    QString stringValue(const QString &key, const QString &section = "Desktop Entry",
                        const QString &defaultValue = QString()) const;
    QString localizedValue(const QString &key, const QString &localeKey = "default",
                           const QString &section = "Desktop Entry", const QString& defaultValue = QString()) const;
    QString localizedValue(const QString &key, const QLocale &locale,
                           const QString &section = "Desktop Entry", const QString& defaultValue = QString()) const;
    QStringList stringListValue(const QString &key, const QString &section = "Desktop Entry") const;

    bool setRawValue(const QString &value, const QString &key, const QString& section = "Desktop Entry");
    bool setStringValue(const QString &value, const QString &key, const QString& section = "Desktop Entry");
    bool setLocalizedValue(const QString &value, const QString& localeKey,
                           const QString &key, const QString& section = "Desktop Entry");

    bool removeEntry(const QString &key, const QString &section = "Desktop Entry");

    static QString &escape(QString &str);
    static QString &escapeExec(QString &str);
    static QString &unescape(QString &str, bool unescapeSemicolons = false);
    static QString &unescapeExec(QString &str);

protected:
    bool setStatus(const Status &status);

private:
    QScopedPointer<DDesktopEntryPrivate> d_ptr;

    Q_DECLARE_PRIVATE(DDesktopEntry)
};

DCORE_END_NAMESPACE
