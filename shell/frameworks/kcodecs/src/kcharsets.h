/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Lars Knoll <knoll@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCHARSETS_H
#define KCHARSETS_H

#include <kcodecs_export.h>

#include <QCoreApplication>
#include <QList>
#include <QStringList>
#include <memory>

#include "kcodecs.h"

class KCharsetsPrivate;

class QChar;
class QString;

/**
 * @class KCharsets kcharsets.h KCharsets
 *
 * Charset font and encoder/decoder handling.
 *
 * This is needed, because Qt's encoding name matching in
 * QTextCodec::codecForName matches only closely-related encoded names
 * but not alternate names, e.g. found in the reality of the Internet.
 */
class KCODECS_EXPORT KCharsets final
{
    Q_DECLARE_TR_FUNCTIONS(KCharsets)

protected:
    /** Protected constructor. If you need the kcharsets object, use
        KCharsets::charsets() instead.
    */
    KCharsets();

public:
    /**
     * Destructor.
     */
    ~KCharsets();

    /**
     * The global charset manager.
     * @return the global charset manager
     */
    static KCharsets *charsets();

    /**
     * @brief Converts an entity to a character.
     *
     * The string must contain only the
     * entity without the trailing ';'.
     * @param str the entity
     * @return QChar::Null if the entity could not be decoded.
     */
    static QChar fromEntity(QStringView str);

    /**
     * Overloaded member function. Tries to find an entity in the
     * QString str.
     * @param str the string containing entified
     * @param len is a return value, that gives the length of the decoded
     * entity.
     * @return a decoded entity if one could be found, QChar::null
     * otherwise
     */
    static QChar fromEntity(QStringView str, int &len);

    /**
     * Converts a QChar to an entity. The returned string does already
     * contain the leading '&' and the trailing ';'.
     * @param ch the char to convert
     * @return the entity
     */
    static QString toEntity(const QChar &ch);

    /**
     * Scans the given string for entities (like &amp;amp;) and resolves them
     * using fromEntity.
     * @param text the string containing the entities
     * @return the clean string
     */
    static QString resolveEntities(const QString &text);

    /**
     * Lists all available encodings as names.
     * @return the list of all encodings
     */
    QStringList availableEncodingNames() const;

    /**
     * Lists the available encoding names together with a more descriptive language.
     * @return the list of descriptive encoding names
     */
    QStringList descriptiveEncodingNames() const;

    /**
     * Lists the available encoding names grouped by script (or language that uses them).
     * @returns the list of lists consisting of description followed by encoding names (i.e. encodingsByScript().at(i).at(0) is a description for
     * encodingsByScript().at(i).at(k), k>0)
     */
    QList<QStringList> encodingsByScript() const;

    /**
     * @brief Returns a long description for an encoding name.
     * @param encoding the encoding for the language
     * @return the long description for the encoding
     */
    QString descriptionForEncoding(QStringView encoding) const;

    /**
     * Returns the encoding for a string obtained with descriptiveEncodingNames().
     * @param descriptiveName the descriptive name for the encoding
     * @return the name of the encoding
     */
    QString encodingForName(const QString &descriptiveName) const;

private:
    std::unique_ptr<KCharsetsPrivate> const d;
    friend struct KCharsetsSingletonPrivate;
};

#endif
