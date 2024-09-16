// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef OPCPACKURI_H
#define OPCPACKURI_H

#include <QStringList>

namespace Docx {

class PackURI : public QString
{
public:
    PackURI();
    PackURI(const QString &str);
    static PackURI fromRelRef(const QString &baseURI, const QString &relative_ref);
    QString baseURI() const;
    QString fullURI() const;
    QString fileName() const;
    int idx() const;
    PackURI relsUri() const;
    QString relsUriStr() const;
    QString memberName() const;
    QString ext() const;
    QString relativeRef(const QString &baseURI);

    virtual ~PackURI();

private:
    QStringList pathSplit() const;
};

}

#endif // PACKURI_H
