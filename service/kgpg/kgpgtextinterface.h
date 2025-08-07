/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2011, 2012 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGTEXTINTERFACE_H
#define KGPGTEXTINTERFACE_H

#include <QObject>
#include <QUrl>
#include <QStringList>

class KGpgTextInterfacePrivate;
class QString;

class KGpgTextInterface : public QObject
{
	Q_OBJECT

private:
	KGpgTextInterfacePrivate * const d;

	KGpgTextInterface();
	Q_DISABLE_COPY(KGpgTextInterface)

public:
	explicit KGpgTextInterface(QObject *parent, const QString &keyID, const QStringList &options);
	~KGpgTextInterface() override;

Q_SIGNALS:
    /**
     * Emitted when all files passed to KgpgSignFile() where processed.
     */
    void fileSignFinished();

public Q_SLOTS:
    /**
     * Sign file function
     * @param srcUrl file to sign.
     */
    void signFiles(const QList<QUrl> &srcUrl);

private Q_SLOTS:
	void slotSignFile();
	void slotSignFinished();
};

#endif
