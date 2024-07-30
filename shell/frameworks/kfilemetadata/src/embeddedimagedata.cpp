/*
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "embeddedimagedata.h"
#include "extractorcollection.h"
#include "simpleextractionresult.h"
#include "writedata.h"
#include "writercollection.h"
#include "kfilemetadata_debug.h"

#include <QMimeDatabase>

using namespace KFileMetaData;

class Q_DECL_HIDDEN EmbeddedImageData::Private
{
public:
    QMimeDatabase mMimeDatabase;
    static const QStringList mMimetypes;
};

const QStringList EmbeddedImageData::Private::mMimetypes =
{
    QStringLiteral("audio/flac"),
    QStringLiteral("audio/mp4"),
    QStringLiteral("audio/mpeg"),
    QStringLiteral("audio/ogg"),
    QStringLiteral("audio/wav"),
    QStringLiteral("audio/x-aiff"),
    QStringLiteral("audio/x-ape"),
    QStringLiteral("audio/x-ms-wma"),
    QStringLiteral("audio/x-musepack"),
    QStringLiteral("audio/x-opus+ogg"),
    QStringLiteral("audio/x-speex+ogg"),
    QStringLiteral("audio/x-vorbis+ogg"),
    QStringLiteral("audio/x-wav"),
    QStringLiteral("audio/x-wavpack"),
};

EmbeddedImageData::EmbeddedImageData()
    : d(std::unique_ptr<Private>(new Private()))
{
}

EmbeddedImageData::~EmbeddedImageData()
= default;
