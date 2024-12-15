// This file is part of the SpeedCrunch project
// Copyright (C) 2007 Ariya Hidayat <ariya@kde.org>
// Copyright (C) 2008-2009, 2016 @heldercorreia
// Copyright (C) 2009 Andreas Scherer <andreas_coder@freenet.de>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; see the file COPYING.  If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
// Boston, MA 02110-1301, USA.

/*#ifndef CORE_CONSTANTS_H
#define CORE_CONSTANTS_H

#include <QObject>
#include <QStringList>

#include <memory>

struct Constant {
    QString category;
    QString name;
    QString unit;
    QString value;
};

class Constants : public QObject {
    Q_OBJECT

public:
    ~Constants(); //  For unique_ptr, define after Private is complete.
    static Constants* instance();
    const QStringList& categories() const;
    const QList<Constant>& list() const;

public slots:
    void retranslateText();

private:
    struct Private;
    std::unique_ptr<Private> d;

    Constants();
    Q_DISABLE_COPY(Constants)
};

#endif*/

