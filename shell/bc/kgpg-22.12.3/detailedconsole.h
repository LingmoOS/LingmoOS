/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGDETAILEDCONSOLE_H
#define KGPGDETAILEDCONSOLE_H

#include <QDialog>

class KgpgDetailedInfo : public QDialog
{
public:
	explicit KgpgDetailedInfo(QWidget *parent = nullptr, const QString &boxLabel = QString(),
			const QString &errormessage = QString(), const QStringList &keysList = QStringList(),
			const QString &caption = QString());
};

#endif // KGPGDETAILEDCONSOLE_H
