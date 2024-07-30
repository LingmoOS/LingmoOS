/*
    A temporary copy to break dependency to KLDAP

    This file is part of libkldap.
    SPDX-FileCopyrightText: 2004-2006 Szombathelyi György <gyurco@freemail.hu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef LDIF_P_H
#define LDIF_P_H

#include <QByteArray>
#include <QString>

/**
 * Ldif
 *
 * Ldif implements an RFC 2849 compliant Ldif parser. Ldif files are used to
 * represent directory information on LDAP-based servers, or to describe a set
 * of changes which are to be applied to a directory.
 */
class Ldif
{
public:
    typedef enum { None, NewEntry, EndEntry, Item, Control, Err, MoreData } ParseValue;

    typedef enum { Entry_None, Entry_Add, Entry_Del, Entry_Mod, Entry_Modrdn } EntryType;

    typedef enum { Mod_None, Mod_Add, Mod_Replace, Mod_Del } ModType;

    Ldif();

    Ldif(const Ldif &that);
    Ldif &operator=(const Ldif &that);

    virtual ~Ldif();

    /**
     * Assembles fieldname and value into a valid Ldif line, BASE64 encodes the
     * value if necessary and optionally splits into more lines.
     * @param fieldname The name of the entry.
     * @param value The value of the entry.
     * @param linelen Maximum length of the lines in the result.
     * @param url If true, encode value as url ( use :< ).
     */
    static QByteArray assembleLine(const QString &fieldname, const QByteArray &value, uint linelen = 0, bool url = false);
    /**
     * This is the same as the above function, the only difference that
     * this accepts QString as the value.
     */
    static QByteArray assembleLine(const QString &fieldname, const QString &value, uint linelen = 0, bool url = false);

    /**
     * Splits one line from an Ldif file to attribute and value components.
     * @return true if value is an URL, false otherwise
     */
    static bool splitLine(const QByteArray &line, QString &fieldname, QByteArray &value);

    /**
     * Splits a control specification (without the "control:" directive)
     * @param line is the control directive
     * @param oid will contain the OID
     * @param critical will contain the criticality of control
     * @param value is the control value
     */
    static bool splitControl(const QByteArray &line, QString &oid, bool &critical, QByteArray &value);

    /**
     * Starts the parsing of a new Ldif
     */
    void startParsing();

    /**
     * Process one Ldif line
     */
    ParseValue processLine();

    /**
     * Process the Ldif until a complete item can be returned
     * @return NewEntry if a new DN encountered, Item if a new item returned,
     * Err if the Ldif contains error, EndEntry if the parser reached the end
     * of the current entry and MoreData if the parser encountered the end of
     * the current chunk of the Ldif.
     *
     * If you want to finish the parsing after receiving MoreData, then call
     * endLdif(), so the parser can safely flush the current entry.
     */
    ParseValue nextItem();

    /**
     * Sets a chunk of Ldif. Call before startParsing(), or if nextItem()
     * returned MoreData.
     */
    void setLdif(const QByteArray &ldif);

    /**
     * Indicates the end of the Ldif file/stream. Call if nextItem() returned
     * MoreData, but actually you don't have more data.
     */
    void endLdif();

    /**
     * Returns the requested LDAP operation extracted from the current entry.
     */
    EntryType entryType() const;

    /**
     * Returns the LDAP modify request type if entryType() returned Entry_Mod.
     */
    int modType() const;

    /**
     * Returns the new Relative Distinguished Name if modType() returned
     * Entry_Modrdn.
     */
    QString newRdn() const;

    /**
     * Returns the new parent of the entry if modType() returned Entry_Modrdn.
     */
    QString newSuperior() const;

    /**
     * Returns if the delete of the old RDN is required.
     */
    bool delOldRdn() const;

    /**
     * Returns the attribute name.
     */
    QString attr() const;

    /**
     * Returns the attribute value.
     */
    QByteArray value() const;

    /**
     * Returns if val() is an url
     */
    bool isUrl() const;

    /**
     * Returns the criticality level when modType() returned Control.
     */
    bool isCritical() const;

    /**
     * Returns the OID when modType() returned Control.
     */
    QString oid() const;

    /**
     * Returns the line number which the parser processes.
     */
    uint lineNumber() const;

private:
    class LdifPrivate;
    LdifPrivate *const d;
};

#endif
