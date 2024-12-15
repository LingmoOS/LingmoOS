// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "packuri.h"

using namespace Docx;

PackURI::PackURI()
{

}

PackURI::PackURI(const QString &str) : QString(str)
{

}

PackURI PackURI::fromRelRef(const QString &baseURI, const QString &relative_ref)
{    
    return PackURI(QString().append(baseURI).append(relative_ref));
}

QString PackURI::baseURI() const
{
    QStringList strs = pathSplit();

    return strs.first();
}

QString PackURI::fullURI() const
{
    return QString("/").append(*this);
}

QString PackURI::fileName() const
{
    QStringList strs = pathSplit();

    return strs.at(1);
}

/*!
 * \brief PackURI::idx
 * \return
 */
int PackURI::idx() const
{
    QString filename = fileName();
    QString namePart = filename.split('.').first();
    QStringList sl;

    for (int i = namePart.length() - 1; i > 0; i--) {
        if (namePart.at(i).isNumber())
            sl.insert(0, namePart.at(i));
        else
            break;
    }
    if (sl.isEmpty())
        return 0;
    return sl.join("").toInt();
}

PackURI PackURI::relsUri() const
{
    QString rels_uri_str = relsUriStr();
    return PackURI(rels_uri_str);
}

QString PackURI::relsUriStr() const
{
    QStringList strs = pathSplit();

    QString rels_uri_str = QString::fromLatin1("%1_rels/%2.rels").arg(strs.at(0)).arg(strs.at(1));
    return rels_uri_str;
}

QString PackURI::memberName() const
{
    // remove first /
    return this->mid(1);
}

QString PackURI::ext() const
{
    return this->split('.').last();
}


QString PackURI::relativeRef(const QString &baseURI)
{
    QString str;
    if (baseURI == QStringLiteral("/"))
        str = this->mid(1);
    else {
        str = this->replace(baseURI, "");
        if (str.startsWith('/'))
            str = str.mid(1);
    }

    return str;
}

PackURI::~PackURI()
{

}

QStringList PackURI::pathSplit() const
{
    QStringList oStr;
    if (this->length() == 1) {
        oStr << QStringLiteral("") << QStringLiteral("");
        return oStr;
    }

    int i = this->lastIndexOf('/');

    oStr.append(this->left(i + 1));

    int l = this->length() - i - 1;

    oStr.append(this->right(l));
    return oStr;

}

