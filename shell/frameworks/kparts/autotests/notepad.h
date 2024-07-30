/*
    SPDX-FileCopyrightText: 1999, 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 1999, 2000 Simon Hausmann <hausmann@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include <kparts/readwritepart.h>

class KPluginMetaData;
class QTextEdit;

/**
 * Who said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 * @internal
 */
class NotepadPart : public KParts::ReadWritePart
{
    Q_OBJECT
public:
    NotepadPart(QWidget *parentWidget, QObject *parent, const KPluginMetaData &metaData);
    ~NotepadPart() override;

    void setReadWrite(bool rw) override;

protected:
    bool openFile() override;
    bool saveFile() override;

protected:
    QTextEdit *m_edit;
};
