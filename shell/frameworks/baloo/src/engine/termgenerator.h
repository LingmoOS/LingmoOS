/*
    This file is part of the KDE Baloo project.
    SPDX-FileCopyrightText: 2014-2015 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef BALOO_TERMGENERATOR_H
#define BALOO_TERMGENERATOR_H

#include <QByteArray>
#include <QString>
#include "engine_export.h"
#include "document.h"

namespace Baloo {

class BALOO_ENGINE_EXPORT TermGenerator
{
public:
    explicit TermGenerator(Document& doc);

    void setDocument(Document& doc) {
        m_doc = doc;
    }

    void indexText(const QString& text);
    void indexText(const QString& text, const QByteArray& prefix);

    void indexXattrText(const QString& text, const QByteArray& prefix);
    void indexFileNameText(const QString& text);

    void setPosition(int position);
    int position() const;

    static QByteArrayList termList(const QString& text);

    // Trim all terms to this size
    const static int maxTermSize = 25;
private:
    Document& m_doc;
    int m_position;
};
}

#endif // BALOO_TERMGENERATOR_H
