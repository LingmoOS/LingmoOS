/*
    This file is part of the KFileMetaData project
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2016 Vishesh Handa <me@vhanda.in>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "writerplugin.h"


using namespace KFileMetaData;

WriterPlugin::WriterPlugin(QObject* parent): QObject(parent)
{
}

WriterPlugin::~WriterPlugin()
{
}

#include "moc_writerplugin.cpp"
