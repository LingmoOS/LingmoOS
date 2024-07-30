/*
    A temporary copy to break dependency to KLDAP

    This file is part of libkldap.
    SPDX-FileCopyrightText: 2004-2006 Szombathelyi György <gyurco@freemail.hu>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "ldif_p.h"

#include "kcontacts_debug.h"

class Q_DECL_HIDDEN Ldif::LdifPrivate
{
public:
    int mModType;
    bool mDelOldRdn, mUrl;
    QByteArray mDn;
    QString mAttr, mNewRdn, mNewSuperior, mOid;
    QByteArray mLdif, mValue;
    EntryType mEntryType;

    bool mIsNewLine, mIsComment, mCritical;
    ParseValue mLastParseValue;
    uint mPos, mLineNumber;
    QByteArray mLine;
};

Ldif::Ldif()
    : d(new LdifPrivate)
{
    startParsing();
}

Ldif::Ldif(const Ldif &that)
    : d(new LdifPrivate)
{
    *d = *that.d;

    startParsing();
}

Ldif &Ldif::operator=(const Ldif &that)
{
    if (this == &that) {
        return *this;
    }

    *d = *that.d;

    return *this;
}

Ldif::~Ldif()
{
    delete d;
}

QByteArray Ldif::assembleLine(const QString &fieldname, const QByteArray &value, uint linelen, bool url)
{
    QByteArray result;

    if (url) {
        result = fieldname.toUtf8() + ":< " + value;
    } else {
        bool safe = false;
        bool isDn = fieldname.toLower() == QLatin1String("dn");
        // SAFE-INIT-CHAR
        if (value.size() > 0 && value[0] > 0 && value[0] != '\n' //
            && value[0] != '\r' && value[0] != ':' && value[0] != '<') {
            safe = true;
        }

        // SAFE-CHAR
        if (safe) {
            for (int i = 1; i < value.size(); ++i) {
                // allow utf-8 in Distinguished Names
                if ((isDn && value[i] == 0) //
                    || (!isDn && value[i] <= 0) //
                    || value[i] == '\r' || value[i] == '\n') {
                    safe = false;
                    break;
                }
            }
        }

        if (value.isEmpty()) {
            safe = true;
        }

        if (safe) {
            result = fieldname.toUtf8() + ": " + value;
        } else {
            result = fieldname.toUtf8() + ":: " + value.toBase64();
        }

        if (linelen > 0) {
            int i = (uint)(fieldname.length() + 2) > linelen ? fieldname.length() + 2 : linelen;
            while (i < result.length()) {
                result.insert(i, "\n ");
                i += linelen + 2;
            }
        }
    }
    return result;
}

QByteArray Ldif::assembleLine(const QString &fieldname, const QString &value, uint linelen, bool url)
{
    return assembleLine(fieldname, value.toUtf8(), linelen, url);
}

bool Ldif::splitLine(const QByteArray &line, QString &fieldname, QByteArray &value)
{
    int position;
    int linelen;

    //  qCDebug(KCONTACTS_LOG) << "line:" << QString::fromUtf8(line);

    position = line.indexOf(":");
    if (position == -1) {
        // strange: we did not find a fieldname
        fieldname = QLatin1String("");
        value = line.trimmed();
        //    qCDebug(KCONTACTS_LOG) << "value :" << value[0];
        return false;
    }

    linelen = line.size();
    fieldname = QString::fromUtf8(line.left(position).trimmed());

    if (linelen > (position + 1) && line[position + 1] == ':') {
        // String is BASE64 encoded -> decode it now.
        if (linelen <= (position + 3)) {
            value.resize(0);
            return false;
        }
        value = QByteArray::fromBase64(line.mid(position + 3));
        return false;
    }

    if (linelen > (position + 1) && line[position + 1] == '<') {
        // String is an URL.
        if (linelen <= (position + 3)) {
            value.resize(0);
            return false;
        }
        value = QByteArray::fromBase64(line.mid(position + 3));
        return true;
    }

    if (linelen <= (position + 2)) {
        value.resize(0);
        return false;
    }
    value = line.mid(position + 2);
    return false;
}

bool Ldif::splitControl(const QByteArray &line, QString &oid, bool &critical, QByteArray &value)
{
    QString tmp;
    critical = false;
    bool url = splitLine(line, tmp, value);

    qCDebug(KCONTACTS_LOG) << "value:" << QString::fromUtf8(value);
    if (tmp.isEmpty()) {
        tmp = QString::fromUtf8(value);
        value.resize(0);
    }
    if (tmp.endsWith(QLatin1String("true"))) {
        critical = true;
        tmp.chop(5);
    } else if (tmp.endsWith(QLatin1String("false"))) {
        critical = false;
        tmp.chop(6);
    }
    oid = tmp;
    return url;
}

Ldif::ParseValue Ldif::processLine()
{
    if (d->mIsComment) {
        return None;
    }

    ParseValue retval = None;
    if (d->mLastParseValue == EndEntry) {
        d->mEntryType = Entry_None;
    }

    d->mUrl = splitLine(d->mLine, d->mAttr, d->mValue);

    QString attrLower = d->mAttr.toLower();

    switch (d->mEntryType) {
    case Entry_None:
        if (attrLower == QLatin1String("version")) {
            if (!d->mDn.isEmpty()) {
                retval = Err;
            }
        } else if (attrLower == QLatin1String("dn")) {
            qCDebug(KCONTACTS_LOG) << "ldapentry dn:" << QString::fromUtf8(d->mValue);
            d->mDn = d->mValue;
            d->mModType = Mod_None;
            retval = NewEntry;
        } else if (attrLower == QLatin1String("changetype")) {
            if (d->mDn.isEmpty()) {
                retval = Err;
            } else {
                QString tmpval = QString::fromUtf8(d->mValue);
                qCDebug(KCONTACTS_LOG) << "changetype:" << tmpval;
                if (tmpval == QLatin1String("add")) {
                    d->mEntryType = Entry_Add;
                } else if (tmpval == QLatin1String("delete")) {
                    d->mEntryType = Entry_Del;
                } else if (tmpval == QLatin1String("modrdn") || tmpval == QLatin1String("moddn")) {
                    d->mNewRdn = QLatin1String("");
                    d->mNewSuperior = QLatin1String("");
                    d->mDelOldRdn = true;
                    d->mEntryType = Entry_Modrdn;
                } else if (tmpval == QLatin1String("modify")) {
                    d->mEntryType = Entry_Mod;
                } else {
                    retval = Err;
                }
            }
        } else if (attrLower == QLatin1String("control")) {
            d->mUrl = splitControl(d->mValue, d->mOid, d->mCritical, d->mValue);
            retval = Control;
        } else if (!d->mAttr.isEmpty() && !d->mValue.isEmpty()) {
            d->mEntryType = Entry_Add;
            retval = Item;
        }
        break;
    case Entry_Add:
        if (d->mAttr.isEmpty() && d->mValue.isEmpty()) {
            retval = EndEntry;
        } else {
            retval = Item;
        }
        break;
    case Entry_Del:
        if (d->mAttr.isEmpty() && d->mValue.isEmpty()) {
            retval = EndEntry;
        } else {
            retval = Err;
        }
        break;
    case Entry_Mod:
        if (d->mModType == Mod_None) {
            qCDebug(KCONTACTS_LOG) << "new modtype" << d->mAttr;
            if (d->mAttr.isEmpty() && d->mValue.isEmpty()) {
                retval = EndEntry;
            } else if (attrLower == QLatin1String("add")) {
                d->mModType = Mod_Add;
            } else if (attrLower == QLatin1String("replace")) {
                d->mModType = Mod_Replace;
                d->mAttr = QString::fromUtf8(d->mValue);
                d->mValue = QByteArray();
                retval = Item;
            } else if (attrLower == QLatin1String("delete")) {
                d->mModType = Mod_Del;
                d->mAttr = QString::fromUtf8(d->mValue);
                d->mValue = QByteArray();
                retval = Item;
            } else {
                retval = Err;
            }
        } else {
            if (d->mAttr.isEmpty()) {
                if (QString::fromUtf8(d->mValue) == QLatin1String("-")) {
                    d->mModType = Mod_None;
                } else if (d->mValue.isEmpty()) {
                    retval = EndEntry;
                } else {
                    retval = Err;
                }
            } else {
                retval = Item;
            }
        }
        break;
    case Entry_Modrdn:
        if (d->mAttr.isEmpty() && d->mValue.isEmpty()) {
            retval = EndEntry;
        } else if (attrLower == QLatin1String("newrdn")) {
            d->mNewRdn = QString::fromUtf8(d->mValue);
        } else if (attrLower == QLatin1String("newsuperior")) {
            d->mNewSuperior = QString::fromUtf8(d->mValue);
        } else if (attrLower == QLatin1String("deleteoldrdn")) {
            if (d->mValue.size() > 0 && d->mValue[0] == '0') {
                d->mDelOldRdn = false;
            } else if (d->mValue.size() > 0 && d->mValue[0] == '1') {
                d->mDelOldRdn = true;
            } else {
                retval = Err;
            }
        } else {
            retval = Err;
        }
        break;
    }
    return retval;
}

Ldif::ParseValue Ldif::nextItem()
{
    ParseValue retval = None;
    char c = 0;

    while (retval == None) {
        if (d->mPos < (uint)d->mLdif.size()) {
            c = d->mLdif[d->mPos];
            d->mPos++;
            if (d->mIsNewLine && c == '\r') {
                continue; // handle \n\r line end
            }
            if (d->mIsNewLine && (c == ' ' || c == '\t')) { // line folding
                d->mIsNewLine = false;
                continue;
            }
            if (d->mIsNewLine) {
                d->mIsNewLine = false;
                retval = processLine();
                d->mLastParseValue = retval;
                d->mLine.resize(0);
                d->mIsComment = (c == '#');
            }
            if (c == '\n' || c == '\r') {
                d->mLineNumber++;
                d->mIsNewLine = true;
                continue;
            }
        } else {
            retval = MoreData;
            break;
        }

        if (!d->mIsComment) {
            d->mLine += c;
        }
    }
    return retval;
}

void Ldif::endLdif()
{
    QByteArray tmp(3, '\n');
    d->mLdif = tmp;
    d->mPos = 0;
}

void Ldif::startParsing()
{
    d->mPos = d->mLineNumber = 0;
    d->mDelOldRdn = false;
    d->mEntryType = Entry_None;
    d->mModType = Mod_None;
    d->mNewRdn.clear();
    d->mNewSuperior.clear();
    d->mLine = QByteArray();
    d->mIsNewLine = false;
    d->mIsComment = false;
    d->mLastParseValue = None;
}

void Ldif::setLdif(const QByteArray &ldif)
{
    d->mLdif = ldif;
    d->mPos = 0;
}

Ldif::EntryType Ldif::entryType() const
{
    return d->mEntryType;
}

int Ldif::modType() const
{
    return d->mModType;
}

QString Ldif::newRdn() const
{
    return d->mNewRdn;
}

QString Ldif::newSuperior() const
{
    return d->mNewSuperior;
}

bool Ldif::delOldRdn() const
{
    return d->mDelOldRdn;
}

QString Ldif::attr() const
{
    return d->mAttr;
}

QByteArray Ldif::value() const
{
    return d->mValue;
}

bool Ldif::isUrl() const
{
    return d->mUrl;
}

bool Ldif::isCritical() const
{
    return d->mCritical;
}

QString Ldif::oid() const
{
    return d->mOid;
}

uint Ldif::lineNumber() const
{
    return d->mLineNumber;
}
