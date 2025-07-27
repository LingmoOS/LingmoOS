/*
    SPDX-FileCopyrightText: 2008 Rolf Eike Beer <kde@opensource.sf-tec.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/
#ifndef KGPGFIRSTASSISTANT_H
#define KGPGFIRSTASSISTANT_H

#include <KAssistantDialog>

class QCheckBox;
class QComboBox;
class QLabel;
class KPageWidgetItem;
class KUrlRequester;

class KGpgFirstAssistant : public KAssistantDialog
{
	Q_OBJECT

private:
	KPageWidgetItem *page_welcome;
	KPageWidgetItem *page_binary;
	KPageWidgetItem *page_config;
	KPageWidgetItem *page_defaultkey;
	KPageWidgetItem *page_done;

	QLabel *defaultkeylabel;
	QLabel *txtGpgVersion;
	QLabel *text_optionsfound;
	QLabel *versionLabel;
	QLabel *binlabel;

	QCheckBox *generateCB;
	QCheckBox *autostartCB;

	QComboBox *CBdefault;

	KUrlRequester *binURL;
	KUrlRequester *pathURL;
	
	QString m_gpgVersion;
	QString m_confPath;

	void findConfigPath(const QString &gpgBinary);

public:
	/**
	 * @brief constructor of KGpgFirstAssistant
	 */
	explicit KGpgFirstAssistant(QWidget *parent = nullptr);

	/**
	 * @brief check if key generation dialog should be started
	 * @return if user requests dialog to be started
	 */
	bool runKeyGenerate() const;
	/**
	 * @brief get user selected GnuPG home directory
	 * @return path to users GnuPG directory
	 */
	QString getConfigPath() const;
	/**
	 * @brief get fingerprint of default key
	 * @return full fingerprint or empty string if user has not selected a default key
	 */
	QString getDefaultKey() const;
	/**
	 * @brief check if KGpg autostart should be activated
	 * @return if user requests autostart or not
	 */
	bool getAutoStart() const;

public Q_SLOTS:
	/**
	 * @brief called when "next" button is pressed
	 */
        void next() override;

private Q_SLOTS:
	void slotBinaryChanged(const QString &binary);
};

#endif
