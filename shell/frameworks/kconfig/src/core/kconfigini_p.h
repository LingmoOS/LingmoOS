/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2006, 2007 Thomas Braxton <kde.braxton@gmail.com>
   SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
   SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCONFIGINI_P_H
#define KCONFIGINI_P_H

#include <QCoreApplication>
#include <QFile>
#include <QMutex>
#include <QSharedData>
#include <kconfigbase.h>
#include <kconfigcore_export.h>

class QLockFile;
class QIODevice;
class KEntryMap;

class KConfigIniBackend : public QSharedData
{
    Q_GADGET
    Q_DECLARE_TR_FUNCTIONS(KConfigIniBackend)

private:
    QLockFile *lockFile;
    QMutex m_mutex;

public:
    class BufferFragment;

    KConfigIniBackend();

    /** Allows the behaviour of parseConfig() to be tuned */
    enum ParseOption {
        ParseGlobal = 1, /// entries should be marked as @em global
        ParseDefaults = 2, /// entries should be marked as @em default
        ParseExpansions = 4, /// entries are allowed to be marked as @em expandable
    };
    Q_FLAG(ParseOption)
    /// @typedef typedef QFlags<ParseOption> ParseOptions
    Q_DECLARE_FLAGS(ParseOptions, ParseOption)

    /** Allows the behaviour of writeConfig() to be tuned */
    enum WriteOption {
        WriteGlobal = 1 /// only write entries marked as "global"
    };
    Q_FLAG(WriteOption)
    /// @typedef typedef QFlags<WriteOption> WriteOptions
    Q_DECLARE_FLAGS(WriteOptions, WriteOption)

    /** Return value from parseConfig() */
    enum ParseInfo {
        ParseOk, /// the configuration was opened read/write
        ParseImmutable, /// the configuration is @em immutable
        ParseOpenError, /// the configuration could not be opened
    };

    ParseInfo parseConfig(const QByteArray &locale, KEntryMap &entryMap, ParseOptions options);
    ParseInfo parseConfig(const QByteArray &locale, KEntryMap &entryMap, ParseOptions options, bool merging);
    bool writeConfig(const QByteArray &locale, KEntryMap &entryMap, WriteOptions options);

    bool isWritable() const;
    QString nonWritableErrorMessage() const;
    KConfigBase::AccessMode accessMode() const;
    void createEnclosing();
    void setFilePath(const QString &path);
    bool lock();
    void unlock();
    bool isLocked() const;

    /** @return the absolute path to the object */
    QString filePath() const;

private:
    enum StringType {
        GroupString = 0,
        KeyString = 1,
        ValueString = 2,
    };
    // Warning: this modifies data in-place. Other BufferFragment objects referencing the same buffer
    // fragment will get their data modified too.
    static void printableToString(BufferFragment *aString, const QFile &file, int line);
    static QByteArray stringToPrintable(const QByteArray &aString, StringType type);
    static char charFromHex(const char *str, const QFile &file, int line);
    static QString warningProlog(const QFile &file, int line);

    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map);
    void writeEntries(const QByteArray &locale, QIODevice &file, const KEntryMap &map, bool defaultGroup, bool &firstEntry);

    void setLocalFilePath(const QString &file);

    QString mLocalFilePath;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigIniBackend::ParseOptions)
Q_DECLARE_OPERATORS_FOR_FLAGS(KConfigIniBackend::WriteOptions)

#endif // KCONFIGINI_P_H
