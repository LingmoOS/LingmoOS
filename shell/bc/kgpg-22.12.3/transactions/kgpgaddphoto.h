/*
    SPDX-FileCopyrightText: 2008, 2009 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGADDPHOTO_H
#define KGPGADDPHOTO_H

#include <QObject>

#include "kgpgeditkeytransaction.h"

class QString;

class KGpgAddPhoto: public KGpgEditKeyTransaction {
	Q_OBJECT

	Q_DISABLE_COPY(KGpgAddPhoto)
public:
    explicit KGpgAddPhoto(QObject *parent, const QString &keyid, const QString &imagepath);
    ~KGpgAddPhoto() override;

	void setImagePath(const QString &imagepath);

protected:
	bool nextLine(const QString &line) override;
	KGpgTransaction::ts_boolanswer boolQuestion(const QString &line) override;

private:
	QString m_photourl;
};

#endif // KGPGADDPHOTO_H
