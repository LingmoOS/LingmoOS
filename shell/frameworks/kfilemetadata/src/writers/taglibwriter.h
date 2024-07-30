/*
    SPDX-FileCopyrightText: 2016 Varun Joshi <varunj.1011@gmail.com>
    SPDX-FileCopyrightText: 2018 Alexander Stippich <a.stippich@gmx.net>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef TAGLIBWRITER_H
#define TAGLIBWRITER_H

#include "writerplugin.h"

namespace KFileMetaData
{

class TagLibWriter : public WriterPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.kf5.kfilemetadata.WriterPlugin")
    Q_INTERFACES(KFileMetaData::WriterPlugin)

public:
    explicit TagLibWriter(QObject* parent = nullptr);

    void write(const WriteData& data) override;
    QStringList writeMimetypes() const override;
};

}

#endif // TAGLIBWRITER_H

