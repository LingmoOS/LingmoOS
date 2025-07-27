/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2008, 2009, 2010, 2011 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef _KGPGEXTERNALACTIONS_H
#define _KGPGEXTERNALACTIONS_H

#include <QObject>
#include <QPointer>
#include <QStringList>

#include <QUrl>

class KeysManager;
class KGpgFirstAssistant;
class KGpgItemModel;
class KJob;
class QKeySequence;
class QTemporaryFile;
class QString;

/**
 * @brief handle actions called from outside the application
 *
 * This class takes care about everything sent to us from outside the
 * application, e.g. command line arguments given on startup.
 */
class KGpgExternalActions : public QObject
{
	Q_OBJECT

public:
	KGpgExternalActions(KeysManager *parent, KGpgItemModel *model);
	~KGpgExternalActions() override;

	void showDroppedFile(const QUrl &file);
	void verifyFile(QUrl url);

	/**
	 * @brief create a detached signature for the given files
	 */
	static void signFiles(KeysManager* parent, const QList<QUrl> &urls);

	static void decryptFiles(KeysManager* parent, const QList<QUrl>& urls);
	static void encryptFolders(KeysManager* parent, const QList<QUrl> &urls);

	/**
	 * @brief create a new object, encrypt the given files, and destroy the object
	 */
	static void encryptFiles(KeysManager* parent, const QList<QUrl>& urls);

	void readOptions();
Q_SIGNALS:
	void createNewKey();
	void updateDefault(QString);

private:
	QStringList customDecrypt;
	QPointer<KGpgFirstAssistant> m_assistant;
	int compressionScheme;
	KGpgItemModel *m_model;
	QTemporaryFile *m_kgpgfoldertmp;

	void startAssistant();
	void firstRun();

	QList<QUrl> m_decryptionFailed;
	KeysManager *m_keysmanager;
	QList<QUrl> droppedUrls;

	QKeySequence goDefaultKey() const;
	void decryptFile(QList<QUrl> urls);

private Q_SLOTS:
	void startFolderEncode();
	void slotSaveOptionsPath();
	void slotVerificationDone(int result);
	void help();
	void slotSetCompression(int cp);
	void slotDecryptionDone(int status);
	void slotFolderFinished(KJob *job);
	void slotSignFiles();
	void slotEncryptionKeySelected();
};

#endif /* _KGPGEXTERNALACTIONS_H */
