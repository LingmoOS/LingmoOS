/*
    SPDX-FileCopyrightText: 2022 Jonah Brüchert <jbb@kaidan.im>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#pragma once

#include <QRandomGenerator>
#include <QString>

inline QString photoImageProviderUri(const QStringView personUri)
{
    return u"image://kpeople-avatar/" % QString::fromUtf8(personUri.toUtf8().toBase64()) % u"#" % QString::number(QRandomGenerator::system()->generate());
}
