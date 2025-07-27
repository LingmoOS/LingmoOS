/*
    SPDX-FileCopyrightText: 2002 Jean-Baptiste Mardelle <bj@altern.org>
    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef KGPGAPPLET_H
#define KGPGAPPLET_H

#include <QApplication>
#include <QKeySequence>

#include <KLocalizedString>

class KeysManager;
class KGpgExternalActions;
class QCommandLineParser;
class QDir;
class QString;

static const QString EMailTemplateText=i18n(
    "Hi,\n\nplease find attached the user id '%UIDNAME%' of your key %KEYID% signed by me. "
    "This mail is encrypted with that key to make sure you control both the email address and the key.\n\n"
    "If you have multiple user ids, I sent the signature for each user id separately to that user id's associated email address. "
    "You can import the signatures by running each through `gpg --import` after you have decrypted them with `gpg --decrypt`.\n\n"
    "If you are using KGpg store the attachment to disk and then import it. Just select `Import Key...` from `Keys` menu and open the file.\n\n"
    "Note that I did not upload your key to any keyservers. If you want this new signature to be available to others, please upload it yourself. "
    "With GnuPG this can be done using gpg --keyserver subkeys.pgp.net --send-key %KEYID%.\n\n" 
    "With KGpg you can right click on the key once you imported all user ids and choose `Export Public Key...`.\n\n"
    "If you have any questions, don't hesitate to ask.\n");

class KGpgApp : public QApplication
{
    Q_OBJECT

public:
    KGpgApp(int &argc, char **argv);
    ~KGpgApp() override;

	bool newInstance();
	QKeySequence goHome;

	/**
	 * @brief configure the QCommandLineParser to know about the control arguments
	 */
	void setupCmdlineParser(QCommandLineParser &parser);

	/**
	 * @brief process the actions requested by the user
	 */
	void handleArguments(const QCommandLineParser &parser, const QDir &workingDirectory);

public Q_SLOTS:
	void slotDBusActivation(const QStringList &arguments, const QString &workingDirectory);

private:
    KGpgExternalActions *w;
    KeysManager *s_keyManager;

private Q_SLOTS:
    void slotHandleQuit();
    void assistantOver(const QString &defaultKeyId);
};

#endif // KGPGAPPLET_H
