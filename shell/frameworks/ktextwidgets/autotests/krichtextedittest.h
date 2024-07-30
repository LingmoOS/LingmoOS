/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2009 Thomas McGuire <mcguire@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KRICHTEXTEDITTEST_H
#define KRICHTEXTEDITTEST_H

#include <QObject>

class KRichTextEditTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testLinebreaks();
    void testUpdateLinkAdd();
    void testUpdateLinkRemove();
    void testHTMLLineBreaks();
    void testHTMLOrderedLists();
    void testHTMLUnorderedLists();
    void testHeading();
    void testRulerScroll();
    void testNestedLists();
};

#endif
