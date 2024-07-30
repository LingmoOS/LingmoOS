/*
    Gettext translation file analyzer

    SPDX-FileCopyrightText: 2007 Montel Laurent <montel@kde.org>
    SPDX-FileCopyrightText: 2009 Jos van den Oever <jos@vandenoever.info>
    SPDX-FileCopyrightText: 2014 Nick Shaforostoff <shaforostoff@gmail.com>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/


#ifndef POEXTRACTOR_H
#define POEXTRACTOR_H

#include "extractorplugin.h"

namespace KFileMetaData
{

class POExtractor : public ExtractorPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.ExtractorPlugin"
                      FILE "poextractor.json")
    Q_INTERFACES(KFileMetaData::ExtractorPlugin)

public:
    explicit POExtractor(QObject* parent = nullptr);

    QStringList mimetypes() const override;
    void extract(ExtractionResult* result) override;

private:
    void endMessage();
    void handleComment(const char* data, quint32 length);
    void handleLine(const char* data, quint32 length);


    enum PoState {COMMENT, MSGCTXT, MSGID, MSGID_PLURAL, MSGSTR, MSGSTR_PLURAL,
        WHITESPACE, ERROR};
    PoState state;
    int messages;
    int untranslated;
    int fuzzy;
    bool isFuzzy=false, isTranslated=false;
};

}

#endif // PLAINTEXTEXTRACTOR_H
