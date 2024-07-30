/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2000, 2007 David Faure <faure@kde.org>
   SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

   SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only
*/
#ifndef FILEGROUPDETAILS_H
#define FILEGROUPDETAILS_H

#include <QWidget>
class MimeTypeData;
class QButtonGroup;

/**
 * This widget contains the details for a filetype group.
 * Currently this only involves the embedding configuration.
 */
class FileGroupDetails : public QWidget
{
    Q_OBJECT
public:
    explicit FileGroupDetails(QWidget *parent = nullptr);

    void setMimeTypeData(MimeTypeData *mimeTypeData);

Q_SIGNALS:
    void changed(bool);

protected Q_SLOTS:
    void slotAutoEmbedClicked(int button);

private:
    MimeTypeData *m_mimeTypeData;

    // Embedding config
    QButtonGroup *m_autoEmbed;
};

#endif
