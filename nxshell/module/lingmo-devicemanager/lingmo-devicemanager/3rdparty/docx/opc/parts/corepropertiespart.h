// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COREPROPERTIESPART_H
#define COREPROPERTIESPART_H

#include <QString>
#include <QByteArray>

#include "../part.h"
#include "../../package.h"

namespace Docx {


class CorePropertiesPart : public XmlPart
{
public:
    CorePropertiesPart(const QString &partName, const QString &contentType
                       , const QByteArray &blob, Package *package = nullptr);
//    static void load(const QString &partName, const QString &contentType
//                , const QByteArray &blob, Package *package = nullptr);
    virtual ~CorePropertiesPart();

    void afterUnmarshal();
};

}
#endif // COREPROPERTIESPART_H
