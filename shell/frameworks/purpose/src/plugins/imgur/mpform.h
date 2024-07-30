/*
    This file is a part of kipi-plugins project
    http://www.kipi-plugins.org

    Date        : 2005-07-07
    Description : a tool to export images to imgur.com

    SPDX-FileCopyrightText: 2005-2008 Vardhman Jain <vardhman at gmail dot com>
    SPDX-FileCopyrightText: 2008-2013 Gilles Caulier <caulier dot gilles at gmail dot com>
    SPDX-FileCopyrightText: 2010-2012 Marius Orcsik <marius at habarnam dot ro>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MPFORM_H
#define MPFORM_H

// Qt includes

#include <QByteArray>
#include <QUrl>

class MPForm
{
public:
    MPForm();
    ~MPForm();

    void finish();
    void reset();

    bool addPair(const QString &name, const QString &value, const QString &type);
    bool addFile(const QString &name, const QString &path);
    bool addFile(const QString &name, const QUrl &filePath, const QByteArray &fileData);

    QByteArray contentType() const;
    QByteArray formData() const;
    QByteArray boundary() const;

private:
    QByteArray m_buffer;
    QByteArray m_boundary;
};

#endif /* MPFORM_H */
