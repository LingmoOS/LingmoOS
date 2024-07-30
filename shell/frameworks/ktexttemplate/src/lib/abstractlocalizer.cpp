/*
  This file is part of the KTextTemplate library

  SPDX-FileCopyrightText: 2010 Stephen Kelly <steveire@gmail.com>

  SPDX-License-Identifier: LGPL-2.1-or-later

*/

#include "abstractlocalizer.h"

#include "util.h"

#include <QDateTime>

using namespace KTextTemplate;

AbstractLocalizer::AbstractLocalizer() = default;

AbstractLocalizer::~AbstractLocalizer() = default;

QString AbstractLocalizer::localize(const QVariant &variant) const
{
    if (variant.userType() == qMetaTypeId<QDate>())
        return localizeDate(variant.value<QDate>());
    if (variant.userType() == qMetaTypeId<QTime>())
        return localizeTime(variant.value<QTime>());
    if (variant.userType() == qMetaTypeId<QDateTime>())
        return localizeDateTime(variant.value<QDateTime>());
    if (isSafeString(variant))
        return localizeString(getSafeString(variant).get());
    if (variant.userType() == qMetaTypeId<double>() || variant.userType() == qMetaTypeId<float>())
        return localizeNumber(variant.value<double>());
    if (variant.canConvert<int>())
        return localizeNumber(variant.value<int>());
    return {};
}
