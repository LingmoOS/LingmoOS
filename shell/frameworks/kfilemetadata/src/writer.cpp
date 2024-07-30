/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2016 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "writer.h"
#include "writer_p.h"
#include "writerplugin.h"

using namespace KFileMetaData;

Writer::Writer()
    : d(new WriterPrivate)
{
}

Writer::~Writer() = default;

Writer::Writer(Writer&& other)
{
    d = std::move(other.d);
}


void Writer::write(const WriteData& data)
{
    d->m_plugin->write(data);
}

QStringList Writer::mimetypes() const
{
    return d->m_plugin->writeMimetypes();
}

void Writer::setAutoDeletePlugin(WriterPluginOwnership autoDelete)
{
    d->m_autoDeletePlugin = autoDelete;
}
