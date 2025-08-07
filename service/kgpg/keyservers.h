/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-FileCopyrightText: 2007, 2008, 2009, 2010, 2012, 2013 Rolf Eike Beer <kde@opensource.sf-tec.de>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KEYSERVERS_H
#define KEYSERVERS_H

#include <QSortFilterProxyModel>

#include <QDialog>

#include "core/kgpgkey.h"
#include "model/kgpgsearchresultmodel.h"
#include "ui_searchres.h"
#include "ui_keyserver.h"

class KGpgKeyserverSearchTransaction;
class KeyListProxyModel;
class KGpgItemModel;

class keyServerWidget : public QWidget, public Ui::keyServerWidget
{
public:
    explicit keyServerWidget(QWidget *parent = nullptr)
      : QWidget(parent)
    {
        setupUi(this);
    }
};

class searchRes : public QWidget, public Ui::searchRes
{
public:
    explicit searchRes(QWidget *parent)
      : QWidget(parent)
    {
        setupUi(this);
    }
};

class KeyServer : public QDialog
{
	Q_OBJECT

public:
	explicit KeyServer(QWidget *parent = nullptr, KGpgItemModel *model = nullptr, const bool autoclose = false);
	~KeyServer() override;

	/**
	 * Returns the server list.
	 * The first item is the server configured in gpg.
	 */
	static QStringList getServerList();

	/**
	 * @brief import the given keys
	 * @param keys the key fingerprints or ids to import
	 * @param server the key server to use, if empty the default server is used
	 * @param proxy the proxy to use
	 *
	 * This will set up the underlying transaction to fetch the keys as well
	 * as it will take care of setting up everything so the user will get the
	 * results shown.
	 */
	void startImport(const QStringList &keys, QString server = QString(), const QString &proxy = QString());

Q_SIGNALS:
	/**
	 * @brief emitted when importing keys succeeded
	 *
	 * This is only emitted when at least one key has been imported.
	 */
	void importFinished(QStringList);

	/**
	 * @brief importing failed
	 *
	 * This is emitted when key importing is finished and importFinished() is
	 * _not_ emitted. The main usage is to properly clean up the object when
	 * using startImport().
	 */
	void importFailed();

public Q_SLOTS:
	void slotImport();

	void slotExport(const QStringList &keyIds);

	void slotSetText(const QString &text);
	void slotTextChanged(const QString &text);
	void slotSetExportAttribute(const QString &attr);
	void slotEnableProxyI(const bool on);
	void slotEnableProxyE(const bool on);
	void slotSetKeyserver(const QString &server);

	void transferKeyID();
	void slotPreImport();
	void slotPreExport();

	void slotOk();
	void handleQuit();

private Q_SLOTS:
	void slotDownloadKeysFinished(int resultcode);
	void slotUploadKeysFinished(int resultcode);

	void slotSearchResult(int result);
	void slotSearch();
	void slotSetFilterString(const QString &expression);
	void slotUpdateLabelOnFilterChange();

private:
	QString m_readmessage;

	QDialog *m_dialogserver;
	KGpgKeyserverSearchTransaction *m_searchproc;

	keyServerWidget *page;
	searchRes *m_listpop;

	bool m_autoclose;
	QString expattr;

	KGpgSearchResultModel m_resultmodel;

	KeyListProxyModel *m_itemmodel;
};

#endif // KEYSERVERS_H
