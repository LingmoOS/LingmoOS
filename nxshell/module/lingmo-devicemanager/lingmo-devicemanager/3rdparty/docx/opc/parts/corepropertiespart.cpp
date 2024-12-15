// SPDX-FileCopyrightText: 2022 lpxxn <mi_duo@live.com>
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "corepropertiespart.h"

using namespace Docx;

CorePropertiesPart::CorePropertiesPart(const QString &partName, const QString &contentType
                                       , const QByteArray &blob, Package *package)
    : XmlPart(partName, contentType, blob, package)
{

}

//void CorePropertiesPart::load(const QString &partName, const QString &contentType, const QByteArray &blob, Package *package)
//{

//}

CorePropertiesPart::~CorePropertiesPart()
{

}



void Docx::CorePropertiesPart::afterUnmarshal()
{
}
